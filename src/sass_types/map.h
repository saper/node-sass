#ifndef SASS_TYPES_MAP_H
#define SASS_TYPES_MAP_H

#include <nan.h>
#include "../libsass/sass_values.h"
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class Map : public CoreValue<Map> {
    public:
      Map(Sass_Value*);
      static constexpr char const* constructor_name = "SassMap";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(GetKey);
      static NAN_METHOD(SetKey);
      static NAN_METHOD(GetLength);
  };
}


#endif