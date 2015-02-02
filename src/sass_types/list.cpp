#include <nan.h>
#include "list.h"
#include "../libsass/sass_values.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  List::List(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* List::construct(const Arguments& args) {
    size_t length = 0;
    bool comma = true;
    unsigned argc = args.Length();

    if (argc >= 1) {
      if (!args[0]->IsNumber()) {
        throw std::invalid_argument("First argument should be an integer");
      }

      length = args[0]->ToInt32()->Value();

      if (argc >= 2) {
        if (!args[1]->IsBoolean()) {
          throw std::invalid_argument("Second argument should be a boolean");
        }

        comma = args[1]->ToBoolean()->Value();
      }
    }

    return sass_make_list(length, comma ? SASS_COMMA : SASS_SPACE);
  }


  void List::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getLength"), FunctionTemplate::New(GetLength)->GetFunction());
    proto->Set(NanNew("getSeparator"), FunctionTemplate::New(GetSeparator)->GetFunction());
    proto->Set(NanNew("setSeparator"), FunctionTemplate::New(SetSeparator)->GetFunction());
    proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
    proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
  }


  NAN_METHOD(List::GetValue) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied index should be an integer"));
    }

    Sass_Value* list = unwrap(args.This())->value;
    size_t index = args[0]->ToInt32()->Value();


    if (index >= sass_list_get_length(list)) {
      return NanThrowError(NanNew("Out of bound index"));
    }

    return Factory::create(sass_list_get_value(list, args[0]->ToInt32()->Value()))->get_js_object();
  }


  NAN_METHOD(List::SetValue) {
    if (args.Length() != 2) {
      return NanThrowError(NanNew("Expected two arguments"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied index should be an integer"));
    }

    if (!args[1]->IsObject()) {
      return NanThrowError(NanNew("Supplied value should be a SassValue object"));
    }

    Value* sass_value = Factory::unwrap(args[1]);
    sass_list_set_value(unwrap(args.This())->value, args[0]->ToInt32()->Value(), sass_value->get_sass_value());
    NanReturnUndefined();
  }


  NAN_METHOD(List::GetSeparator) {
    return NanNew(sass_list_get_separator(unwrap(args.This())->value) == SASS_COMMA);
  }


  NAN_METHOD(List::SetSeparator) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsBoolean()) {
      return NanThrowError(NanNew("Supplied value should be a boolean"));
    }

    sass_list_set_separator(unwrap(args.This())->value, args[0]->ToBoolean()->Value() ? SASS_COMMA : SASS_SPACE);
    NanReturnUndefined();
  }


  NAN_METHOD(List::GetLength) {
    return NanNew<v8::Number>(sass_list_get_length(unwrap(args.This())->value));
  }
}