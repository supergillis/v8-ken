#ifndef V8_KEN_HTTP_H
#define V8_KEN_HTTP_H

extern "C" {
#include <kenhttp.h>
}

namespace v8 {
  namespace ken {
    namespace http {
      bool RequestToObject(ken_http_request_t* request, Handle<Object> object);
      bool ObjectToResponse(Handle<Object> object, ken_http_response_t* response);
    }
  }
}

#endif
