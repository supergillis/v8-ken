#include "v8.h"
#include "v8-ken-http.h"

namespace v8 {
  namespace ken {
    namespace http {
      Handle<Value> CharToString(const char* str) {
        if (str == NULL)
          return Undefined();
        return String::New(str);
      }

      bool RequestToObject(ken_http_request_t* request, Handle<Object> object) {
        ken_http_header_t* header = request->headers;

        Handle<Object> headersObject = Object::New();
        while (header != NULL) {
          headersObject->Set(CharToString(header->name), CharToString(header->value));
          header = header->next;
        }

        object->Set(String::New("method"), CharToString(request->method));
        object->Set(String::New("uri"), CharToString(request->uri));
        object->Set(String::New("body"), CharToString(request->body));
        object->Set(String::New("headers"), headersObject);

        return true;
      }

      bool ObjectToResponse(Handle<Object> object, ken_http_response_t* response) {
        Handle<Value> statusCode = object->Get(String::New("statusCode"));
        Handle<Value> status = object->Get(String::New("status"));
        Handle<Value> body = object->Get(String::New("body"));
        Handle<Value> headers = object->Get(String::New("headers"));

        // Status code *must* be set
        if (!statusCode->IsUint32())
          return false;

        response->status_code = statusCode->ToUint32()->Value();

        if (status->IsString()) {
          Handle<String> statusString = status->ToString();
          response->status = (char*) malloc(sizeof(char) * (statusString->Length() + 1));
          statusString->WriteAscii(response->status);
        }

        if (body->IsString()) {
          Handle<String> bodyString = body->ToString();
          response->body = (char*) malloc(sizeof(char) * (bodyString->Length() + 1));
          bodyString->WriteAscii(response->body);
        }

        if (headers->IsObject()) {
          Handle<Object> headersObject = headers->ToObject();
          Handle<Array> headerNames = headersObject->GetPropertyNames();

          uint32_t headerSize = headerNames->Length();
          for (uint32_t index = 0; index < headerSize; index++) {
            ken_http_header_t* header = (ken_http_header_t*) malloc(sizeof(ken_http_header_t));
            Handle<String> nameString = headerNames->Get(index)->ToString();
            Handle<String> valueString = headersObject->Get(nameString)->ToString();

            header->name = (char*) malloc(sizeof(char) * (nameString->Length() + 1));
            nameString->WriteAscii(header->name);

            header->value = (char*) malloc(sizeof(char) * (valueString->Length() + 1));
            valueString->WriteAscii(header->value);

            header->next = response->headers;
            response->headers = header;
          }
        }

        /*ken_http_header_t* header = (ken_http_header_t*) malloc(sizeof(ken_http_header_t));
        header->name = { 'C', 'o', 'n', 't', 'e', 'n', '-', 'T', 'y', 'p', 'e', '\0' };
        header->value = { 't', 'e', 'x', 't',, '/', 'h', 't', 'm', 'l', '\0' };
        response->headers = header;*/

        return true;
      }
    }
  }
}
