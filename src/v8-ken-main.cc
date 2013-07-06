#include <http_parser.h>

#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-http.h"
#include "v8-ken-data.h"
#include "v8-ken-v8.h"

#define NOT_FOUND_STATUS ((char*) "Not Found")
#define NOT_FOUND_BODY ((char*) "This page was not found.")

/***
 * V8 ken shell main loop
 */

int64_t ken_handler(void* msg, int32_t len, kenid_t ken_sender) {
  static v8::ken::Data* data = v8::ken::Data::instance();

  if (0 == ken_id_cmp(ken_sender, kenid_NULL)) {
    if (data == NULL) {
      data = v8::ken::Data::initialize();

      // Prepare REPL
      v8::ken::print("Initialized session\n");
      v8::ken::print("> ");
    }
    else {
      // Update process id, restore V8, restore statics, ...
      data->restore();

      // Prepare REPL
      v8::ken::print("Restored session\n");
      v8::ken::print("> ");
    }
  }
  else if (0 == ken_id_cmp(ken_sender, kenid_stdin)) {
    v8::HandleScope handle_scope;
    v8::TryCatch try_catch;

    // Execute javascript string
    v8::Handle<v8::Value> result = v8::ken::Eval("shell", (const char*) msg, len);

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
  else if (0 == ken_id_cmp(ken_sender, kenid_http)) {
    static ken_http_response_t response;

    v8::HandleScope handle_scope;
    v8::Handle<v8::Object> request_object = v8::Object::New();
    v8::Handle<v8::Object> response_object = v8::Object::New();

    ken_http_request_t* request = (ken_http_request_t*) msg;

    bool success = false;
    if (v8::ken::http::RequestToObject(request, request_object)) {
      v8::ken::HandleHttpRequest(request_object, response_object);

      if (v8::ken::http::ObjectToResponse(response_object, &response)) {
        success = true;
      }
    }

    if (!success) {
      response.status_code = 404;
      response.status = NOT_FOUND_STATUS;
      response.body = NOT_FOUND_BODY;
      ken_http_send(request, &response);
    }
    else {
      ken_http_send(request, &response);
      ken_http_response_free(&response);
    }

    ken_http_close(request);
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
