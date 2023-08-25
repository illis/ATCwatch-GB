#ifndef atczig_h
#define atczig_h

#pragma once

#include <cstdint>
#include <ctime>
#include "atcrust_c.h"

extern "C" {
#define SHORT_MSG_MAX_LEN 30

  typedef void (*tx_callback)(const char *msg, uint8_t len);
  typedef void (*show_notf_callback)(const char *s);
  typedef void (*set_time_callback)(time_t epoch);
  typedef void (*wakeup_callback)();

  // this also needs to be duplicated in the zib lib
  // typedef struct __attribute__ ((__packed__)) BLERxData {
  typedef struct BLERxData {
    uint16_t buffer_pos;
    const char* buffer;
    uint8_t short_buffer_len;
    const char* short_msg_buffer;
    struct NotfData* notfs;
    set_time_callback set_time_cb;
    wakeup_callback wakeup_cb;
  } BLERxData;

  int32_t add(int32_t a, int32_t b);

  void handle_ble_rx(BLERxData *bleRxData, const unsigned char *val, uint8_t val_len);
  void generate_short_notf_string(Notf *n, const char *buffer, uint8_t buffer_len);

  // for debug
  char *to_string_int_c(uint8_t i);

} // extern "C"

#endif // atczig_h
