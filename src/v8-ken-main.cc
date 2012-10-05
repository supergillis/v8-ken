#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-persist.h"

bool eval(v8::Handle<v8::String> source, v8::Handle<v8::Value> name);
void print(const char* str);
void printException(v8::TryCatch* tryCatch);

static V8Data* v8Data;

const char* toCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void print(const char* str) {
  ken_send(kenid_stdout, str, strlen(str));
}

void printException(v8::TryCatch* tryCatch) {
  v8::HandleScope handleScope;
  v8::String::Utf8Value exception(tryCatch->Exception());

  // Convert to cstring and print
  const char* exception_string = toCString(exception);
  print(exception_string);
}

bool eval(v8::Handle<v8::String> source, v8::Handle<v8::Value> name) {
  v8::HandleScope handleScope;
  v8::TryCatch tryCatch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
  if (script.IsEmpty()) {
    printException(&tryCatch);
    return false;
  }
  else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      assert(tryCatch.HasCaught());

      // Print errors that happened during execution.
      printException(&tryCatch);
      return false;
    }
    else {
      assert(!tryCatch.HasCaught());
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
	if (0 == ken_id_cmp(sender, kenid_NULL)) {
		print("Initializing...\n");

		NTF(data == NULL);
		
		data = create_v8_ken_data();
    data->pid = getpid();
    
    // Initialize V8
    // TODO
	}
	else if (data->pid != getpid()) {
		print("Restoring...\n");
    print("> ");
		
		// Restore data and reset process id
		restore_v8_ken_data(data);
		data->pid = getpid();
    
    // TODO persist this
    v8Data = new V8Data();
    v8Data->enter();
		
		// Restore V8
    // TODO
    
    // Continue normal behaviour
		return ken_handler(msg, len, sender);
	}
	else if (0 == ken_id_cmp(sender, kenid_stdin)) {
    const char* str = (const char*) msg;
    
    // Execute javascript string
    v8::HandleScope handleScope;
    v8::Handle<v8::String> name = v8::Handle<v8::String>(v8::String::New("(shell)"));
    eval(v8::String::New(str), name);
    
    // Prepare next REPL
    print("\n> ");
	}
	else if (0 == ken_id_cmp(sender, kenid_alarm)) {
	}

  // Save the data at each turn
  save_v8_ken_data(data);

	return -1;
}
