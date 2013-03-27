#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-v8.h"

namespace v8 {
  namespace ken {
    Handle<String> ReadFile(const char* name);

    Handle<Value> Print(const Arguments& args);
    Handle<Value> Send(const Arguments& args);
    Handle<Value> Read(const Arguments& args);
    Handle<Value> Load(const Arguments& args);

    void Initialize(Handle<Object> object) {
      object->Set(String::New("print"), FunctionTemplate::New(Print)->GetFunction());
      object->Set(String::New("send"), FunctionTemplate::New(Send)->GetFunction());
      object->Set(String::New("read"), FunctionTemplate::New(Read)->GetFunction());
      object->Set(String::New("load"), FunctionTemplate::New(Load)->GetFunction());
    }

    const char* ToCString(Handle<String> value) {
      return ToCString(String::Utf8Value(value));
    }

    const char* ToCString(const String::Utf8Value& value) {
      return *value ? *value : "NULL";
    }

    Handle<Value> JSONStringify(Handle<Value> object) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));

      Handle<Value> args[1];
      args[0] = object;

      return JSON_stringify->Call(global, 1, args);
    }

    Handle<Value> JSONParse(Handle<String> json) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::New("parse")));

      Handle<Value> args[1];
      args[0] = json;

      return JSON_parse->Call(global, 1, args);
    }

    Handle<Value> Eval(const char* source, int32_t length) {
      HandleScope handle_scope;
      Handle<String> source_string = String::New(source, length);
      return Eval(source_string);
    }

    Handle<Value> Eval(Handle<String> source) {
      HandleScope handle_scope;

      Handle<String> name = String::New("(shell)");
      Handle<Script> script = Script::Compile(source, name);

      if (script.IsEmpty()) {
        return Undefined();
      }

      Handle<Value> result = script->Run();
      if (result.IsEmpty()) {
        return Undefined();
      }

      return result;
    }

    void ReportException(TryCatch* try_catch) {
      String::Utf8Value utf8(try_catch->Exception());
      print(ToCString(utf8));
      print("\n");
    }

    Handle<Value> HandleReceive(Handle<String> sender, Handle<String> message) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("receive"));

      if (!function->IsFunction()) {
        return Undefined();
      }

      Handle<Function> ken_receive = Handle<Function>::Cast(function);
      Handle<Value> ken_receive_args[2];
      ken_receive_args[0] = sender;
      ken_receive_args[1] = JSONStringify(message);

      return ken_receive->Call(global, 2, ken_receive_args);
    }

    Handle<Value> HandleHttpRequest(Handle<String> method, Handle<String> uri) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("handleRequest"));

      if (!function->IsFunction()) {
        return Undefined();
      }

      Handle<Function> handle_request = Handle<Function>::Cast(function);
      Handle<Value> handle_request_args[2];
      handle_request_args[0] = method;
      handle_request_args[1] = uri;

      return handle_request->Call(global, 2, handle_request_args);
    }

    Handle<String> ReadFile(const char* name) {
      FILE* file = fopen(name, "rb");
      if (file == NULL) {
        return Handle<String>();
      }

      fseek(file, 0, SEEK_END);
      int size = ftell(file);
      rewind(file);

      char* chars = new char[size + 1];
      chars[size] = '\0';
      for (int i = 0; i < size;) {
        int read = fread(&chars[i], 1, size - i, file);
        i += read;
      }
      fclose(file);

      Handle<String> result = String::New(chars, size);
      delete[] chars;
      return result;
    }

    Handle<Value> Print(const Arguments& args) {
      HandleScope handle_scope;

      for (int index = 0; index < args.Length(); index++) {
        if (index != 0) {
          print(" ");
        }

        String::Utf8Value string(args[index]);
        print(ToCString(string));
        print("\n");
      }

      return Undefined();
    }

    Handle<Value> Send(const Arguments& args) {
      HandleScope handle_scope;

      Handle<Value> kenid_val = args[0];
      if (!kenid_val->IsString()) {
        return ThrowException(String::New("Expected a string as first argument!"));
      }

      Handle<Value> message_val = JSONStringify(args[1]);
      if (!message_val->IsString()) {
        return ThrowException(String::New("Expected an object that can be stringified as second argument!"));
      }

      Handle<String> kenid_str = Handle<String>::Cast(kenid_val);
      Handle<String> message_str = Handle<String>::Cast(message_val);

      const char* kenid = ToCString(kenid_str);
      const char* message = ToCString(message_str);

      // Call the primitive ken_send
      seqno_t seqno = ken_send(ken_id_from_string(kenid), message, strlen(message));

      return Integer::New(seqno);
    }

    Handle<Value> Read(const Arguments& args) {
      Handle<String> name = Handle<String>::Cast(args[0]);
      return ReadFile(ToCString(name));
    }

    Handle<Value> Load(const Arguments& args) {
      Handle<String> name = Handle<String>::Cast(args[0]);
      Handle<String> source = ReadFile(ToCString(name));

      return Eval(source);
    }
  }
}
