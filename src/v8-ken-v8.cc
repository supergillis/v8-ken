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

    const char* ToCString(const String::Utf8Value& value) {
      return *value ? *value : "NULL";
    }
    
    Handle<String> JSONStringify(Handle<Value> object) {
      HandleScope handle_scope;
      TryCatch try_catch;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));
      
      Handle<Value> args[1];
      args[0] = object;

      // Try to stringify the argument
      Handle<Value> result = JSON_stringify->Call(global, 1, args);

      if (try_catch.HasCaught()) {
        HandleException(&try_catch);
        return String::Empty();
      }
      
      return Handle<String>::Cast(result);
    }
    
    Handle<Value> JSONParse(Handle<String> json) {
      HandleScope handle_scope;
      TryCatch try_catch;
      
      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::New("parse")));
      
      Handle<Value> args[1];
      args[0] = json;

      // Try to parse the JSON string
      Handle<Value> result = JSON_parse->Call(global, 1, args);

      if (try_catch.HasCaught()) {
        HandleException(&try_catch);
        return Undefined();
      }
      
      return result;
    }

    bool Eval(const char* source, int32_t length) {
      HandleScope handle_scope;
      Handle<String> source_string = String::New(source, length);
      return Eval(source_string);
    }

    bool Eval(Handle<String> source) {
      HandleScope handle_scope;
      Context::Scope context_scope(Context::GetCurrent());

      TryCatch try_catch;
      Handle<String> name = String::New("(shell)");
      Handle<Script> script = Script::Compile(source, name);

      if (script.IsEmpty()) {
        HandleException(&try_catch);
        return false;
      }

      Handle<Value> result = script->Run();

      if (try_catch.HasCaught() || result.IsEmpty()) {
        HandleException(&try_catch);
        return false;
      }

      if (!result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value
        Handle<String> stringified = JSONStringify(result);
        if (stringified->Length() > 0) {
          String::Utf8Value utf8(stringified);
          print(ToCString(utf8));
          print("\n");
        }
      }
      return true;
    }

    bool HandleException(TryCatch* try_catch) {
      // Rethrow exception
      String::Utf8Value utf8(try_catch->Exception());
      print(ToCString(utf8));
      print("\n");

      return true;
    }

    bool HandleReceive(const char* sender, const char* message, int32_t length) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("receive"));

      if (!function->IsFunction()) {
        return false;
      }
    
      Handle<Function> ken_receive = Handle<Function>::Cast(function);
      Handle<Value> ken_receive_args[2];
      ken_receive_args[0] = String::New(sender);
      ken_receive_args[1] = JSONStringify(String::New(message, length));

      ken_receive->Call(global, 2, ken_receive_args);
      return true;
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
      HandleScope handleScope;
      for (int index = 0; index < args.Length(); index++) {
        if (index != 0)
          print(" ");

        String::Utf8Value string(args[index]);
        print(ToCString(string));
        print("\n");
      }
      return Undefined();
    }

    Handle<Value> Send(const Arguments& args) {
      HandleScope handle_scope;
      Handle<String> stringified = JSONStringify(args[0]);

      String::Utf8Value kenid_string(args[0]);
      const char* kenid = ToCString(kenid_string);

      String::Utf8Value stringified_string(stringified);
      const char* message = ToCString(stringified_string);

      // Call the primitive ken_send
      seqno_t seqno = ken_send(ken_id_from_string(kenid), message, strlen(message));

      return Integer::New(seqno);
    }
    
    Handle<Value> Read(const Arguments& args) {
      Handle<String> name = Handle<String>::Cast(args[0]);
      String::Utf8Value utf8(name);
      return ReadFile(ToCString(utf8));
    }
    
    Handle<Value> Load(const Arguments& args) {
      Handle<String> name = Handle<String>::Cast(args[0]);
      
      String::Utf8Value utf8(name);
      Handle<String> source = ReadFile(ToCString(utf8));
      
      Eval(source);
      
      return Undefined();
    }
  }
}
