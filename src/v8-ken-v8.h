#ifndef V8_KEN_V8_H
#define V8_KEN_V8_H

#include "v8.h"

namespace v8 {
  namespace ken {
    void Initialize(Handle<Object> object);

    const char* ToCString(const String::Utf8Value& value);

    bool Eval(const char* source, int32_t length);
    bool Eval(Handle<String> source);

    bool HandleException(TryCatch* tryCatch);
    bool HandleReceive(const char* sender, const char* string, int32_t length);
  }
}

#endif
