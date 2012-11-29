#ifndef V8_KEN_DATA_STATICS_H
#define V8_KEN_DATA_STATICS_H

#include "api.h"
#include "v8.h"
#include "x64/assembler-x64.h"
#include "extensions/externalize-string-extension.h"
#include "extensions/gc-extension.h"

class Statics {
public:
  void save() {
    api_first_extension_ = ::v8::RegisteredExtension::first_extension_;
    api_stress_type_ = v8::internal::Testing::stress_type_;

    v8_is_running_ = v8::internal::V8::is_running_;
    v8_has_been_set_up_ = v8::internal::V8::has_been_set_up_;
    v8_has_fatal_error_ = v8::internal::V8::has_fatal_error_;
    v8_has_been_disposed_ = v8::internal::V8::has_been_disposed_;
    v8_use_crankshaft_ = v8::internal::V8::use_crankshaft_;
    v8_call_completed_callbacks_ = v8::internal::V8::call_completed_callbacks_;

    heap_initialized_gc = v8::internal::initialized_gc;

    zone_allocation_size_ = v8::internal::Zone::allocation_size_;

#ifdef DEBUG
    x64_initialized_ = v8::internal::CpuFeatures::initialized_;
#endif
    x64_supported_ = v8::internal::CpuFeatures::supported_;
    x64_found_by_runtime_probing_ = v8::internal::CpuFeatures::found_by_runtime_probing_;

    gc_extension_ = v8::internal::gc_extension;
  }

  void restore() {
    v8::RegisteredExtension::first_extension_ = api_first_extension_;
    v8::internal::Testing::stress_type_ = api_stress_type_;

    v8::internal::V8::is_running_ = v8_is_running_;
    v8::internal::V8::has_been_set_up_ = v8_has_been_set_up_;
    v8::internal::V8::has_fatal_error_ = v8_has_fatal_error_;
    v8::internal::V8::has_been_disposed_ = v8_has_been_disposed_;
    v8::internal::V8::use_crankshaft_ = v8_use_crankshaft_;
    v8::internal::V8::call_completed_callbacks_ = v8_call_completed_callbacks_;

    v8::internal::initialized_gc = heap_initialized_gc;

    v8::internal::Zone::allocation_size_ = zone_allocation_size_;

#ifdef DEBUG
    v8::internal::CpuFeatures::initialized_ = x64_initialized_;
#endif
    v8::internal::CpuFeatures::supported_ = x64_supported_;
    v8::internal::CpuFeatures::found_by_runtime_probing_ = x64_found_by_runtime_probing_;

    v8::internal::gc_extension = gc_extension_;
  }

private:
  /* api.h */
  v8::RegisteredExtension* api_first_extension_;
  v8::Testing::StressType api_stress_type_;

  /* v8.h */
  bool v8_is_running_;
  bool v8_has_been_set_up_;
  bool v8_has_fatal_error_;
  bool v8_has_been_disposed_;
  bool v8_use_crankshaft_;
  v8::internal::List<v8::CallCompletedCallback>* v8_call_completed_callbacks_;

  /* heap.h */
  bool heap_initialized_gc;

  /* zone.h */
  unsigned zone_allocation_size_;

  /* x64/assembler-x64.h */
#ifdef DEBUG
  bool x64_initialized_;
#endif
  uint64_t x64_supported_;
  uint64_t x64_found_by_runtime_probing_;

  /* extensions/gc-extension.h */
  v8::internal::GCExtension* gc_extension_;
};

#endif
