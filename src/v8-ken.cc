#include <string.h>

#include "v8-ken.h"

/***
 * Override new and delete operators
 */

static void* ken_new(size_t s) {
  assert(0 != ken_heap_ready);
  void* ptr = ken_malloc(s);
  memset(ptr, '\0', s);
  assert(NULL != ptr);
  return ptr;
}

void* operator new(size_t size) {
  return ken_new(size);
}
void* operator new[](size_t size) {
  return ken_new(size);
}

static void ken_delete(void* ptr) {
  assert(0 != ken_heap_ready && NULL != ptr);
  ken_free(ptr);
}

void operator delete(void* ptr) {
  ken_delete(ptr);
}
void operator delete[](void* ptr) {
  ken_delete(ptr);
}

namespace v8 {
  namespace ken {
    void print(const char* string) {
      print(string, strlen(string));
    }

    void print(const char* string, int32_t length) {
      if (length > 0) {
        ken_send(kenid_stdout, string, length);
      }
    }
  }
}
