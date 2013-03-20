#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-v8.h"

#include "platform.h"

/***
 * V8 ken shell main loop
 */

int64_t ken_handler(void* msg, int32_t len, kenid_t sender) {
  static v8::ken::Data* data = v8::ken::Data::instance();

  if (0 == ken_id_cmp(sender, kenid_NULL) && data == NULL) {
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
    return ken_handler(msg, len, sender);
  }
  else if (0 == ken_id_cmp(sender, kenid_stdin)) {
    // Execute javascript string
    v8::ken::Eval((const char*) msg, len);

    // Prepare next REPL
    v8::ken::print("> ");
  }
  else if (0 == ken_id_cmp(sender, kenid_alarm)) {
    // Do nothing on alarm
  }
  else {
    // Incoming message
    v8::ken::HandleReceive((const char*) msg, len);
  }

  // Save data at each turn
  data->save();

  return -1;
}
