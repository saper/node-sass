#ifndef SASS_TYPES_NULL_H
#define SASS_TYPES_NULL_H

#include <nan.h>
#include <sass_values.h>
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class Null : public CoreValue<Null> {
    public:
      Null(Sass_Value* v);
      static constexpr char const* constructor_name = "SassNull";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);
  };
}


#endif
