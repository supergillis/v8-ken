#ifndef V8_KEN_DATA_H
#define V8_KEN_DATA_H

#define MAX_PERSISTS 50

namespace v8 {

namespace internal {
class Isolate;
}

namespace ken {

class V8;

class Data {
  struct Persist {
    void* system;
    void* ken;
    size_t size;
  };

public:
  Data();

  void save();
  void restore();
  void persist(void* system, size_t size);

  template<class T>
  void persist(T* system) {
    persist(system, sizeof(T));
  }

  pid_t pid() {
    return pid_;
  }

  V8* v8() {
    return v8_;
  }

  static Data* instance();

  static Data* initialize();

private:
  pid_t pid_;

  // Allocate on the heap, for some kind of reason ken gives an error when allocated on stack
  Persist* persists_;
  uint32_t persist_count_;

  v8::internal::Isolate* isolate_;
  V8* v8_;

};

}
}

#endif
