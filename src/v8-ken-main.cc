#include <string.h>
#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"

#include "platform.h"

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

/***
 * V8 ken shell core
 */

bool eval(v8::Handle<v8::String> source, v8::Handle<v8::Value> name);
void print(const char* str);
void print_exception(v8::TryCatch* try_catch);

const char* toCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<null>";
}

void print(const char* str) {
  ken_send(kenid_stdout, str, strlen(str));
}

void print_exception(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope;
  v8::String::Utf8Value exception(try_catch->Exception());

  // Convert to cstring and print
  const char* exception_string = toCString(exception);
  print(exception_string);
}

bool eval(v8::Handle<v8::String> source, v8::Handle<v8::Value> name) {
  v8::HandleScope handle_scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
  if (script.IsEmpty()) {
    print_exception(&try_catch);
    return false;
  }
  else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      assert(try_catch.HasCaught());

      // Print errors that happened during execution.
      print_exception(&try_catch);
      return false;
    }
    else {
      assert(!try_catch.HasCaught());
      if (!result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::Utf8Value string(result);

        // Convert to cstring and print
        const char* cstr = toCString(string);
        print(cstr);
      }
      return true;
    }
  }
}

}
}

/***
 * V8 ken shell main loop
 */

int64_t ken_handler(void* msg, int32_t len, kenid_t sender) {
  static v8::ken::Data* data = v8::ken::Data::instance();

  if (0 == ken_id_cmp(sender, kenid_NULL) && data == NULL) {
    fprintf(stderr, "Initializing...\n");

    data = v8::ken::Data::initialize();

    // Verify heap
    v8::internal::Isolate::Current()->heap()->Verify();

    // Prepare REPL
    v8::ken::print("> ");
  }
  else if (data->pid() != getpid()) {
    fprintf(stderr, "Restoring from %d...\n", data->pid());

    // Update process id, restore V8, restore statics, ...
    data->restore();

    // Verify heap
    v8::internal::Isolate::Current()->heap()->Verify();

    // Prepare REPL
    v8::ken::print("> ");

    // Continue normal behaviour
    return ken_handler(msg, len, sender);
  }
  else if (0 == ken_id_cmp(sender, kenid_stdin)) {
    v8::HandleScope handle_scope;
    v8::Handle<v8::String> string = v8::String::New((const char*) msg, len);
    v8::Handle<v8::String> name = v8::String::New("(shell)");

    // Execute javascript string
    v8::ken::eval(string, name);

    // Prepare next REPL
    v8::ken::print("\n> ");
  }
  else if (0 == ken_id_cmp(sender, kenid_alarm)) {
  }

  // Save data at each turn
  data->save();

  return -1;
}
