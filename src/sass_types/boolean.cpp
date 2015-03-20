#include <nan.h>
#include "boolean.h"
#include <sass_values.h>
#include "core_value.h"

using namespace v8;

namespace SassTypes
{
  Boolean::Boolean(Sass_Value* v) : CoreValue(v) {}

  Sass_Value* Boolean::construct(const int len, const std::vector<Local<v8::Value>> raw_val) {
    bool value = false;

    if (len >= 1) {
      if (!raw_val[0]->IsBoolean()) {
        throw std::invalid_argument("Argument should be a bool");
      }

      value = raw_val[0]->ToBoolean()->Value();
    }

    return sass_make_boolean(value);
  }

  void Boolean::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getValue"), NanNew<FunctionTemplate>(GetValue)->GetFunction());
    proto->Set(NanNew("setValue"), NanNew<FunctionTemplate>(SetValue)->GetFunction());
  }

  NAN_METHOD(Boolean::GetValue) {
    NanReturnValue(NanNew(sass_boolean_get_value(unwrap(args.This())->value)));
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
