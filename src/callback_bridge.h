#ifndef CALLBACK_BRIDGE_H
#define CALLBACK_BRIDGE_H

#include <uv.h>
#include <vector>
#include <nan.h>
#include <condition_variable>
#include <algorithm>

#define COMMA ,

using namespace v8;

template <typename T, typename L = void*>
class CallbackBridge {
  public:
    CallbackBridge(NanCallback*, bool);
    virtual ~CallbackBridge();

    // Executes the callback
    T operator()(std::vector<L>);

  protected:
    // We will expose a bridge object to the JS callback that wraps this instance so we don't loose context.
    // This is the V8 constructor for such objects.
    static Handle<Function> get_wrapper_constructor();
    static void async_gone(uv_handle_t *handle);
    static NAN_METHOD(New);
    static NAN_METHOD(ReturnCallback);
    static Persistent<Function> wrapper_constructor;
    Persistent<Object> wrapper;

    // The callback that will get called in the main thread after the worker thread used for the sass
    // compilation step makes a call to uv_async_send()
    static void queued_work(uv_work_t*);
    static void completed_work(uv_work_t*, int);

    // The V8 values sent to our ReturnCallback must be read on the main thread not the sass worker thread.
    // This gives a chance to specialized subclasses to transform those values into whatever makes sense to
    // sass before we resume the worker thread.
    virtual T post_process_return_value(Handle<Value>) const =0;


    virtual std::vector<Handle<Value>> pre_process_args(std::vector<L>) const =0;

    NanCallback* callback;
    bool is_sync;
    int cnt;

    uv_work_t *async;
    uv_barrier_t blocker;
    std::vector<L> argv;
    bool has_returned;
    T return_value;
};

template <typename T, typename L>
Persistent<Function> CallbackBridge<T, L>::wrapper_constructor;

template <typename T, typename L>
CallbackBridge<T, L>::CallbackBridge(NanCallback* callback, bool is_sync) : callback(callback), is_sync(is_sync), cnt(0) {
  // This assumes the main thread will be the one instantiating the bridge
  if (!is_sync) {
    this->async = new uv_work_t;
    fprintf(stderr, "T<%p>: Scheduling handle %p; sizeof(T) is 0x%zx, sizeof(uv_async_t) is 0x%zx\n", (void*) this, (void *)this->async, sizeof(*this), sizeof(*this->async));
    this->async->data = (void*) this;
    // uv_async_init(uv_default_loop(), this->async, (uv_async_cb) dispatched_async_uv_callback);
  }

  NanAssignPersistent(wrapper, NanNew(CallbackBridge<T, L>::get_wrapper_constructor())->NewInstance());
  NanSetInternalFieldPointer(NanNew(wrapper), 0, this);
}

template <typename T, typename L>
CallbackBridge<T, L>::~CallbackBridge() {
  delete this->callback;
  NanDisposePersistent(this->wrapper);
  if (!is_sync) {
    fprintf(stderr, "T<%p>: bridge will be destroyed, hope %p got called\n", (void*) this, (void *)this->async);
    uv_close((uv_handle_t*)this->async, &async_gone);
  }
}

template <typename T, typename L>
T CallbackBridge<T, L>::operator()(std::vector<L> argv) {
  // argv.push_back(wrapper);
  cnt ++;

  if (this->is_sync) {
    fprintf(stderr, "T<%p>: Calling sync(%d)\n", (void *)this, cnt);
    std::vector<Handle<Value>> argv_v8 = pre_process_args(argv);
    argv_v8.push_back(NanNew(wrapper));

    auto a = this->post_process_return_value(
      NanNew<Value>(this->callback->Call(argv_v8.size(), &argv_v8[0]))
    );
    fprintf(stderr, "T<%p>: ... sync returned\n", (void *)this);
    return a;
  }

  this->argv = argv;
  this->has_returned = false;
  fprintf(stderr, "T<%p>: Calling async(%d)\n", (void *)this, cnt);
  uv_barrier_init(&this->blocker, 4);
  uv_queue_work(uv_default_loop(), this->async, (uv_work_cb)queued_work, (uv_after_work_cb)completed_work);
//  fprintf(stderr, "T<%p>: Calling async(%d)\n", (void *)this, cnt);
//  uv_async_send(this->async);
  fprintf(stderr, "T<%p>: About to wait for %p\n", (void *)this, (void *)this->async);
  if (uv_barrier_wait(&this->blocker) > 0)
    uv_barrier_destroy(&this->blocker);
  return this->return_value;
}

template <typename T, typename L>
void CallbackBridge<T, L>::queued_work(uv_work_t *req) {
  CallbackBridge* bridge = static_cast<CallbackBridge*>(req->data);

  fprintf(stderr, "T<%p>: Running handle %p\n", (void *)bridge, (void *)req);
  NanScope();
  TryCatch try_catch;

  std::vector<Handle<Value>> argv_v8 = bridge->pre_process_args(bridge->argv);
  argv_v8.push_back(NanNew(bridge->wrapper));

  NanNew<Value>(bridge->callback->Call(argv_v8.size(), &argv_v8[0]));

  fprintf(stderr, "T<%p>: NOT Closing handle(%d) %p\n", (void *)bridge, bridge->cnt, (void *)req);
  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }
}

template <typename T, typename L>
void CallbackBridge<T, L>::completed_work(uv_work_t *req, int status) {
  CallbackBridge* bridge = static_cast<CallbackBridge*>(req->data);

  fprintf(stderr, "T<%p>: Completed work %p, status=%d\n", (void *)bridge, (void *)req, status);
}

template <typename T, typename L>
NAN_METHOD(CallbackBridge<T COMMA L>::ReturnCallback) {
  NanScope();

  CallbackBridge<T, L>* bridge = static_cast<CallbackBridge<T, L>*>(NanGetInternalFieldPointer(args.This(), 0));
  TryCatch try_catch;

  fprintf(stderr, "T<%p>: ReturnCallback(%d)\n", (void *)bridge, bridge->cnt);
  bridge->return_value = bridge->post_process_return_value(args[0]);

  if (uv_barrier_wait(&bridge->blocker) > 0)
    uv_barrier_destroy(&bridge->blocker);
  fprintf(stderr, "T<%p>: barrier open\n", (void *)bridge);

  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }

  NanReturnUndefined();
}

template <typename T, typename L>
Handle<Function> CallbackBridge<T, L>::get_wrapper_constructor() {
  if (wrapper_constructor.IsEmpty()) {
    fprintf(stderr, "wrapper_constructor empty\n");
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
    tpl->SetClassName(NanNew("CallbackBridge"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->PrototypeTemplate()->Set(
      NanNew("success"),
      NanNew<FunctionTemplate>(ReturnCallback)->GetFunction()
    );

    NanAssignPersistent(wrapper_constructor, tpl->GetFunction());
  } else {
    fprintf(stderr, "wrapper_constructor NOT empty\n");
  }

  return NanNew(wrapper_constructor);
}

template <typename T, typename L>
NAN_METHOD(CallbackBridge<T COMMA L>::New) {
  NanScope();
  fprintf(stderr, "CallbackBridge<T,L>::New\n");
  NanReturnValue(args.This());
}

template <typename T, typename L>
void CallbackBridge<T, L>::async_gone(uv_handle_t *handle) {
  delete (uv_work_t *)handle;
}

#endif
