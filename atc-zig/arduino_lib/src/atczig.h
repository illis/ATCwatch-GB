#ifndef atczig_h
#define atczig_h

#pragma once

#include <cstdint>
#include <ctime>
#include "atcrust_c.h"

extern "C" {

  int32_t add(int32_t a, int32_t b);

  typedef void (*tx_callback)(const char *msg, uint8_t len);
  typedef void (*show_notf_callback)(const char *s);
  typedef void (*set_time_callback)(time_t epoch);
  void process_bangle_input(const char *s, const uint8_t len, tx_callback tx_cb, set_time_callback set_time_cb, show_notf_callback show_notf_cb, struct NotfData *notfData, const char* short_msg_buffer, uint8_t short_buffer_len);
  void generate_short_notf_string(Notf *n, const char *buffer, uint8_t buffer_len);

  // for debug
  char *to_string_int_c(uint8_t i);

} // extern "C"

#endif // atczig_h
