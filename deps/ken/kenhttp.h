#ifndef KENHTTP_H_INCLUDED
#define KENHTTP_H_INCLUDED

#include <stddef.h>  /* for size_t */

typedef struct {
  char name[128];
  char value[512];
} ken_http_request_header_t;

typedef struct {
  int socket;

  char method[256];
  char uri[2048];
  ken_http_request_header_t headers[128];

  char data[2048];
} ken_http_request_t;

typedef struct {
  int statusCode;
  char status[256];
  ken_http_request_header_t headers[128];

  char data[2048];
} ken_http_response_t;

extern int ken_http_parse(ken_http_request_t* request, char* buffer, size_t length);

extern void ken_http_send(ken_http_request_t* request, ken_http_response_t* response);

extern void ken_http_close(ken_http_request_t* request);

#endif
