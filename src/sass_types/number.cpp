#include <nan.h>
#include "number.h"
#include <sass_values.h>
#include "../create_string.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Number::Number(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* Number::construct(const Arguments& args) {
    double value = 0;
    char const* unit = "";
    unsigned argc = args.Length();

    if (argc >= 1) {
        if (!args[0]->IsNumber()) {
          throw std::invalid_argument("First argument should be a number");
        }

        value = args[0]->ToNumber()->Value();

        if (argc >= 2) {
          if (!args[1]->IsString()) {
            throw std::invalid_argument("Second argument should be a string");
          }

          unit = CreateString(args[1]);
        }
    }

    return sass_make_number(value, unit);
  }


  void Number::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
    proto->Set(NanNew("getUnit"), FunctionTemplate::New(GetUnit)->GetFunction());
    proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
    proto->Set(NanNew("setUnit"), FunctionTemplate::New(SetUnit)->GetFunction());
  }


  NAN_METHOD(Number::GetValue) {
    return NanNew(sass_number_get_value(unwrap(args.This())->value));
  }


  NAN_METHOD(Number::GetUnit) {
    return NanNew(sass_number_get_unit(unwrap(args.This())->value));
  }


  NAN_METHOD(Number::SetValue) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied value should be a number"));
    }

    sass_number_set_value(unwrap(args.This())->value, args[0]->ToNumber()->Value());
    NanReturnUndefined();
  }


  NAN_METHOD(Number::SetUnit) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsString()) {
      return NanThrowError(NanNew("Supplied value should be a string"));
    }

    sass_number_set_unit(unwrap(args.This())->value, CreateString(args[0]));
    NanReturnUndefined();
  }
}
