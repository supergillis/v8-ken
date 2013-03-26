#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-v8.h"

#include "platform.h"

/***
 * V8 ken shell main loop
 */

int64_t ken_handler(void* msg, int32_t len, kenid_t ken_sender) {
  static v8::ken::Data* data = v8::ken::Data::instance();

  if (0 == ken_id_cmp(ken_sender, kenid_NULL) && data == NULL) {
    data = v8::ken::Data::initialize();

    // Prepare REPL
    v8::ken::print("Initialized session\n");
    v8::ken::print("> ");
  }
  else if (data->pid() != getpid()) {
    // Update process id, restore V8, restore statics, ...
    data->restore();

    // Prepare REPL
    v8::ken::print("Restored session\n");
    v8::ken::print("> ");

    // Continue normal behaviour
    return ken_handler(msg, len, ken_sender);
  }
  else if (0 == ken_id_cmp(ken_sender, kenid_stdin)) {
    v8::HandleScope handle_scope;
    v8::TryCatch try_catch;

    // Execute javascript string
    v8::Handle<v8::Value> result = v8::ken::Eval((const char*) msg, len);

    if (try_catch.HasCaught()) {
      v8::ken::ReportException(&try_catch);
    }
    else if (!result->IsUndefined()) {
      v8::Handle<v8::Value> stringified = v8::ken::JSONStringify(result);

      if (stringified->IsString()) {
        v8::Handle<v8::String> string = v8::Handle<v8::String>::Cast(stringified);

        if (string->Length() > 0) {
          v8::ken::print(v8::ken::ToCString(string));
          v8::ken::print("\n");
        }
      }
    }

    // Prepare next REPL
    v8::ken::print("> ");
  }
  else if (0 == ken_id_cmp(ken_sender, kenid_alarm)) {
    // Do nothing on alarm
  }
  else {
    // Incoming message
    char sender_string[22];
    ken_id_to_string(ken_sender, sender_string, 22);

    v8::HandleScope handle_scope;
    v8::Handle<v8::String> sender = v8::String::New(sender_string);
    v8::Handle<v8::String> message = v8::String::New((const char*) msg, len);

    v8::ken::HandleReceive(sender, message);
  }

  // Save data at each turn
  data->save();

  return -1;
}
