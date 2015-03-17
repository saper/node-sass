#include <nan.h>
#include "boolean.h"
#include <sass_values.h>
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Boolean::Boolean(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* Boolean::construct(const Arguments& args) {
    bool value = false;
    unsigned argc = args.Length();

    if (argc >= 1) {
      if (!args[0]->IsBoolean()) {
        throw std::invalid_argument("Argument should be a bool");
      }

      value = args[0]->ToBoolean()->Value();
    }

    return sass_make_boolean(value);
  }


  void Boolean::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
    proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
  }


  NAN_METHOD(Boolean::GetValue) {
    return NanNew(sass_boolean_get_value(unwrap(args.This())->value));
  }


  NAN_METHOD(Boolean::SetValue) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsBoolean()) {
      return NanThrowError(NanNew("Supplied value should be a boolean"));
    }

    sass_boolean_set_value(unwrap(args.This())->value, args[0]->ToBoolean()->Value());
    NanReturnUndefined();
  }
}
