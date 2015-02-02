#ifndef SASS_TYPES_LIST_H
#define SASS_TYPES_LIST_H

#include <nan.h>
#include "../libsass/sass_values.h"
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class List : public CoreValue<List> {
    public:
      List(Sass_Value*);
      static constexpr char const* constructor_name = "SassList";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(GetSeparator);
      static NAN_METHOD(SetSeparator);
      static NAN_METHOD(GetLength);
  };
}


#endif