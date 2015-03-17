#include <nan.h>
#include "color.h"
#include <sass_values.h>
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Color::Color(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* Color::construct(const Arguments& args) {
    double a = 1.0, r = 0, g = 0, b = 0;
    unsigned argc = args.Length(), argb;

    switch (argc) {
      case 1:
        if (!args[0]->IsNumber()) {
          throw std::invalid_argument("Only argument should be an integer");
        }

        argb = args[0]->ToInt32()->Value();
        a = (double) ((argb >> 030) & 0xff) / 0xff;
        r = (double) ((argb >> 020) & 0xff);
        g = (double) ((argb >> 010) & 0xff);
        b = (double) (argb & 0xff);
        break;

      case 4:
        if (!args[3]->IsNumber()) {
          throw std::invalid_argument("Constructor arguments should be numbers exclusively");
        }

        a = args[3]->ToNumber()->Value();
        // fall through vvv

      case 3:
        if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber()) {
          throw std::invalid_argument("Constructor arguments should be numbers exclusively");
        }

        r = args[0]->ToNumber()->Value();
        g = args[1]->ToNumber()->Value();
        b = args[2]->ToNumber()->Value();
        break;

      case 0:
        break;

      default:
        throw std::invalid_argument("Constructor should be invoked with either 0, 1, 3 or 4 arguments");
    }

    return sass_make_color(r, g, b, a);
  }


  void Color::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getR"), FunctionTemplate::New(GetR)->GetFunction());
    proto->Set(NanNew("getG"), FunctionTemplate::New(GetG)->GetFunction());
    proto->Set(NanNew("getB"), FunctionTemplate::New(GetB)->GetFunction());
    proto->Set(NanNew("getA"), FunctionTemplate::New(GetA)->GetFunction());
    proto->Set(NanNew("setR"), FunctionTemplate::New(SetR)->GetFunction());
    proto->Set(NanNew("setG"), FunctionTemplate::New(SetG)->GetFunction());
    proto->Set(NanNew("setB"), FunctionTemplate::New(SetB)->GetFunction());
    proto->Set(NanNew("setA"), FunctionTemplate::New(SetA)->GetFunction());
  }


  NAN_METHOD(Color::GetR) {
    return NanNew(sass_color_get_r(unwrap(args.This())->value));
  }


  NAN_METHOD(Color::GetG) {
    return NanNew(sass_color_get_g(unwrap(args.This())->value));
  }


  NAN_METHOD(Color::GetB) {
    return NanNew(sass_color_get_b(unwrap(args.This())->value));
  }


  NAN_METHOD(Color::GetA) {
    return NanNew(sass_color_get_a(unwrap(args.This())->value));
  }


  NAN_METHOD(Color::SetR) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied value should be a number"));
    }

    sass_color_set_r(unwrap(args.This())->value, args[0]->ToNumber()->Value());
    NanReturnUndefined();
  }


  NAN_METHOD(Color::SetG) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied value should be a number"));
    }

    sass_color_set_g(unwrap(args.This())->value, args[0]->ToNumber()->Value());
    NanReturnUndefined();
  }


  NAN_METHOD(Color::SetB) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied value should be a number"));
    }

    sass_color_set_b(unwrap(args.This())->value, args[0]->ToNumber()->Value());
    NanReturnUndefined();
  }


  NAN_METHOD(Color::SetA) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied value should be a number"));
    }

    sass_color_set_a(unwrap(args.This())->value, args[0]->ToNumber()->Value());
    NanReturnUndefined();
  }
}
