/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "heartrate.h"
#include "pinout.h"
#include "i2c.h"
#include "inputoutput.h"
#include "sleep.h"
#ifndef HEARTRATE_DISABLE
#include "HRS3300lib.h"
#endif

bool heartrate_enable = false;
bool is_heartrate_enable = false;
byte last_heartrate_ms;
byte last_heartrate;

//timed heartrate stuff
bool timed_heart_rates = true;
bool has_good_heartrate = false;
int hr_answers;
bool disabled_hr_allready = false;

void init_hrs3300() {
#ifndef HEARTRATE_DISABLE

  pinMode(HRS3300_TEST, INPUT);
  HRS3300_begin(user_i2c_read, user_i2c_write);//set the i2c read and write function so it can be a user defined i2c hardware see i2c.h
  heartrate_enable = true;
  end_hrs3300();
#else
   pinMode(HRS3300_TEST, INPUT);
   // disable heartrate sensor
   // ref: https://files.pine64.org/doc/datasheet/pinetime/HRS3300%20Heart%20Rate%20Sensor.pdf
   user_i2c_write(0x44, 0x01, 0b00000000, 1);
    // hrs led driver set register
   user_i2c_write(0x44, 0x0c, 0b00000000, 1);
#endif // HEARTRATE
}

void start_hrs3300() {
#ifndef HEARTRATE_DISABLE
  if (!heartrate_enable) {
    HRS3300_enable();
    heartrate_enable = true;
  }
#endif
}

void end_hrs3300() {
#ifndef HEARTRATE_DISABLE
  if (heartrate_enable) {
    heartrate_enable = false;
    HRS3300_disable();
  }
#endif
}

byte get_heartrate() {
#ifndef HEARTRATE_DISABLE
  byte hr = last_heartrate_ms;
  switch (hr) {
    case 0:
      break;
    case 255:
      break;
    case 254://No Touch
      break;
    case 253://Please wait
      break;
    default:
      last_heartrate = hr;
      break;
  }
  return hr;
#else // HEARTRATE
  return 0x0;
#endif
}

byte get_last_heartrate() {
#ifndef HEARTRATE_DISABLE
  return last_heartrate;
#else // HEARTRATE
  return 0x0;
#endif
}

void get_heartrate_ms() {
#ifndef HEARTRATE_DISABLE
  if (heartrate_enable) {
    last_heartrate_ms = HRS3300_getHR();
  }
#endif // HEARTRATE
}

void check_timed_heartrate(int minutes) {
#ifndef HEARTRATE_DISABLE
  if (timed_heart_rates) {
    if (minutes == 0 || minutes == 15 || minutes == 30 || minutes == 45) {
      if (!has_good_heartrate) {
        disabled_hr_allready = false;
        start_hrs3300();
        byte hr = get_heartrate();
        if (hr > 0 && hr < 253) {
          hr_answers++;
          if (hr_answers >= 5) {
            has_good_heartrate = true;
          }
        } else if (hr == 254) {
          hr_answers++;
          if (hr_answers >= 10) {
            has_good_heartrate = true;
          }
        }
      } else {
        end_hrs3300();
      }
    } else {
      if (!disabled_hr_allready) {
        disabled_hr_allready = true;
        end_hrs3300();
        hr_answers = 0;
        has_good_heartrate = false;
      }
    }
  }
#endif // HEARTRATE
}
