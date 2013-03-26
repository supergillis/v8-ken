#ifndef V8_KEN_V8_H
#define V8_KEN_V8_H

#include "v8.h"

namespace v8 {
  namespace ken {
    void Initialize(Handle<Object> object);

    const char* ToCString(Handle<String> value);
    const char* ToCString(const String::Utf8Value& value);

    Handle<Value> JSONStringify(Handle<Value> object);
    Handle<Value> JSONParse(Handle<String> json);

    Handle<Value> Eval(const char* source, int32_t length);
    Handle<Value> Eval(Handle<String> source);

    Handle<Value> HandleReceive(Handle<String> sender, Handle<String> message);

    void ReportException(TryCatch* tryCatch);
  }
}

#endif
