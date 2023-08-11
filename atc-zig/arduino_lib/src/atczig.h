#ifndef atczig_h
#define atczig_h

#pragma once

#include <cstdint>
#include "atcrust_c.h"

extern "C" {

  int32_t add(int32_t a, int32_t b);

  typedef void (*show_notf_callback)(const char *s);
  typedef void (*set_time_callback)(long epoch);
  void process_bangle_input(const char *s, const uint8_t len, set_time_callback set_time_cb, show_notf_callback show_notf_cb, struct NotfData *notfData);

} // extern "C"

#endif // atczig_h
