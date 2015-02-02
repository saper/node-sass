#include <nan.h>
#include "string.h"
#include "../libsass/sass_values.h"
#include "../create_string.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  String::String(Sass_Value* v) : CoreValue(v) {}


  Sass_Value* String::construct(const Arguments& args) {
    char const* value = "";
    unsigned argc = args.Length();

    if (argc >= 1) {
      if (!args[0]->IsString()) {
        throw std::invalid_argument("Argument should be a string");
      }

      value = CreateString(args[0]);
    }

    return sass_make_string(value);
  }


  void String::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
    proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
  }


  NAN_METHOD(String::GetValue) {
    return NanNew(sass_string_get_value(unwrap(args.This())->value));
  }


  NAN_METHOD(String::SetValue) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsString()) {
      return NanThrowError(NanNew("Supplied value should be a string"));
    }

    sass_string_set_value(unwrap(args.This())->value, CreateString(args[0]));
    NanReturnUndefined();
  }
}