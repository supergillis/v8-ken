#ifndef KENHTTP_H_INCLUDED
#define KENHTTP_H_INCLUDED

#include <stddef.h>  /* for size_t */

typedef struct {
  char name[128];
  char value[512];
} http_request_header_t;

typedef struct {
  int socket;
  char method[256];
  char uri[2048];
  http_request_header_t headers[128];
} http_request_t;

extern int ken_http_parse(http_request_t* request, char* buffer, size_t length);

extern void ken_http_close(http_request_t* request);

#endif
