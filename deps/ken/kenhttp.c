#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <http_parser.h>

#include "kencom.h"
#include "kenhttp.h"

#define RESPONSE_MAX_SIZE 10*1024*1024

#define checked_free(ptr) \
  if (ptr != NULL) { \
    free(ptr); \
    ptr = NULL; \
  }


int message_begin_cb (http_parser*);
int url_cb(http_parser*, const char*, size_t);
int header_field_cb(http_parser*, const char*, size_t);
int header_value_cb(http_parser*, const char*, size_t);
int body_cb(http_parser*, const char*, size_t);

static http_parser parser;

static http_parser_settings settings = {
  .on_message_begin = message_begin_cb,
  .on_header_field = header_field_cb,
  .on_header_value = header_value_cb,
  .on_url = url_cb,
  .on_body = body_cb,
  .on_headers_complete = 0,
  .on_message_complete = 0
};

char* method_to_string(unsigned char method) {
  switch(method) {
    case HTTP_DELETE:
      return "DELETE";
    case HTTP_GET:
      return "GET";
    case HTTP_HEAD:
      return "HEAD";
    case HTTP_POST:
      return "POST";
    case HTTP_PUT:
      return "PUT";
  }
  return "";
}

int ken_http_parse(ken_http_request_t* request, const char* buf, size_t len) {
  size_t parsed;

  http_parser_init(&parser, HTTP_REQUEST);
  parser.data = request;
  parsed = http_parser_execute(&parser, &settings, buf, len);

  request->method = method_to_string(parser.method);

  return parsed == len;
}

void ken_http_header_free(ken_http_header_t* header) {
  while (header != NULL) {
    checked_free(header->name);
    checked_free(header->value);
    header = header->next;
  }
}

void ken_http_request_init(ken_http_request_t* request) {
  request->socket = -1;
  request->method = NULL;
  request->uri = NULL;
  request->body = NULL;
  request->headers = NULL;
}

void ken_http_request_free(ken_http_request_t* request) {
  checked_free(request->uri);
  checked_free(request->body);
  ken_http_header_free(request->headers);
  ken_http_request_init(request);
}

void ken_http_response_init(ken_http_response_t* response) {
  response->status = NULL;
  response->body = NULL;
  response->headers = NULL;
}

void ken_http_response_free(ken_http_response_t* response) {
  response->status_code = 0;

  checked_free(response->status);
  checked_free(response->body);
  ken_http_header_free(response->headers);
}

int message_begin_cb (http_parser* p) {
  int socket;
  ken_http_request_t* request;

  request = p->data;

  /* Make sure the request is freed. */
  socket = request->socket;
  ken_http_request_free(request);
  request->socket = socket;

  return 0;
}

int url_cb(http_parser* p, const char* buf, size_t len) {
  ken_http_request_t* request;

  request = p->data;
  request->uri = malloc(sizeof(char) * (len + 1));
  request->uri[len] = '\0';
  memcpy(request->uri, buf, len);

  return 0;
}

int header_field_cb(http_parser* p, const char* buf, size_t len) {
  ken_http_request_t* request;
  ken_http_header_t* header;

  request = p->data;

  header = malloc(sizeof(ken_http_header_t));
  header->value = NULL;
  header->name = malloc(sizeof(char) * (len + 1));
  header->name[len] = '\0';
  memcpy(header->name, buf, len);

  header->next = request->headers;
  request->headers = header;

  return 0;
}

int header_value_cb(http_parser* p, const char* buf, size_t len) {
  ken_http_request_t* request;
  ken_http_header_t* header;

  request = p->data;

  header = request->headers;
  header->value = malloc(sizeof(char) * (len + 1));
  header->value[len] = '\0';
  memcpy(header->value, buf, len);

  return 0;
}

int body_cb(http_parser* p, const char* buf, size_t len) {
  ken_http_request_t* request;

  request = p->data;
  request->body = malloc(sizeof(char) * (len + 1));
  request->body[len] = '\0';
  memcpy(request->body, buf, len);

  return 0;
}

int ken_http_compose(ken_http_response_t* response, char* buf, size_t len) {
  ken_http_header_t* header;
  int bytes;

  bytes = snprintf(buf, len, "HTTP/1.1 %d %s\r\n", response->status_code, response->status);
  buf += bytes;

  header = response->headers;
  while (header != NULL) {
    bytes = snprintf(buf, len, "%s: %s\r\n", header->name, header->value);
    buf += bytes;
    header = header->next;
  }

  bytes = snprintf(buf, len, "\r\n%s", response->body);
  buf += bytes;
  *buf = '\0';

  return 1;
}

void ken_http_send(ken_http_request_t* request, ken_http_response_t* response) {
  static char buf[RESPONSE_MAX_SIZE];

  if (ken_http_compose(response, buf, RESPONSE_MAX_SIZE)) {
    ssize_t len = send(request->socket, buf, strlen(buf), 0);
    KENASRT(len != -1);
  }
}

void ken_http_close(ken_http_request_t* request) {
  if (request->socket != -1) {
    close(request->socket);
    request->socket = -1;
  }
}