#include "v8.h"
#include "v8-ken-http.h"

namespace v8 {
  namespace ken {
    namespace http {
      Handle<Value> CharToString(const char* str) {
        if (str == NULL) {
          return Undefined();
        }
        return String::New(str);
      }

      bool RequestToObject(ken_http_request_t* request, Handle<Object> object) {
        object->Set(String::New("method"), CharToString(request->method));
        object->Set(String::New("uri"), CharToString(request->uri));
        object->Set(String::New("body"), CharToString(request->body));

        return true;
      }

      bool ObjectToResponse(Handle<Object> object, ken_http_response_t* response) {
        Handle<Value> statusCode = object->Get(String::New("statusCode"));
        Handle<Value> status = object->Get(String::New("status"));
        Handle<Value> body = object->Get(String::New("body"));

        // Status code *must* be set
        if (!statusCode->IsUint32())
          return false;

        response->status_code = statusCode->ToUint32()->Value();

        if (status->IsString()) {
          Handle<String> statusString = status->ToString();
          response->status = new char[statusString->Length() + 1];
          statusString->WriteAscii(response->status);
        }

        if (body->IsString()) {
          Handle<String> bodyString = body->ToString();
          response->body = new char[bodyString->Length() + 1];
          bodyString->WriteAscii(response->body);
        }

        return true;
      }
    }
  }
}
