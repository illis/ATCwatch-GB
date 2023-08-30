/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"
#include <BLEPeripheral.h>
#include "pinout.h"
#include "atczig.h"

bool enable_gps_view;
GPSData gps_data;

void init_ble();
void ble_feed();
void ble_ConnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
#ifdef D6NOTIFICATION
void ble_written(BLECentral& central, BLECharacteristic& characteristic);
void ble_write(String Command);
#endif // D6NOTIFICATION
void ble_written_bangle(BLECentral& central, BLECharacteristic& characteristic);
void ble_tx_bangle(const char* cmd, uint16_t len);
void bangleSubscribed(BLECentral& central, BLECharacteristic& characteristic);
bool get_vars_ble_connected();
void set_vars_ble_connected(bool state);
void filterCmd(String Command);
