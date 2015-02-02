#include <nan.h>
#include "map.h"
#include "../libsass/sass_values.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Map::Map(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* Map::construct(const Arguments& args) {
    size_t length = 0;
    unsigned argc = args.Length();

    if (argc >= 1) {
      if (!args[0]->IsNumber()) {
        throw std::invalid_argument("First argument should be an integer");
      }

      length = args[0]->ToInt32()->Value();
    }

    return sass_make_map(length);
  }


  void Map::initPrototype(Handle<ObjectTemplate> proto) {
    proto->Set(NanNew("getLength"), FunctionTemplate::New(GetLength)->GetFunction());
    proto->Set(NanNew("getKey"), FunctionTemplate::New(GetKey)->GetFunction());
    proto->Set(NanNew("setKey"), FunctionTemplate::New(SetKey)->GetFunction());
    proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
    proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
  }


  NAN_METHOD(Map::GetValue) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied index should be an integer"));
    }

    Sass_Value* map = unwrap(args.This())->value;
    size_t index = args[0]->ToInt32()->Value();


    if (index >= sass_map_get_length(map)) {
      return NanThrowError(NanNew("Out of bound index"));
    }

    return Factory::create(sass_map_get_value(map, args[0]->ToInt32()->Value()))->get_js_object();
  }


  NAN_METHOD(Map::SetValue) {
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
    sass_map_set_value(unwrap(args.This())->value, args[0]->ToInt32()->Value(), sass_value->get_sass_value());
    NanReturnUndefined();
  }


  NAN_METHOD(Map::GetKey) {
    if (args.Length() != 1) {
      return NanThrowError(NanNew("Expected just one argument"));
    }

    if (!args[0]->IsNumber()) {
      return NanThrowError(NanNew("Supplied index should be an integer"));
    }

    Sass_Value* map = unwrap(args.This())->value;
    size_t index = args[0]->ToInt32()->Value();


    if (index >= sass_map_get_length(map)) {
      return NanThrowError(NanNew("Out of bound index"));
    }

    return Factory::create(sass_map_get_key(map, args[0]->ToInt32()->Value()))->get_js_object();
  }


  NAN_METHOD(Map::SetKey) {
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
    sass_map_set_key(unwrap(args.This())->value, args[0]->ToInt32()->Value(), sass_value->get_sass_value());
    NanReturnUndefined();
  }


  NAN_METHOD(Map::GetLength) {
    return NanNew<v8::Number>(sass_map_get_length(unwrap(args.This())->value));
  }
}