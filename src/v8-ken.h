extern "C" {
#include <ken.h>
#include <kenapp.h>
#include <kencom.h>
}

namespace v8 {
  namespace ken {
    extern void print(const char* string);
    extern void print(const char* string, int32_t length);
  }
}
