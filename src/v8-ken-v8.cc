#include "v8.h"
#include "v8-ken.h"
#include "v8-ken-v8.h"

#include <time.h>

namespace v8 {
  namespace ken {
    Handle<String> ReadFile(const char* name);

    Handle<Value> Print(const Arguments& args);
    Handle<Value> Send(const Arguments& args);
    Handle<Value> Read(const Arguments& args);
    Handle<Value> Load(const Arguments& args);
    Handle<Value> Eval(const Arguments& args);
    Handle<Value> HrTime(const Arguments& args);

    void Initialize(Handle<Object> object) {
      object->Set(String::New("print"), FunctionTemplate::New(Print)->GetFunction());
      object->Set(String::New("send"), FunctionTemplate::New(Send)->GetFunction());

      object->Set(String::New("read"), FunctionTemplate::New(Read)->GetFunction());
      object->Set(String::New("load"), FunctionTemplate::New(Load)->GetFunction());
      object->Set(String::New("eval"), FunctionTemplate::New(Eval)->GetFunction());

      object->Set(String::New("hrtime"), FunctionTemplate::New(HrTime)->GetFunction());
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

    Handle<Value> Eval(const char* file, const char* source, int32_t length) {
      HandleScope handle_scope;
      Handle<String> file_string = String::New(file);
      Handle<String> source_string = String::New(source, length);
      return Eval(file_string, source_string);
    }

    Handle<Value> Eval(Handle<String> file, Handle<String> source) {
      HandleScope handle_scope;
      Handle<Script> script = Script::Compile(source, file);

      if (script.IsEmpty()) {
        return Undefined();
      }

      Handle<Value> result = script->Run();
      if (result.IsEmpty()) {
        return Undefined();
      }

      return result;
    }

    void ReportException(v8::TryCatch* try_catch) {
      char buffer[2048];

      HandleScope handle_scope;
      String::Utf8Value exception(try_catch->Exception());
      const char* exception_string = ToCString(exception);
      Handle<v8::Message> message = try_catch->Message();
      if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        snprintf(buffer, 2048, "%s\n", exception_string);
        print(buffer);
      } else {
        // Print (filename):(line number): (message).
        String::Utf8Value filename(message->GetScriptResourceName());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber();
        snprintf(buffer, 2048, "%s:%i: %s\n", filename_string, linenum, exception_string);
        print(buffer);
        // Print line of source code.
        String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceline_string = ToCString(sourceline);
        snprintf(buffer, 2048, "%s\n", sourceline_string);
        print(buffer);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn();
        for (int i = 0; i < start; i++) {
          print(" ");
        }
        int end = message->GetEndColumn();
        for (int i = start; i < end; i++) {
          print("^");
        }
        print("\n");
        String::Utf8Value stack_trace(try_catch->StackTrace());
        if (stack_trace.length() > 0) {
          const char* stack_trace_string = ToCString(stack_trace);
          snprintf(buffer, 2048, "%s\n", stack_trace_string);
          print(buffer);
        }
      }
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

    Handle<Value> HandleHttpRequest(Handle<Object> request, Handle<Object> response) {
      HandleScope handle_scope;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("handleRequest"));

      if (!function->IsFunction()) {
        return Undefined();
      }

      Handle<Function> handle_request = Handle<Function>::Cast(function);
      Handle<Value> handle_request_args[2];
      handle_request_args[0] = request;
      handle_request_args[1] = response;

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
        String::Utf8Value string(args[index]);
        print(ToCString(string));
      }

      print("\n");

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
      if (args.Length() != 1 || !args[0]->IsString()) {
        return ThrowException(String::New("Read expects a string as first parameter!"));
      }

      Handle<String> file = args[0]->ToString();
      Handle<String> source = ReadFile(*String::Utf8Value(file));
      if (source.IsEmpty()) {
        return ThrowException(String::Concat(String::New("Error reading file: "), file));
      }

      return source;
    }

    Handle<Value> Load(const Arguments& args) {
      if (args.Length() != 1 || !args[0]->IsString()) {
        return ThrowException(String::New("Load expects a string as first parameter!"));
      }

      Handle<Value> value = Read(args);
      if (!value->IsString()) {
        return value;
      }

      return Eval(args[0]->ToString(), value->ToString());
    }

    Handle<Value> Eval(const Arguments& args) {
      if (args.Length() != 1 || !args[0]->IsString()) {
        return ThrowException(String::New("Eval expects a string as first parameter!"));
      }

      return Eval(String::New("eval"), args[0]->ToString());
    }

    Handle<Value> HrTime(const Arguments& args) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);

      if (args.Length() == 0) {
        Handle<Array> result = Array::New(2);
        result->Set(0, Integer::New(ts.tv_sec));
        result->Set(1, Integer::New(ts.tv_nsec));
        return result;
      }

      if (args.Length() != 1 || !args[0]->IsArray()) {
        return ThrowException(String::New("HrTime expects no parameters, or time as first argument!"));
      }

      Handle<Array> start = Handle<Array>::Cast(args[0]);
      if (!start->Get(0)->IsNumber() || !start->Get(1)->IsNumber()) {
        return ThrowException(String::New("HrTime expects no parameters, or time as first argument!"));
      }

      int64_t elapsed_sec = ts.tv_sec - start->Get(0)->ToInteger()->IntegerValue();
      int64_t elapsed_nsec = ts.tv_nsec - start->Get(1)->ToInteger()->IntegerValue();

      if (elapsed_sec > 0) {
        if (elapsed_nsec < 0) {
          elapsed_nsec += 1e9;
          elapsed_sec -= 1;
        }
      }
      else if (elapsed_sec < 0) {
        if (elapsed_nsec > 0) {
          elapsed_nsec -= 1e9;
          elapsed_sec += 1;
        }
      }

      Handle<Array> result = Array::New(2);
      result->Set(0, Integer::New(elapsed_sec));
      result->Set(1, Integer::New(elapsed_nsec));
      return result;
    }
  }
}
