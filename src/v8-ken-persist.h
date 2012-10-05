#ifndef V8_KEN_PERSIST_H
#define V8_KEN_PERSIST_H

V8KenData* create_v8_ken_data() {
  V8KenData* data = (V8KenData*) ken_malloc(sizeof(V8KenData));
  ken_set_app_data(data);
  return data;
}

void save_v8_ken_data(V8KenData* data) {
}

void restore_v8_ken_data(V8KenData* data) {
}

#endif
