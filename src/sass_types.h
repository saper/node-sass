#ifndef SASS_TYPES_H
#define SASS_TYPES_H

#include <nan.h>
#include <vector>
#include "sass_context_wrapper.h"


namespace SassTypes 
{
  // This is the interface that all sass values must comply with
  class Value {
    public:
      Value(Sass_Value* v) { this->value = sass_clone_value(v); }
      virtual ~Value() { sass_delete_value(this->value); }
      virtual Sass_Value* get_sass_value() =0;
      virtual Local<Object> get_js_object() =0;

    protected:
      Sass_Value* value;
  };


  // This is the guru that knows everything about instantiating the right subclass of SassTypes::Value
  // to wrap a given Sass_Value object.
  class Factory {
    public:
      static void initExports(Handle<Object>);
      static Value* create(Sass_Value*);
      static Value* unwrap(Handle<v8::Value>);
  };


  // Include this in any SassTypes::Value subclasses to handle all the heavy lifting of constructing JS
  // objects and wrapping sass values inside them
  template <class T>
  class CoreValue : public Value {
    public:
      static constexpr char const* constructor_name = "SassValue";

      CoreValue(Sass_Value* v) : Value(v) {}

      Sass_Value* get_sass_value() { 
        return sass_clone_value(this->value); 
      }

      Local<Object> get_js_object() {
        Local<Object> wrapper = NanNew(T::get_constructor())->NewInstance();

        delete static_cast<T*>(NanGetInternalFieldPointer(wrapper, 0));
        NanSetInternalFieldPointer(wrapper, 0, this);

        return wrapper;
      }

      static Handle<Function> get_constructor() {
        if (constructor.IsEmpty()) {
          Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
          tpl->SetClassName(NanNew(T::constructor_name));
          tpl->InstanceTemplate()->SetInternalFieldCount(1);
          T::initPrototype(tpl->PrototypeTemplate());
          constructor = Persistent<Function>::New(tpl->GetFunction());
        }

        return constructor;
      }

      static NAN_METHOD(New) {
        if (!args.IsConstructCall()) {
          unsigned argc = args.Length();
          std::vector<Handle<v8::Value>> argv;

          argv.reserve(argc);
          for (unsigned i = 0; i < argc; i++) {
            argv.push_back(args[i]);
          }

          return NanNew(T::get_constructor())->NewInstance(argc, &argv[0]);
        }

        Sass_Value* value = T::construct(args);
        T* obj = new T(value);
        sass_delete_value(value);

        NanSetInternalFieldPointer(args.This(), 0, obj);

        return args.This();
      }

    protected:
      static T* unwrap(Local<Object> obj) {
        return static_cast<T*>(Factory::unwrap(obj));
      }

    private:
      static Persistent<Function> constructor;
  };


  template <class T>
  Persistent<Function> CoreValue<T>::constructor;


  class Number : public CoreValue<Number> {
    public:
      Number(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassNumber";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
        proto->Set(NanNew("getUnit"), FunctionTemplate::New(GetUnit)->GetFunction());
        proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
        proto->Set(NanNew("setUnit"), FunctionTemplate::New(SetUnit)->GetFunction());
      }

      static NAN_METHOD(GetValue);
      static NAN_METHOD(GetUnit);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(SetUnit);
  };


  class String : public CoreValue<String> {
    public:
      String(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassString";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
        proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
      }

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
  };


  class Color : public CoreValue<Color> {
    public:
      Color(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassColor";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getR"), FunctionTemplate::New(GetR)->GetFunction());
        proto->Set(NanNew("getG"), FunctionTemplate::New(GetG)->GetFunction());
        proto->Set(NanNew("getB"), FunctionTemplate::New(GetB)->GetFunction());
        proto->Set(NanNew("getA"), FunctionTemplate::New(GetA)->GetFunction());
        proto->Set(NanNew("setR"), FunctionTemplate::New(SetR)->GetFunction());
        proto->Set(NanNew("setG"), FunctionTemplate::New(SetG)->GetFunction());
        proto->Set(NanNew("setB"), FunctionTemplate::New(SetB)->GetFunction());
        proto->Set(NanNew("setA"), FunctionTemplate::New(SetA)->GetFunction());
      }

      static NAN_METHOD(GetR);
      static NAN_METHOD(GetG);
      static NAN_METHOD(GetB);
      static NAN_METHOD(GetA);
      static NAN_METHOD(SetR);
      static NAN_METHOD(SetG);
      static NAN_METHOD(SetB);
      static NAN_METHOD(SetA);
  };


  class Boolean : public CoreValue<Boolean> {
    public:
      Boolean(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassBoolean";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
        proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
      }

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
  };


  class List : public CoreValue<List> {
    public:
      List(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassList";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getLength"), FunctionTemplate::New(GetLength)->GetFunction());
        proto->Set(NanNew("getSeparator"), FunctionTemplate::New(GetSeparator)->GetFunction());
        proto->Set(NanNew("setSeparator"), FunctionTemplate::New(SetSeparator)->GetFunction());
        proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
        proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
      }

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(GetSeparator);
      static NAN_METHOD(SetSeparator);
      static NAN_METHOD(GetLength);
  };


  class Map : public CoreValue<Map> {
    public:
      Map(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassMap";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {
        proto->Set(NanNew("getLength"), FunctionTemplate::New(GetLength)->GetFunction());
        proto->Set(NanNew("getKey"), FunctionTemplate::New(GetKey)->GetFunction());
        proto->Set(NanNew("setKey"), FunctionTemplate::New(SetKey)->GetFunction());
        proto->Set(NanNew("getValue"), FunctionTemplate::New(GetValue)->GetFunction());
        proto->Set(NanNew("setValue"), FunctionTemplate::New(SetValue)->GetFunction());
      }

      static NAN_METHOD(GetValue);
      static NAN_METHOD(SetValue);
      static NAN_METHOD(GetKey);
      static NAN_METHOD(SetKey);
      static NAN_METHOD(GetLength);
  };


  class Null : public CoreValue<Null> {
    public:
      Null(Sass_Value* v) : CoreValue(v) {}
      static constexpr char const* constructor_name = "SassNull";
      static Sass_Value* construct(const Arguments&);

      static void initPrototype(Handle<ObjectTemplate> proto) {}
  };
}

#endif