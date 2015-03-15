#ifndef SASS_TYPES_STRING_H
#define SASS_TYPES_STRING_H

#include <nan.h>
#include <sass_values.h>
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class String : public CoreValue<String> {
    public:
      String(Sass_Value*);
      static constexpr char const* constructor_name = "SassString";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
  };
}


#endif
