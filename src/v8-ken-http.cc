#include "v8.h"
#include "v8-ken-http.h"

namespace v8 {
  namespace ken {
    namespace http {
      bool RequestToObject(ken_http_request_t* request, Handle<Object> object) {
        object->Set(String::New("method"), String::New(request->method));
        object->Set(String::New("uri"), String::New(request->uri));
        object->Set(String::New("data"), String::New(request->data));

        return true;
      }

      bool ObjectToResponse(Handle<Object> object, ken_http_response_t* response) {
        Handle<Value> statusCode = object->Get(String::New("statusCode"));
        Handle<Value> status = object->Get(String::New("status"));
        Handle<Value> data = object->Get(String::New("data"));

        // Status code *must* be set
        if (!statusCode->IsUint32())
          return false;

        response->statusCode = statusCode->ToUint32()->Value();

        if (status->IsString())
          status->ToString()->WriteAscii(response->status);

        if (data->IsString())
          data->ToString()->WriteAscii(response->data);

        return true;
      }
    }
  }
}
