#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-persist.h"

#include "platform.h"

bool eval(v8::Handle<v8::String> source, v8::Handle<v8::Value> name);
void print(const char* str);
void print_exception(v8::TryCatch* try_catch);

static V8Data* v8Data;

const char* toCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
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

int64_t ken_handler(void* msg, int32_t len, kenid_t sender) {
	V8KenData* data = (V8KenData*) ken_get_app_data();
	if (0 == ken_id_cmp(sender, kenid_NULL) && data == NULL) {
		fprintf(stderr, "Initializing...\n");
		
		data = create_v8_ken_data();
    data->pid = getpid();
    
    // Initialize V8
    // TODO
	}
	else if (data->pid != getpid()) {
		fprintf(stderr, "Restoring...\n");
    
    // Prepare REPL
    print("> ");
		
		// Restore data and reset process id
		restore_v8_ken_data(data);
		data->pid = getpid();
		
		// Restore V8
    // TODO
    
    // Continue normal behaviour
		return ken_handler(msg, len, sender);
	}
	else if (0 == ken_id_cmp(sender, kenid_stdin)) {
    if (v8Data == NULL) {
      // TODO persist this
      v8Data = new V8Data();
      v8Data->enter();
    }
    
    // Execute javascript string
    v8::HandleScope handle_scope;
    v8::Handle<v8::String> string = v8::Handle<v8::String>(v8::String::New((const char*) msg));
    v8::Handle<v8::String> name = v8::Handle<v8::String>(v8::String::New("(shell)"));
    eval(string, name);
    
    // Prepare next REPL
    print("\n> ");
	}
	else if (0 == ken_id_cmp(sender, kenid_alarm)) {
	}

  // Save the data at each turn
  save_v8_ken_data(data);

	return -1;
}
