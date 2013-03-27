#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kenhttp.h"

int isctl(char c) {
  return (c >= 0 && c <= 31) || c == 127;
}

char* strnchr(const char* s, size_t n, int ch) {
  if (n == 0) return NULL;

  while (*s) {
    if (*s == ch) return (char*) s;
    if (--n == 0) return NULL;
    ++s;
  }

  /* While stops at 0, but we might be looking for it */
  if (*s == ch) return (char*) s;
  return NULL;
}

char* strnpbrk(const char* s, size_t n, const char* brk) {
  const char* s1;

  while (*s) {
    for (s1 = brk; *s1 && *s1 != *s; ++s1);
    if (*s1) return (char*) s;
    if (--n == 0) return NULL;
    ++s;
  }

  return NULL;
}

size_t line_length(char* s) {
  char* temp;

  /* Find CRLF */
  temp = strstr(s, "\r\n");

  if (temp == NULL)
    return 0;

  assert(temp > s);

  /* Return length plus length of CRLF */
  return (temp - s) + 2;
}

int parse_request_line(char* line, size_t line_len,
    char** method, size_t* method_len,
    char** uri, size_t* uri_len,
    char** version, size_t* version_len) {
  char* temp;

  // Find method
  temp = strnchr(line, line_len, ' ');
  if (temp == NULL)
    return 0;

  *method = line;
  *method_len = (temp - line);

  // The (+ 1)'s are to skip the space
  assert(line_len > *method_len + 1);
  line += *method_len + 1;
  line_len -= *method_len + 1;

  // Find uri
  temp = strnchr(line, line_len, ' ');
  if (temp == NULL)
    return 0;

  *uri = line;
  *uri_len = (temp - line);

  // The (+ 1)'s are to skip the space
  assert(line_len > *uri_len + 1);
  line += *uri_len + 1;
  line_len -= *uri_len + 1;

  *version = line;
  *version_len = (temp - line);

  return 1;
}

int ken_http_parse(http_request_t* request, char* buffer, size_t length) {
  char* line = buffer, *method, *uri, *version;
  size_t line_len, method_len, uri_len, version_len;

  line_len = line_length(line);
  if (line_len == 0)
    return 0;

  // Read request method
  if (parse_request_line(line, line_len,
      &method, &method_len,
      &uri, &uri_len,
      &version, &version_len) == 0)
    return 0;

  memcpy(request->method, method, method_len);
  request->method[method_len] = '\0';

  memcpy(request->uri, uri, uri_len);
  request->uri[uri_len] = '\0';

  // Go to next line
  /*line += line_length;

  line_len = line_length(line);
  if (line_len == 0)
    return 0;*/

  return 1;
}

void ken_http_close(http_request_t* request) {
  if (request->socket != -1) {
    close(request->socket);
    request->socket = -1;
  }
}
