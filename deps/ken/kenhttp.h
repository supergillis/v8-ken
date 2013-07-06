#ifndef KENHTTP_H_INCLUDED
#define KENHTTP_H_INCLUDED

#include <stddef.h>  /* for size_t */

typedef struct ken_http_header {
  char* name;
  char* value;

  struct ken_http_header* next;
} ken_http_header_t;

typedef struct {
  int socket;

  char* method;
  char* uri;
  char* body;

  ken_http_header_t* headers;
} ken_http_request_t;

typedef struct {
  unsigned short status_code;
  char* status;
  char* body;

  ken_http_header_t* headers;
} ken_http_response_t;

extern int ken_http_parse(ken_http_request_t*, const char*, size_t);

extern void ken_http_send(ken_http_request_t*, ken_http_response_t*);

extern void ken_http_close(ken_http_request_t*);

extern void ken_http_request_init(ken_http_request_t*);
extern void ken_http_request_free(ken_http_request_t*);

extern void ken_http_response_free(ken_http_response_t*);

#endif
