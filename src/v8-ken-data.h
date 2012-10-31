#ifndef V8_KEN_DATA_H
#define V8_KEN_DATA_H

#include "v8-ken.h"
#include "isolate.h"

using namespace v8::internal;

class V8Data {
public:
  V8Data():
    handle_scope_(),
    global_(v8::ObjectTemplate::New()),
    context_(v8::Context::New(NULL, global_)) {
    context_->Enter();
  }

  ~V8Data() {
    context_.Dispose();
  }

private:
  v8::HandleScope handle_scope_;
  v8::Handle<v8::ObjectTemplate> global_;
  v8::Persistent<v8::Context> context_;
};

class Data {
public:
  static Data* instance() {
    return (Data*) ken_get_app_data();
  }

  static Data* initialize() {
    Data* data = new Data();
    ken_set_app_data(data);
    return data;
  }

  Data():
    pid_(getpid()),
    default_isolate_(Isolate::New()),
    v8_data_(new V8Data()) {
    // Set default isolate
    Isolate::RestoreDefaultIsolate(default_isolate_);
  }

  void restore() {
    // Update process id
    pid_ = getpid();

    // Restore default isolate
    Isolate::RestoreDefaultIsolate(default_isolate_);
  }

  pid_t pid() {
    return pid_;
  }

private:
  pid_t pid_;

  Isolate* default_isolate_;
  V8Data* v8_data_;
};

#endif
