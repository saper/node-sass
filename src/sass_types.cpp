#include <nan.h>
#include "sass_context_wrapper.h"
#include "sass_types.h"
#include "create_string.h"

namespace SassTypes 
{
  Value* Factory::create(Sass_Value* v) {
    switch (sass_value_get_tag(v)) {
      case SASS_NUMBER:
        return new Number(v);

      case SASS_STRING:
        return new String(v);

      case SASS_COLOR:
        return new Color(v);

      case SASS_BOOLEAN:
        return new Boolean(v);

      case SASS_LIST:
        return new List(v);

      case SASS_MAP:
        return new Map(v);

      case SASS_NULL:
        return new Null(v);

      default:
        throw std::invalid_argument("Unknown type");
    }
  }


  void Factory::initExports(Handle<Object> exports) {
    Local<Object> types = Object::New();
    exports->Set(NanNew("types"), types);

    types->Set(NanNew("Number"), Number::get_constructor());
    types->Set(NanNew("String"), String::get_constructor());
    types->Set(NanNew("Color"), Color::get_constructor());
    types->Set(NanNew("Boolean"), Boolean::get_constructor());
    types->Set(NanNew("List"), List::get_constructor());
    types->Set(NanNew("Map"), Map::get_constructor());
    types->Set(NanNew("Null"), Null::get_constructor());
  }


  Value* Factory::unwrap(Handle<v8::Value> obj) {
    // Todo: non-SassValue objects could easily fall under that condition, need to be more specific.
    if (!obj->IsObject() || obj->ToObject()->InternalFieldCount() != 1) {
      throw std::invalid_argument("A SassValue object was expected");
    }

    return static_cast<Value*>(NanGetInternalFieldPointer(obj->ToObject(), 0));
  }


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


  Sass_Value* Null::construct(const Arguments& args) {
    return sass_make_null();
  }
}