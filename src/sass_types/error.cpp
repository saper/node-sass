#include <nan.h>
#include "error.h"
#include <sass_values.h>
#include "../create_string.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes
{
  Error::Error(Sass_Value* v) : CoreValue(v) {}


  Sass_Value* Error::construct(const int len, const std::vector<Local<v8::Value>> raw_val) {
    char const* value = "";

    if (len >= 1) {
      if (!raw_val[0]->IsString()) {
        throw std::invalid_argument("Argument should be a string");
      }

      value = CreateString(raw_val[0]);
    }

    return sass_make_error(value);
  }


  void Error::initPrototype(Handle<ObjectTemplate>) {}
}
