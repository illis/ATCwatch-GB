/*
   Copyright (c) 2020 Aaron Christophel

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "accl.h"
#include "accl_config.h"
#include "Arduino.h"
#include "i2c.h"
#include "pinout.h"
#include "watchdog.h"
#include "inputoutput.h"
#include "ble.h"
#include "sleep.h"

struct accl_data_struct accl_data;

void init_accl() {
#ifndef ACCL_DISABLE
  pinMode(BMA421_INT, INPUT);
  reset_accl();
  delay(10);
  accl_write_reg(0x7C, 0x00);//Sleep disable
  delay(1);
  accl_write_reg(0x59, 0x00);//Write Blob
  accl_config_read_write(0, 0x5E, (uint8_t *)accl_config_file, sizeof(accl_config_file), 0);
  accl_write_reg(0x59, 0x01);
  delay(10);
  accl_write_reg(0x7D, 0x04);//Accl Enable
  accl_write_reg(0x40, 0b00101000);//Acc Conf
  reset_step_counter();//Enable and Reset
  accl_write_reg(0x7C, 0x03);//Sleep Enable
#endif
}

void accl_config_read_write(bool rw, uint8_t addr, uint8_t *data, uint32_t len, uint32_t offset)
{
#ifndef ACCL_DISABLE
  for (int i = 0; i < len; i += 16) {
    accl_write_reg(0x5B, (offset + (i / 2)) & 0x0F);
    accl_write_reg(0x5C, (offset + (i / 2)) >> 4);
    if (rw)
      user_i2c_read(0x18, 0x5E, data + i, (len - i >= 16) ? 16 : (len - i));
    else
      user_i2c_write(0x18, 0x5E, data + i, (len - i >= 16) ? 16 : (len - i));
  }
#endif
}

void reset_accl() {
#ifndef ACCL_DISABLE
  accl_write_reg(0x7E, 0xB6);
#endif
}

void reset_step_counter() {
#ifndef ACCL_DISABLE
  uint8_t feature_config[0x47] = { 0 };
  accl_write_reg(0x7C, 0x00);//Sleep disable
  delay(1);
  accl_config_read_write(1, 0x5E, feature_config, 0x46, 256);
  feature_config[0x3A + 1] = 0x34;
  accl_config_read_write(0, 0x5E, feature_config, 0x46, 256);
  accl_write_reg(0x7C, 0x03);//Sleep Enable
#endif
}

uint32_t read_step_data() {
#ifndef ACCL_DISABLE
  uint32_t data;
  user_i2c_read(0x18, 0x1E, (uint8_t *)&data, 4);
  return data;
#else // ACCL_DISABLE
  return 0;
#endif
}

int last_y_acc = 0;
bool acc_input() {
#ifndef ACCL_DISABLE
  update_accl_data();
  if ((accl_data.x + 335) <= 670 && accl_data.z < 0) {
    if (!get_sleep()) {
      if (accl_data.y <= 0) {
        return false;
      } else {
        last_y_acc = 0;
        return false;
      }
    }
    if (accl_data.y >= 0) {
      last_y_acc = 0;
      return false;
    }
    if (accl_data.y + 230 < last_y_acc) {
      last_y_acc = accl_data.y;
      return true;
    }
  }
#endif
  return false;
}

bool get_is_looked_at() {
#ifndef ACCL_DISABLE
  update_accl_data();
  if ((accl_data.y + 300) <= 600 && (accl_data.x + 300) <= 600 && accl_data.z < 100)
    return true;
#endif
  return false;
}

accl_data_struct get_accl_data() {
#ifndef ACCL_DISABLE
  update_accl_data();
  accl_data.steps = read_step_data();
  accl_data.activity = accl_read_reg(0x27);
  accl_data.temp = accl_read_reg(0x22) + 23;
#endif
  return accl_data;
}

void update_accl_data() {
#ifndef ACCL_DISABLE
  user_i2c_read(0x18, 0x12, (uint8_t *)&accl_data.x, 6);
#ifdef SWITCH_X_Y // pinetime has 90° rotated Accl
  short tempX = accl_data.x;
  accl_data.x = accl_data.y;
  accl_data.y = tempX;
#endif
  accl_data.x = (accl_data.x / 0x10);
  accl_data.y = (accl_data.y / 0x10);
  accl_data.z = (accl_data.z / 0x10);
#endif
}

void accl_write_reg(uint8_t reg, uint8_t data) {
#ifndef ACCL_DISABLE
  user_i2c_write(0x18, reg, &data, 1);
#endif
}

uint8_t accl_read_reg(uint8_t reg) {
#ifndef ACCL_DISABLE
  uint8_t data;
  user_i2c_read(0x18, reg, &data, 1);
  return data;
#else // ACCL_DISABLE
  return 0;
#endif
}
