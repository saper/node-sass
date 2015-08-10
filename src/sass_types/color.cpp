#include <nan.h>
#include "color.h"

namespace SassTypes
{
  Color::Color(Sass_Value* v) : SassValueWrapper(v) {}

  Sass_Value* Color::construct(Nan::Maybe <double> r, 
                               Nan::Maybe <double> g, 
                               Nan::Maybe <double> b,
                               Nan::Maybe <double> a)
  {
    if (r.IsNothing())
        throw std::invalid_argument("Cannot determine red color value");
    if (g.IsNothing())
        throw std::invalid_argument("Cannot determine green color value");
    if (b.IsNothing())
        throw std::invalid_argument("Cannot determine blue color value");
    if (a.IsNothing())
        throw std::invalid_argument("Cannot determine alpha color value");
    return sass_make_color(r.FromJust(), g.FromJust(), b.FromJust(), a.FromJust());
  }


  Sass_Value* Color::construct(Nan::Maybe <double> r, 
                               Nan::Maybe <double> g, 
                               Nan::Maybe <double> b)
  {
    if (r.IsNothing())
        throw std::invalid_argument("Cannot determine red color value");
    if (g.IsNothing())
        throw std::invalid_argument("Cannot determine green color value");
    if (b.IsNothing())
        throw std::invalid_argument("Cannot determine blue color value");
    return sass_make_color(r.FromJust(), g.FromJust(), b.FromJust(), 1.0);
  }


  Sass_Value* Color::construct(Nan::Maybe <int32_t> argb)
  {
    if (argb.IsJust()) {  
      return sass_make_color(
        (double)((argb.FromJust() >> 020) & 0xff),
        (double)((argb.FromJust() >> 010) & 0xff),
        (double) (argb.FromJust() & 0xff),
        (double)((argb.FromJust() >> 030) & 0xff) / 0xff);
    } else {
      throw std::invalid_argument("The only argument should be an integer representing RGBA color.");
    }
  }

  Sass_Value* Color::construct(void) {
    return sass_make_color(0.0, 0.0, 0.0, 1.0);
  }

  Sass_Value* Color::construct(const std::vector<v8::Local<v8::Value>> raw_val) {

    switch (raw_val.size()) {
     case 1:
      return Color::construct(Nan::To<int32_t>(raw_val[0]));

    case 4:
      return Color::construct(Nan::To<double>(raw_val[0]),
                              Nan::To<double>(raw_val[1]),
                              Nan::To<double>(raw_val[2]),
                              Nan::To<double>(raw_val[3]));
      
    case 3:
      return Color::construct(Nan::To<double>(raw_val[0]),
                              Nan::To<double>(raw_val[1]),
                              Nan::To<double>(raw_val[2]));

    case 0:
      return Color::construct();

    default:
      throw std::invalid_argument("Constructor should be invoked with either 0, 1, 3 or 4 arguments.");
    }
  }

  void Color::initPrototype(v8::Local<v8::FunctionTemplate> proto) {
    Nan::SetPrototypeMethod(proto, "getR", GetR);
    Nan::SetPrototypeMethod(proto, "getG", GetG);
    Nan::SetPrototypeMethod(proto, "getB", GetB);
    Nan::SetPrototypeMethod(proto, "getA", GetA);
    Nan::SetPrototypeMethod(proto, "setR", SetR);
    Nan::SetPrototypeMethod(proto, "setG", SetG);
    Nan::SetPrototypeMethod(proto, "setB", SetB);
    Nan::SetPrototypeMethod(proto, "setA", SetA);
  }

  NAN_METHOD(Color::GetR) {
    info.GetReturnValue().Set(sass_color_get_r(unwrap(info.This())->value));
  }

  NAN_METHOD(Color::GetG) {
    info.GetReturnValue().Set(sass_color_get_g(unwrap(info.This())->value));
  }

  NAN_METHOD(Color::GetB) {
    info.GetReturnValue().Set(sass_color_get_b(unwrap(info.This())->value));
  }

  NAN_METHOD(Color::GetA) {
    info.GetReturnValue().Set(sass_color_get_a(unwrap(info.This())->value));
  }

  NAN_METHOD(Color::SetR) {
    if (info.Length() != 1) {
      return Nan::ThrowError(Nan::New("Expected just one argument").ToLocalChecked());
    }

    if (!info[0]->IsNumber()) {
      return Nan::ThrowError(Nan::New("Supplied value should be a number").ToLocalChecked());
    }

    sass_color_set_r(unwrap(info.This())->value, info[0]->ToNumber()->Value());
  }

  NAN_METHOD(Color::SetG) {
    if (info.Length() != 1) {
      return Nan::ThrowError(Nan::New("Expected just one argument").ToLocalChecked());
    }

    if (!info[0]->IsNumber()) {
      return Nan::ThrowError(Nan::New("Supplied value should be a number").ToLocalChecked());
    }

    sass_color_set_g(unwrap(info.This())->value, info[0]->ToNumber()->Value());
  }

  NAN_METHOD(Color::SetB) {
    if (info.Length() != 1) {
      return Nan::ThrowError(Nan::New("Expected just one argument").ToLocalChecked());
    }

    if (!info[0]->IsNumber()) {
      return Nan::ThrowError(Nan::New("Supplied value should be a number").ToLocalChecked());
    }

    sass_color_set_b(unwrap(info.This())->value, info[0]->ToNumber()->Value());
  }

  NAN_METHOD(Color::SetA) {
    if (info.Length() != 1) {
      return Nan::ThrowError(Nan::New("Expected just one argument").ToLocalChecked());
    }

    if (!info[0]->IsNumber()) {
      return Nan::ThrowError(Nan::New("Supplied value should be a number").ToLocalChecked());
    }

    sass_color_set_a(unwrap(info.This())->value, info[0]->ToNumber()->Value());
  }
}
