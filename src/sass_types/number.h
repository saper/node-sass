#ifndef SASS_TYPES_NUMBER_H
#define SASS_TYPES_NUMBER_H

#include <nan.h>
#include "../libsass/sass_values.h"
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class Number : public CoreValue<Number> {
    public:
      Number(Sass_Value*);
      static constexpr char const* constructor_name = "SassNumber";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);

      static NAN_METHOD(GetValue);
      static NAN_METHOD(GetUnit);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(SetUnit);
  };
}


#endif