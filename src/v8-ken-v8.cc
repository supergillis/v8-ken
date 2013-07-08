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

    const char* ToCString(const String::Utf8Value& value) {
      return *value ? *value : "NULL";
    }

    Handle<Value> JSONStringify(Handle<Value> object) {
      HandleScope handle_scope;
      Handle<Value> result;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));

      Handle<Value> args[1];
      args[0] = object;

      result = JSON_stringify->Call(global, 1, args);
      return handle_scope.Close(result);
    }

    Handle<Value> JSONParse(Handle<String> json) {
      HandleScope handle_scope;
      Handle<Value> result;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
      Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::New("parse")));

      Handle<Value> args[1];
      args[0] = json;

      result = JSON_parse->Call(global, 1, args);
      return handle_scope.Close(result);
    }

    Handle<Value> Eval(const char* file, const char* source, int32_t length) {
      HandleScope handle_scope;
      Handle<Value> result;

      Handle<String> file_string = String::New(file);
      Handle<String> source_string = String::New(source, length);
      result = Eval(file_string, source_string);
      return handle_scope.Close(result);
    }

    Handle<Value> Eval(Handle<String> file, Handle<String> source) {
      HandleScope handle_scope;
      Handle<Value> result;

      Handle<Script> script = Script::Compile(source, file);
      if (script.IsEmpty()) {
        result = Undefined();
      } else {
        result = script->Run();
      }

      return handle_scope.Close(result);
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
      Handle<Value> result;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("receive"));

      if (!function->IsFunction()) {
        result = Undefined();
      } else {
        Handle<Function> ken_receive = Handle<Function>::Cast(function);
        Handle<Value> ken_receive_args[2];
        ken_receive_args[0] = sender;
        ken_receive_args[1] = JSONStringify(message);

        result = ken_receive->Call(global, 2, ken_receive_args);
      }

      return handle_scope.Close(result);
    }

    /**
     *
     */
    Handle<Value> HandleHttpRequest(Handle<Object> request, Handle<Object> response) {
      HandleScope handle_scope;
      Handle<Value> result;

      Handle<Object> global = Context::GetCurrent()->Global();
      Handle<Value> function = global->Get(String::New("handleRequest"));

      if (!function->IsFunction()) {
        result = Undefined();
      } else {
        Handle<Function> handle_request = Handle<Function>::Cast(function);
        Handle<Value> handle_request_args[2];
        handle_request_args[0] = request;
        handle_request_args[1] = response;

        result = handle_request->Call(global, 2, handle_request_args);
      }

      return handle_scope.Close(result);
    }

    /**
     * We do not enter a handle scope here. This function must be called using
     * code that is already in a handle scope.
     *
     * @param name
     */
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
      Handle<Value> result;

      int length = args.Length();
      for (int index = 0; index < length; index++) {
        String::Utf8Value string(args[index]);
        print(ToCString(string));
      }

      print("\n");

      result = Undefined();
      return handle_scope.Close(result);
    }

    Handle<Value> Send(const Arguments& args) {
      HandleScope handle_scope;
      Handle<Value> result;
      Handle<Value> kenid_val;
      Handle<Value> message_val;

      kenid_val = args[0];
      if (!kenid_val->IsString()) {
        result = ThrowException(String::New("Expected a string as first argument!"));
      } else {
        message_val = JSONStringify(args[1]);
        if (!message_val->IsString()) {
          result = ThrowException(String::New("Expected an object that can be stringified as second argument!"));
        } else {
          String::Utf8Value kenid(kenid_val->ToString());
          String::Utf8Value message(message_val->ToString());

          // Call the primitive ken_send
          seqno_t seqno = ken_send(ken_id_from_string(*kenid), *message, strlen(*message));

          result = Integer::New(seqno);
        }
      }

      return handle_scope.Close(result);
    }

    Handle<Value> Read(const Arguments& args) {
      HandleScope handle_scope;
      Handle<Value> result;
      Handle<String> file;
      Handle<String> source;

      if (args.Length() != 1 || !args[0]->IsString()) {
        result = ThrowException(String::New("Read expects a string as first parameter!"));
      } else {
        file = args[0]->ToString();
        source = ReadFile(*String::Utf8Value(file));
        if (source.IsEmpty()) {
          result = ThrowException(String::Concat(String::New("Error reading file: "), file));
        } else {
          result = source;
        }
      }

      return handle_scope.Close(result);
    }

    Handle<Value> Load(const Arguments& args) {
      HandleScope handle_scope;
      Handle<Value> result;

      if (args.Length() != 1 || !args[0]->IsString()) {
        result = ThrowException(String::New("Load expects a string as first parameter!"));
      } else {
        result = Read(args);

        // Stringify if it's a string
        if (result->IsString()) {
          result = Eval(args[0]->ToString(), result->ToString());
        }
      }

      return handle_scope.Close(result);
    }

    Handle<Value> Eval(const Arguments& args) {
      HandleScope handle_scope;
      Handle<Value> result;

      if (args.Length() != 1 || !args[0]->IsString()) {
        result = ThrowException(String::New("Eval expects a string as first parameter!"));
      } else {
        result = Eval(String::New("eval"), args[0]->ToString());
      }

      return handle_scope.Close(result);
    }

    Handle<Value> HrTime(const Arguments& args) {
      HandleScope handle_scope;
      Handle<Value> result;
      Handle<Array> array;
      Handle<Array> start;

      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);

      if (args.Length() == 0) {
        array = Array::New(2);
        array->Set(0, Integer::New(ts.tv_sec));
        array->Set(1, Integer::New(ts.tv_nsec));
        result = array;
      } else if (args.Length() != 1 || !args[0]->IsArray()) {
        result = ThrowException(String::New("HrTime expects no parameters, or time as first argument!"));
      } else {
        start = Handle<Array>::Cast(args[0]);
        if (!start->Get(0)->IsNumber() || !start->Get(1)->IsNumber()) {
          result = ThrowException(String::New("HrTime expects no parameters, or time as first argument!"));
        } else {
          int64_t elapsed_sec = ts.tv_sec - start->Get(0)->ToInteger()->IntegerValue();
          int64_t elapsed_nsec = ts.tv_nsec - start->Get(1)->ToInteger()->IntegerValue();

          if (elapsed_sec > 0) {
            if (elapsed_nsec < 0) {
              elapsed_nsec += 1e9;
              elapsed_sec -= 1;
            }
          } else if (elapsed_sec < 0) {
            if (elapsed_nsec > 0) {
              elapsed_nsec -= 1e9;
              elapsed_sec += 1;
            }
          }

          array = Array::New(2);
          array->Set(0, Integer::New(elapsed_sec));
          array->Set(1, Integer::New(elapsed_nsec));
          result = array;
        }
      }

      return handle_scope.Close(result);
    }
  }
}
