#ifndef V8_KEN_DATA_H
#define V8_KEN_DATA_H

class V8Data {
  public:
    V8Data():
      contextScope_(NULL) {
      global_ = v8::ObjectTemplate::New();
      context_ = v8::Context::New(NULL, global_);
    }

    ~V8Data() {
      if (contextScope_ != NULL) {
        delete contextScope_;
      }
      context_.Dispose();
    }

    void enter() {
      context_->Enter();
      contextScope_ = new v8::Context::Scope(context_);
    }

    void exit() {
      if (contextScope_ != NULL) {
        delete contextScope_;
        contextScope_ = NULL;
      }
      context_->Exit();
    }

  private:
    v8::HandleScope handleScope_;
    v8::Handle<v8::ObjectTemplate> global_;
    v8::Persistent<v8::Context> context_;

    v8::Context::Scope* contextScope_;
};

class V8KenData {
  public:
    pid_t pid;
};

#endif
