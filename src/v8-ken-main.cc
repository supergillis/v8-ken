#include <string.h>
#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-v8.h"

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

static v8::Handle<v8::Value> ken_print(const v8::Arguments& args) {
  for (int index = 0; index < args.Length(); index++) {
    v8::HandleScope handle_scope;
    if (index != 0)
      print(" ");

    v8::String::Utf8Value str(args[index]);
    const char* cstr = toCString(str);
    print(cstr);
  }
  return v8::Undefined();
}

static v8::Handle<v8::Value> ken_send(const v8::Arguments& args) {
  v8::String::Utf8Value kenid_string(args[0]);
  v8::String::Utf8Value msg_string(args[1]);

  const char* kenid = toCString(kenid_string);
  const char* msg = toCString(msg_string);

  seqno_t seqno = ken_send(ken_id_from_string(kenid), msg, strlen(msg));

  return v8::Integer::New(seqno);
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

    // Install custom functions
    v8::HandleScope handle_scope;
    v8::Handle<v8::Object> global = data->v8()->context()->Global();
    global->Set(v8::String::New("print"), v8::FunctionTemplate::New(v8::ken::ken_print)->GetFunction());
    global->Set(v8::String::New("send"), v8::FunctionTemplate::New(v8::ken::ken_send)->GetFunction());

    // Prepare REPL
    v8::ken::print("> ");
  }
  else if (data->pid() != getpid()) {
    fprintf(stderr, "Restoring from %d...\n", data->pid());

    // Update process id, restore V8, restore statics, ...
    data->restore();

    // Prepare REPL
    v8::ken::print("> ");

    // Continue normal behaviour
    return ken_handler(msg, len, sender);
  }
  else if (0 == ken_id_cmp(sender, kenid_stdin)) {
    v8::HandleScope handle_scope;
    v8::Context::Scope context_scope(data->v8()->context());

    v8::Handle<v8::String> string = v8::String::New((const char*) msg, len);
    v8::Handle<v8::String> name = v8::String::New("(shell)");

    // Execute javascript string
    v8::ken::eval(string, name);

    // Prepare next REPL
    v8::ken::print("\n> ");
  }
  else if (0 == ken_id_cmp(sender, kenid_alarm)) {
    // Alarm
  }
  else {
    // Incoming message
    v8::HandleScope handle_scope;
    v8::Context::Scope context_scope(data->v8()->context());

    v8::Handle<v8::Object> global = data->v8()->context()->Global();
    v8::Handle<v8::Value> value = global->Get(v8::String::New("receive"));

    if (value->IsFunction()) {
      v8::Handle<v8::Function> ken_receive = v8::Handle<v8::Function>::Cast(value);
      v8::Handle<v8::Value> args[1];
      args[0] = v8::String::New((const char*) msg, len);;

      ken_receive->Call(global, 1, args);
    }
  }

  // Save data at each turn
  data->save();

  return -1;
}
