#ifndef V8_KEN_V8_H
#define V8_KEN_V8_H

#include "v8.h"

namespace v8 {
namespace ken {

class V8 {
public:
  V8() :
      handle_scope_(),
      global_(v8::ObjectTemplate::New()),
      context_(v8::Context::New(NULL, global_)) {
    context_->Enter();
  }

  ~V8() {
    context_.Dispose();
  }

  v8::Handle<v8::ObjectTemplate> global() {
    return global_;
  }

  v8::Persistent<v8::Context> context() {
    return context_;
  }

private:
  v8::HandleScope handle_scope_;
  v8::Handle<v8::ObjectTemplate> global_;
  v8::Persistent<v8::Context> context_;
};

}
}

#endif
