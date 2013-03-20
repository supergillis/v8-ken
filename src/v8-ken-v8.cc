#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-v8.h"

namespace v8 {
  namespace ken {
    Handle<Value> Print(const Arguments& args);
    Handle<Value> Send(const Arguments& args);

    void Initialize(Handle<Object> object) {
      object->Set(String::New("print"), FunctionTemplate::New(Print)->GetFunction());
      object->Set(String::New("send"), FunctionTemplate::New(Send)->GetFunction());
    }

    const char* ToCString(const String::Utf8Value& value) {
      return *value ? *value : "NULL";
    }

    bool Eval(const char* source, int32_t length) {
      HandleScope handleScope;
      Handle<String> sourceString = String::New(source, length);
      return Eval(sourceString);
    }

    bool Eval(Handle<String> source) {
      HandleScope handleScope;
      Context::Scope contextScope(Context::GetCurrent());

      TryCatch tryCatch;
      Handle<String> name = String::New("(shell)");
      Handle<Script> script = Script::Compile(source, name);

      if (script.IsEmpty()) {
        HandleException(&tryCatch);
        return false;
      }

      Handle<Value> result = script->Run();

      if (tryCatch.HasCaught() || result.IsEmpty()) {
        HandleException(&tryCatch);
        return false;
      }

      if (!result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value
        String::Utf8Value string(result);
        if (string.length() > 0) {
          print(ToCString(string));
          print("\n");
        }
      }
      return true;
    }

    bool HandleException(TryCatch* tryCatch) {
      HandleScope handleScope;

      // Convert exception to string and print
      String::Utf8Value message(tryCatch->Exception());
      fprintf(stderr, "Unhandled exception: %s\n", ToCString(message));

      return true;
    }

    bool HandleReceive(const char* sender, const char* message, int32_t length) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> value = global->Get(String::New("receive"));

      if (!value->IsFunction()) {
        return false;
      }

      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::New("parse")));
      Handle<Value> JSON_parse_args[1];
      JSON_parse_args[0] = String::New(message, length);

      // Try to parse the JSON strinĝ
      TryCatch tryCatch;
      Handle<Value> result = JSON_parse->Call(global, 1, JSON_parse_args);

      if (tryCatch.HasCaught()) {
        HandleException(&tryCatch);
        return false;
      }

      Handle<Function> ken_receive = Handle<Function>::Cast(value);
      Handle<Value> ken_receive_args[2];
      ken_receive_args[0] = String::New(sender);
      ken_receive_args[1] = result;

      ken_receive->Call(global, 2, ken_receive_args);
      return true;
    }

    Handle<Value> Print(const Arguments& args) {
      HandleScope handleScope;
      for (int index = 0; index < args.Length(); index++) {
        if (index != 0)
          print(" ");

        String::Utf8Value string(args[index]);
        print(ToCString(string));
      }
      return Undefined();
    }

    Handle<Value> Send(const Arguments& args) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));
      Handle<Value> JSON_stringify_args[1];
      JSON_stringify_args[0] = args[1];

      // Try to stringify the argument
      TryCatch tryCatch;
      Handle<Value> result = JSON_stringify->Call(global, 1, JSON_stringify_args);

      if (tryCatch.HasCaught()) {
        return ThrowException(tryCatch.Exception());
      }

      String::Utf8Value kenid_string(args[0]);
      const char* kenid = ToCString(kenid_string);

      String::Utf8Value stringified_string(result);
      const char* stringified = ToCString(stringified_string);

      // Call the primitive ken_send
      seqno_t seqno = ken_send(ken_id_from_string(kenid), stringified, strlen(stringified));

      return Integer::New(seqno);
    }
  }
}