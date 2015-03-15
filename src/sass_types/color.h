#ifndef SASS_TYPES_COLOR_H
#define SASS_TYPES_COLOR_H

#include <nan.h>
#include <sass_values.h>
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class Color : public CoreValue<Color> {
    public:
      Color(Sass_Value*);
      static constexpr char const* constructor_name = "SassColor";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);

      static NAN_METHOD(GetR);
      static NAN_METHOD(GetG);
      static NAN_METHOD(GetB);
      static NAN_METHOD(GetA);
      static NAN_METHOD(SetR);
      static NAN_METHOD(SetG);
      static NAN_METHOD(SetB);
      static NAN_METHOD(SetA);
  };
}


#endif
