#ifndef V8_KEN_DATA_H
#define V8_KEN_DATA_H

#include "v8-ken.h"
#include "v8-ken-data-v8.h"
#include "v8-ken-data-statics.h"

#include "isolate.h"

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

  Data() {
    pid_ = getpid();
    isolate_ = v8::internal::Isolate::New();

    // Set default isolate
    v8::internal::Isolate::RestoreDefaultIsolate(isolate_);

    // Initialize V8 stuff after the isolate
    statics_ = new Statics();
    v8_ = new V8();
  }

  void save() {
    statics_->save();
  }

  void restore() {
    pid_ = getpid();
    statics_->restore();

    // Restore default isolate
    v8::internal::Isolate::RestoreDefaultIsolate(isolate_);
  }

  pid_t pid() {
    return pid_;
  }

private:
  pid_t pid_;
  v8::internal::Isolate* isolate_;
  Statics* statics_;
  V8* v8_;
};

#endif
