#include <nan.h>
#include "error.h"
#include <sass_values.h>
#include "../create_string.h"
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Error::Error(Sass_Value* v) : CoreValue(v) {}


  Sass_Value* Error::construct(const Arguments& args) {
    char const* value = "";
    unsigned argc = args.Length();

    if (argc >= 1) {
      if (!args[0]->IsString()) {
        throw std::invalid_argument("Argument should be a string");
      }

      value = CreateString(args[0]);
    }

    return sass_make_error(value);
  }


  void Error::initPrototype(Handle<ObjectTemplate>) {}
}
