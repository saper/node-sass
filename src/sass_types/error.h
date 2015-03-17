#ifndef SASS_TYPES_ERROR_H
#define SASS_TYPES_ERROR_H

#include <nan.h>
#include <sass_values.h>
#include "core_value.h"


namespace SassTypes 
{
  using namespace v8;
  
  class Error : public CoreValue<Error> {
    public:
      Error(Sass_Value*);
      static constexpr char const* constructor_name = "SassError";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate>);
  };
}


#endif
