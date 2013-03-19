#include "v8-ken.h"
#include "v8-ken-data.h"
#include "v8-ken-v8.h"

#include "isolate.h"

// For persisting statics
#include "api.h"
#include "elements.h"
#include "heap.h"
#include "hydrogen.h"
#include "lithium.h"
#include "v8.h"
#include "zone.h"
#include "x64/assembler-x64.h"
#include "extensions/externalize-string-extension.h"
#include "extensions/gc-extension.h"

namespace v8 {
namespace ken {

Data* Data::instance() {
  return (Data*) ken_get_app_data();
}

Data* Data::initialize() {
  Data* data = new Data();
  ken_set_app_data(data);
  return data;
}

Data::Data() {
  pid_ = getpid();
  persist_count_ = 0;
  persists_ = (Persist*) ken_malloc(MAX_PERSISTS * sizeof(Data));
  memset(persists_, 0, MAX_PERSISTS * sizeof(Data));

  // Set default isolate
  isolate_ = v8::internal::Isolate::New();
  v8::internal::Isolate::RestoreDefaultIsolate(isolate_);

  // Initialize V8 after the isolate
  v8_ = new V8();

  v8::RegisteredExtension::initialize_persists(this);
  v8::internal::Heap::initialize_persists(this);
  v8::internal::V8::initialize_persists(this);
  v8::internal::Zone::initialize_persists(this);
  v8::internal::CpuFeatures::initialize_persists(this);
  v8::internal::GCExtension::initialize_persists(this);
  v8::internal::ExternalizeStringExtension::initialize_persists(this);
  v8::internal::LConstantOperand::initialize_persists(this);
  v8::internal::LStackSlot::initialize_persists(this);
  v8::internal::LDoubleStackSlot::initialize_persists(this);
  v8::internal::LRegister::initialize_persists(this);
  v8::internal::LDoubleRegister::initialize_persists(this);
  v8::internal::ElementsAccessor::initialize_persists(this);
  v8::internal::HStatistics::initialize_persists(this);
  v8::internal::HTracer::initialize_persists(this);
  v8::internal::RuntimeProfiler::initialize_persists(this);

  persist(&v8::internal::return_address_location_resolver);
}

void Data::persist(void* system, size_t size) {
  NTF(persists_ != NULL);

  Persist* current = &persists_[persist_count_];
  current->system = system; NTF(current->system != NULL);
  current->ken = ken_malloc(size); NTF(current->ken != NULL);
  memcpy(current->ken, system, size);
  current->size = size; NTF(current->size != 0);

  persist_count_++;
}

void Data::save() {
  for (uint32_t index = 0; index < persist_count_; index++) {
    Persist* persist = &persists_[index];
    memcpy(persist->ken, persist->system, persist->size);
  }
}

void Data::restore() {
  pid_ = getpid();

  for (uint32_t index = 0; index < persist_count_; index++) {
    Persist* persist = &persists_[index];
    memcpy(persist->system, persist->ken, persist->size);
  }

  // Restore default isolate
  v8::internal::Isolate::RestoreDefaultIsolate(isolate_);

    // Verify heap
  v8::internal::Isolate::Current()->heap()->Verify();

  // Reset stack guard
  v8::internal::ExecutionAccess access(isolate_);
  isolate_->stack_guard()->ClearThread(access);
  isolate_->stack_guard()->InitThread(access);
}

}
}
