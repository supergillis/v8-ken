#ifndef V8_KEN_DATA_H
#define V8_KEN_DATA_H

class V8Data {
  public:
    V8Data():
      context_scope_(NULL) {
      global_ = v8::ObjectTemplate::New();
      context_ = v8::Context::New(NULL, global_);
    }

    ~V8Data() {
      if (context_scope_ != NULL) {
        delete context_scope_;
      }
      context_.Dispose();
    }

    void enter() {
      context_->Enter();
      context_scope_ = new v8::Context::Scope(context_);
    }

    void exit() {
      if (context_scope_ != NULL) {
        delete context_scope_;
        context_scope_ = NULL;
      }
      context_->Exit();
    }

  private:
    v8::HandleScope handle_scope_;
    v8::Handle<v8::ObjectTemplate> global_;
    v8::Persistent<v8::Context> context_;

    v8::Context::Scope* context_scope_;
};

class V8KenData {
  public:
    pid_t pid;
};

#endif
