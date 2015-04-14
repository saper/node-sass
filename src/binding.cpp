#include <nan.h>
#include <vector>
#include "sass_context_wrapper.h"
#if defined(SASS_C_CONTEXT_H)
/* Hack: libsass 3.1.0 used SASS_C_CONTEXT */
#include <sass_functions.h>
#endif

char* CreateString(Local<Value> value) {
  if (value->IsNull() || !value->IsString()) {
    return 0;
  }

  String::Utf8Value string(value);
  char *str = (char *)malloc(string.length() + 1);
  strcpy(str, *string);
  return str;
}

std::vector<sass_context_wrapper*> imports_collection;

void prepare_import_results(Local<Value> returned_value, sass_context_wrapper* ctx_w) {
  NanScope();

  if (returned_value->IsArray()) {
    Handle<Array> array = Handle<Array>::Cast(returned_value);

    ctx_w->imports = sass_make_import_list(array->Length());

    for (size_t i = 0; i < array->Length(); ++i) {
      Local<Value> value = array->Get(i);

      if (!value->IsObject())
        continue;

      Local<Object> object = Local<Object>::Cast(value);
      char* path = CreateString(object->Get(NanNew<String>("file")));
      char* contents = CreateString(object->Get(NanNew<String>("contents")));

      ctx_w->imports[i] = sass_make_import_entry(path, (!contents || contents[0] == '\0') ? 0 : strdup(contents), 0);
    }
  }
  else if (returned_value->IsObject()) {
    ctx_w->imports = sass_make_import_list(1);
    Local<Object> object = Local<Object>::Cast(returned_value);
    char* path = CreateString(object->Get(NanNew<String>("file")));
    char* contents = CreateString(object->Get(NanNew<String>("contents")));

    ctx_w->imports[0] = sass_make_import_entry(path, (!contents || contents[0] == '\0') ? 0 : strdup(contents), 0);
  }
  else {
    ctx_w->imports = sass_make_import_list(1);
    ctx_w->imports[0] = sass_make_import_entry(ctx_w->file, 0, 0);
  }
}

void dispatched_async_uv_callback(uv_async_t *req) {
  NanScope();
  sass_context_wrapper* ctx_w = static_cast<sass_context_wrapper*>(req->data);

  TryCatch try_catch;

  fprintf(stderr, "Thread=%p: dispatched_async_uv_callback()\n", uv_thread_self());
  imports_collection.push_back(ctx_w);

  Handle<Value> argv[] = {
    NanNew<String>(strdup(ctx_w->file ? ctx_w->file : 0)),
    NanNew<String>(strdup(ctx_w->prev ? ctx_w->prev : 0)),
    NanNew<Number>(imports_collection.size() - 1)
  };

  NanNew<Value>(ctx_w->importer_callback->Call(3, argv));

  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }
}

struct Sass_Import** sass_importer(const char* file, const char* prev, void* cookie)
{
  sass_context_wrapper* ctx_w = static_cast<sass_context_wrapper*>(cookie);

  fprintf(stderr, "Thread=%p: sass_importer()\n", uv_thread_self());
  if (!ctx_w->is_sync) {
    /*  that is async: Render() or RenderFile(),
     *  the default even loop is unblocked so it
     *  can run uv_async_send without a push.
     */

    std::unique_lock<std::mutex> lock(*ctx_w->importer_mutex);

    ctx_w->file = file ? strdup(file) : 0;
    ctx_w->prev = prev ? strdup(prev) : 0;
    ctx_w->async->data = (void*)ctx_w;

    fprintf(stderr, "Thread=%p: sass_importer(): About to wait...", uv_thread_self());
    uv_async_send(ctx_w->async);
    ctx_w->importer_condition_variable->wait(lock);
    fprintf(stderr, "Thread=%p: sass_importer(): ... waiting complete\n", uv_thread_self());
  }
  else {
    fprintf(stderr, "Thread=%p: sass_importer(): sync call prepare\n", uv_thread_self());
    NanScope();
    fprintf(stderr, "Thread=%p: sass_importer(): got scope\n", uv_thread_self());

    Handle<Value> argv[] = {
      NanNew<String>(file),
      NanNew<String>(prev)
    };

    fprintf(stderr, "Thread=%p: sass_importer(): sync call\n", uv_thread_self());
    Local<Object> returned_value = Local<Object>::Cast(NanNew<Value>(ctx_w->importer_callback->Call(2, argv)));
    fprintf(stderr, "Thread=%p: sass_importer(): sync call done\n", uv_thread_self());

    prepare_import_results(returned_value->Get(NanNew("objectLiteral")), ctx_w);
  }

  return ctx_w->imports;
}

void ExtractOptions(Local<Object> options, void* cptr, sass_context_wrapper* ctx_w, bool isFile, bool isSync) {
  NanScope();

  struct Sass_Context* ctx;

  NanAssignPersistent(ctx_w->result, options->Get(NanNew("result"))->ToObject());

  if (isFile) {
    ctx_w->fctx = (struct Sass_File_Context*) cptr;
    ctx = sass_file_context_get_context(ctx_w->fctx);
  }
  else {
    ctx_w->dctx = (struct Sass_Data_Context*) cptr;
    ctx = sass_data_context_get_context(ctx_w->dctx);
  }

  struct Sass_Options* sass_options = sass_context_get_options(ctx);

  ctx_w->importer_callback = NULL;
  ctx_w->is_sync = isSync;
  ctx_w->async = new uv_async_t;
  ctx_w->request = new uv_work_t;

  if (!isSync) {
    ctx_w->request->data = ctx_w;

    // async (callback) style
    Local<Function> success_callback = Local<Function>::Cast(options->Get(NanNew("success")));
    Local<Function> error_callback = Local<Function>::Cast(options->Get(NanNew("error")));

    ctx_w->success_callback = new NanCallback(success_callback);
    ctx_w->error_callback = new NanCallback(error_callback);
  }

  Local<Function> importer_callback = Local<Function>::Cast(options->Get(NanNew("importer")));

  if (importer_callback->IsFunction()) {
    ctx_w->importer_callback = new NanCallback(importer_callback);
    uv_async_init(uv_default_loop(), ctx_w->async, (uv_async_cb)dispatched_async_uv_callback);
#if defined(SASS_C_CONTEXT_H) /* Hack: libsass 3.1.0 used SASS_C_CONTEXT */
    Sass_Importer_List c_importers = sass_make_importer_list(1);
    c_importers[0] = sass_make_importer((Sass_Importer_Fn)sass_importer, 0, (void *)ctx_w);
    sass_option_set_c_importers(sass_options, c_importers);
#else
    /* libsass 3.1.0 supported only one single importer */
    uv_async_init(uv_default_loop(), ctx_w->async, (uv_async_cb)dispatched_async_uv_callback);
    sass_option_set_importer(sass_options, sass_make_importer(sass_importer, ctx_w));
#endif
  }

  if(!isFile) {
    sass_option_set_input_path(sass_options, CreateString(options->Get(NanNew("file"))));
  }

  sass_option_set_output_path(sass_options, CreateString(options->Get(NanNew("outFile"))));
  sass_option_set_output_style(sass_options, (Sass_Output_Style)options->Get(NanNew("style"))->Int32Value());
  sass_option_set_is_indented_syntax_src(sass_options, options->Get(NanNew("indentedSyntax"))->BooleanValue());
  sass_option_set_source_comments(sass_options, options->Get(NanNew("comments"))->BooleanValue());
  sass_option_set_omit_source_map_url(sass_options, options->Get(NanNew("omitSourceMapUrl"))->BooleanValue());
  sass_option_set_source_map_embed(sass_options, options->Get(NanNew("sourceMapEmbed"))->BooleanValue());
  sass_option_set_source_map_contents(sass_options, options->Get(NanNew("sourceMapContents"))->BooleanValue());
  sass_option_set_source_map_file(sass_options, CreateString(options->Get(NanNew("sourceMap"))));
  sass_option_set_include_path(sass_options, CreateString(options->Get(NanNew("paths"))));
  sass_option_set_precision(sass_options, options->Get(NanNew("precision"))->Int32Value());
}

void GetStats(sass_context_wrapper* ctx_w, Sass_Context* ctx) {
  NanScope();

  char** included_files = sass_context_get_included_files(ctx);
  Handle<Array> arr = NanNew<Array>();

  if (included_files) {
    for (int i = 0; included_files[i] != nullptr; ++i) {
      arr->Set(i, NanNew<String>(included_files[i]));
    }
  }

  NanNew(ctx_w->result)->Get(NanNew("stats"))->ToObject()->Set(NanNew("includedFiles"), arr);
}

void GetSourceMap(sass_context_wrapper* ctx_w, Sass_Context* ctx) {
  NanScope();

  Handle<Value> source_map;

  if (sass_context_get_error_status(ctx)) {
    return;
  }

  if (sass_context_get_source_map_string(ctx)) {
    source_map = NanNew<String>(sass_context_get_source_map_string(ctx));
  }
  else {
    source_map = NanNew<String>("{}");
  }

  NanNew(ctx_w->result)->Set(NanNew("map"), source_map);
}

int GetResult(sass_context_wrapper* ctx_w, Sass_Context* ctx) {
  NanScope();

  int status = sass_context_get_error_status(ctx);

  if (status == 0) {
    NanNew(ctx_w->result)->Set(NanNew("css"), NanNew<String>(sass_context_get_output_string(ctx)));

    GetStats(ctx_w, ctx);
    GetSourceMap(ctx_w, ctx);
  }

  return status;
}

static void async_gone(uv_handle_t *handle) {
  delete (uv_async_t *)handle;
}

void make_callback(uv_work_t* req) {
  NanScope();

  TryCatch try_catch;
  sass_context_wrapper* ctx_w = static_cast<sass_context_wrapper*>(req->data);
  struct Sass_Context* ctx;

  fprintf(stderr, "Thread=%p: make_callback()\n", uv_thread_self());
  if (ctx_w->dctx) {
    ctx = sass_data_context_get_context(ctx_w->dctx);
  }
  else {
    ctx = sass_file_context_get_context(ctx_w->fctx);
  }

  int status = GetResult(ctx_w, ctx);

  if (status == 0 && ctx_w->success_callback) {
    // if no error, do callback(null, result)
    ctx_w->success_callback->Call(0, 0);
  }
  else if(ctx_w->error_callback) {
    // if error, do callback(error)
    const char* err = sass_context_get_error_json(ctx);
    Local<Value> argv[] = {
      NanNew<String>(err),
      NanNew<Integer>(status)
    };
    ctx_w->error_callback->Call(2, argv);
  }
  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }

  if (ctx_w->importer_callback) {
    uv_close((uv_handle_t*)ctx_w->async, async_gone);
  }
  delete ctx_w->request;

  sass_free_context_wrapper(ctx_w);
}

NAN_METHOD(Render) {
  NanScope();

  Local<Object> options = args[0]->ToObject();
  char* source_string = CreateString(options->Get(NanNew("data")));
  struct Sass_Data_Context* dctx = sass_make_data_context(source_string);
  sass_context_wrapper* ctx_w = sass_make_context_wrapper();
  fprintf(stderr, "Thread=%p: Render()\n", uv_thread_self());

  ExtractOptions(options, dctx, ctx_w, false, false);

  int status = uv_queue_work(uv_default_loop(), ctx_w->request, compile_it, (uv_after_work_cb)make_callback);

  assert(status == 0);

  NanReturnUndefined();
}

NAN_METHOD(RenderSync) {
  NanScope();

  Local<Object> options = args[0]->ToObject();
  char* source_string = CreateString(options->Get(NanNew("data")));
  struct Sass_Data_Context* dctx = sass_make_data_context(source_string);
  struct Sass_Context* ctx = sass_data_context_get_context(dctx);
  sass_context_wrapper* ctx_w = sass_make_context_wrapper();
  fprintf(stderr, "Thread=%p: RenderSync()\n", uv_thread_self());

  ExtractOptions(options, dctx, ctx_w, false, true);

  compile_data(dctx);

  int result = GetResult(ctx_w, ctx);
  Local<String> error;

  if (result != 0) {
    error = NanNew<String>(sass_context_get_error_json(ctx));
  }

  sass_wrapper_dispose(ctx_w, source_string);

  if (result != 0) {
    NanThrowError(error);
  }

  NanReturnValue(NanNew<Boolean>(result == 0));
}

NAN_METHOD(RenderFile) {
  NanScope();

  Local<Object> options = args[0]->ToObject();
  char* input_path = CreateString(options->Get(NanNew("file")));
  struct Sass_File_Context* fctx = sass_make_file_context(input_path);
  sass_context_wrapper* ctx_w = sass_make_context_wrapper();
  fprintf(stderr, "Thread=%p: RenderFile()\n", uv_thread_self());

  ExtractOptions(options, fctx, ctx_w, true, false);

  int status = uv_queue_work(uv_default_loop(), ctx_w->request, compile_it, (uv_after_work_cb)make_callback);

  assert(status == 0);

  NanReturnUndefined();
}

NAN_METHOD(RenderFileSync) {
  NanScope();

  Local<Object> options = args[0]->ToObject();
  char* input_path = CreateString(options->Get(NanNew("file")));
  struct Sass_File_Context* fctx = sass_make_file_context(input_path);
  struct Sass_Context* ctx = sass_file_context_get_context(fctx);
  sass_context_wrapper* ctx_w = sass_make_context_wrapper();
  fprintf(stderr, "Thread=%p: RenderFileSync()\n", uv_thread_self());

  ExtractOptions(options, fctx, ctx_w, true, true);
  compile_file(fctx);

  int result = GetResult(ctx_w, ctx);
  Local<String> error;

  if (result != 0) {
    error = NanNew<String>(sass_context_get_error_json(ctx));
  }

  sass_wrapper_dispose(ctx_w, input_path);

  if (result != 0) {
    NanThrowError(error);
  }

  NanReturnValue(NanNew<Boolean>(result == 0));
}

NAN_METHOD(ImportedCallback) {
  NanScope();

  TryCatch try_catch;

  fprintf(stderr, "Thread=%p: ImportedCallback()\n", uv_thread_self());
  Local<Object> options = args[0]->ToObject();
  Local<Value> returned_value = options->Get(NanNew("objectLiteral"));
  size_t index = options->Get(NanNew("index"))->Int32Value();

  if (index >= imports_collection.size()) {
    NanReturnUndefined();
  }

  sass_context_wrapper* ctx_w = imports_collection[index];

  prepare_import_results(returned_value, ctx_w);
  ctx_w->importer_condition_variable->notify_all();

  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }

  NanReturnValue(NanNew<Number>(0));
}

void RegisterModule(v8::Handle<v8::Object> target) {
  NODE_SET_METHOD(target, "render", Render);
  NODE_SET_METHOD(target, "renderSync", RenderSync);
  NODE_SET_METHOD(target, "renderFile", RenderFile);
  NODE_SET_METHOD(target, "renderFileSync", RenderFileSync);
  NODE_SET_METHOD(target, "importedCallback", ImportedCallback);
}

NODE_MODULE(binding, RegisterModule);
