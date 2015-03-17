#ifndef SASS_TYPES_CORE_VALUE_H
#define SASS_TYPES_CORE_VALUE_H

#include <nan.h>
#include <sass_values.h>
#include "value.h"
#include "factory.h"


namespace SassTypes 
{
  using namespace v8;

  // Include this in any SassTypes::Value subclasses to handle all the heavy lifting of constructing JS
  // objects and wrapping sass values inside them
  template <class T>
  class CoreValue : public Value {
    public:
      static constexpr char const* constructor_name = "SassValue";

      CoreValue(Sass_Value*);

      Sass_Value* get_sass_value();
      Local<Object> get_js_object();

      static Handle<Function> get_constructor();
      static NAN_METHOD(New);

    protected:
      static T* unwrap(Local<Object>);

    private:
      static Persistent<Function> constructor;
  };

  
  template <class T>
  Persistent<Function> CoreValue<T>::constructor;


  template <class T>
  CoreValue<T>::CoreValue(Sass_Value* v) : Value(v) {}


  template <class T>
  Sass_Value* CoreValue<T>::get_sass_value() { 
    return sass_clone_value(this->value); 
  }


  template <class T>
  Local<Object> CoreValue<T>::get_js_object() {
    Local<Object> wrapper = NanNew(T::get_constructor())->NewInstance();

    delete static_cast<T*>(NanGetInternalFieldPointer(wrapper, 0));
    NanSetInternalFieldPointer(wrapper, 0, this);

    return wrapper;
  }


  template <class T>
  Handle<Function> CoreValue<T>::get_constructor() {
    if (constructor.IsEmpty()) {
      Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
      tpl->SetClassName(NanNew(T::constructor_name));
      tpl->InstanceTemplate()->SetInternalFieldCount(1);
      T::initPrototype(tpl->PrototypeTemplate());
      constructor = Persistent<Function>::New(tpl->GetFunction());
    }

    return constructor;
  }


  template <class T>
  NAN_METHOD(CoreValue<T>::New) {
    if (!args.IsConstructCall()) {
      unsigned argc = args.Length();
      std::vector<Handle<v8::Value>> argv;

      argv.reserve(argc);
      for (unsigned i = 0; i < argc; i++) {
        argv.push_back(args[i]);
      }

      return NanNew(T::get_constructor())->NewInstance(argc, &argv[0]);
    }

    try {
      Sass_Value* value = T::construct(args);
      T* obj = new T(value);
      sass_delete_value(value);

      NanSetInternalFieldPointer(args.This(), 0, obj);

      return args.This();
    } catch (const std::exception& e) {
      return NanThrowError(NanNew(e.what()));
    }
  }


  template <class T>
  T* CoreValue<T>::unwrap(Local<Object> obj) {
    return static_cast<T*>(Factory::unwrap(obj));
  }
}


#endif
