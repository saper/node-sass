#include <nan.h>
#include "null.h"
#include <sass_values.h>
#include "core_value.h"


using namespace v8;


namespace SassTypes 
{
  Null::Null(Sass_Value* v) : CoreValue(v) {}
  

  Sass_Value* Null::construct(const Arguments& args) {
    return sass_make_null();
  }


  void Null::initPrototype(Handle<ObjectTemplate>) {}
}
