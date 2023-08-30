/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ble.h"
#include "pinout.h"
#include <BLEPeripheral.h>
#include "sleep.h"
#include "time.h"
#include "battery.h"
#include "inputoutput.h"
#include "backlight.h"
#include "bootloader.h"
#include "push.h"
#include "accl.h"
#include "TimeLib.h"

BLEPeripheral                   blePeripheral           = BLEPeripheral();

#ifdef D6NOTIFICATION
BLEService                      main_service     = BLEService("190A");
BLECharacteristic   TXchar        = BLECharacteristic("0002", BLENotify, 20);
BLECharacteristic   RXchar        = BLECharacteristic("0001", BLEWriteWithoutResponse, 20);
#endif // D6NOTIFICATION

bool vars_ble_connected = false;

BLEService bangleService =                       BLEService("6e400001b5a3f393e0a9e50e24dcca9e");
BLECharacteristic   bangleTXchar        = BLECharacteristic("6e400003b5a3f393e0a9e50e24dcca9e", BLENotify, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
BLECharacteristic   bangleRXchar        = BLECharacteristic("6e400002b5a3f393e0a9e50e24dcca9e", BLEWriteWithoutResponse, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);


char bleRxData_buffer[ANDROID_MAX_MSG_LENGTH];
BLERxData bleRxData;

void init_ble() {
  blePeripheral.setLocalName("Espruino");
  blePeripheral.setConnectionInterval(400,401);
  blePeripheral.setAdvertisingInterval(500);
  blePeripheral.setDeviceName("Espruino");

  blePeripheral.setAdvertisedServiceUuid(bangleService.uuid());
  blePeripheral.addAttribute(bangleService);
  blePeripheral.addAttribute(bangleTXchar);
  blePeripheral.addAttribute(bangleRXchar);
  bangleRXchar.setEventHandler(BLEWritten, ble_written_bangle);

#ifdef D6NOTIFICATION
  blePeripheral.setAdvertisedServiceUuid(main_service.uuid());
  blePeripheral.addAttribute(main_service);
  blePeripheral.addAttribute(TXchar);
  blePeripheral.addAttribute(RXchar);
  RXchar.setEventHandler(BLEWritten, ble_written);
#endif //  D6NOTIFICATION

  blePeripheral.setEventHandler(BLEConnected, ble_ConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, ble_DisconnectHandler);
  blePeripheral.begin();
  ble_feed();

  enable_gps_view = false;
  bleRxData = BLERxData {
    .buffer_pos = 0,
    .buffer = (const char *) bleRxData_buffer,
    .short_buffer_len = MSGTEXT_MAX_LEN,
    .short_msg_buffer = show_push_get_buffer(),
    .notfs = get_notf_data(),
    .gps_data = &gps_data,
    .set_time_cb = setTime,
    .wakeup_cb = show_push_wakeup,
    .tx_cb = ble_tx_bangle,
  };
}

void ble_tx_bangle(const char* cmd, uint16_t len) {
  uint16_t i = 0;
  while (i < len) {
    bangleTXchar.setValue(cmd + i);
    i += BLE_ATTRIBUTE_MAX_VALUE_LENGTH;
  }
  bangleTXchar.setValue("\n");
}

void ble_written_bangle(BLECentral& central, BLECharacteristic& characteristic) {
  handle_ble_rx(&bleRxData, characteristic.value(), characteristic.valueLength());
}

void ble_feed() {
  blePeripheral.poll();

}

void ble_ConnectHandler(BLECentral& central) {
  sleep_up(WAKEUP_BLECONNECTED);
  set_vars_ble_connected(true);
}

void ble_DisconnectHandler(BLECentral& central) {
  sleep_up(WAKEUP_BLEDISCONNECTED);
  set_vars_ble_connected(false);
}

String answer = "";
String tempCmd = "";
int tempLen = 0, tempLen1;
boolean syn;

#ifdef D6NOTIFICATION
void ble_written(BLECentral& central, BLECharacteristic& characteristic) {
  char remoteCharArray[22];
  tempLen1 = characteristic.valueLength();
  tempLen = tempLen + tempLen1;
  memset(remoteCharArray, 0, sizeof(remoteCharArray));
  memcpy(remoteCharArray, characteristic.value(), tempLen1);
  tempCmd = tempCmd + remoteCharArray;
  if (tempCmd[tempLen - 2] == '\r' && tempCmd[tempLen - 1] == '\n') {
    answer = tempCmd.substring(0, tempLen - 2);
    tempCmd = "";
    tempLen = 0;
    filterCmd(answer);
  }
}

void ble_write(String Command) {
  Command = Command + "\r\n";
  while (Command.length() > 0) {
    const char* TempSendCmd;
    String TempCommand = Command.substring(0, 20);
    TempSendCmd = &TempCommand[0];
    TXchar.setValue(TempSendCmd);
    Command = Command.substring(20);
  }
}

#endif // D6NOTIFICATION

bool get_vars_ble_connected() {
  return vars_ble_connected;
}

void set_vars_ble_connected(bool state) {
  vars_ble_connected = state;
}

#ifdef D6NOTIFICATION
void filterCmd(String Command) {
  if (Command == "AT+BOND") {
    ble_write("AT+BOND:OK");
  } else if (Command == "AT+ACT") {
    ble_write("AT+ACT:0");
  } else if (Command.substring(0, 7) == "BT+UPGB") {
    start_bootloader();
  } else if (Command.substring(0, 8) == "BT+RESET") {
    set_reboot();
  } else if (Command.substring(0, 7) == "AT+RUN=") {
    ble_write("AT+RUN:" + Command.substring(7));
  } else if (Command.substring(0, 8) == "AT+USER=") {
    ble_write("AT+USER:" + Command.substring(8));
  } else if (Command == "AT+PACE") {
    accl_data_struct accl_data = get_accl_data();
    ble_write("AT+PACE:" + String(accl_data.steps));
  } else if (Command == "AT+BATT") {
    ble_write("AT+BATT:" + String(get_battery_percent()));
  } else if (Command.substring(0, 8) == "AT+PUSH=") {
    ble_write("AT+PUSH:OK");
    String msg = Command.substring(8);
    show_push(msg.c_str(), msg.length());
  } else if (Command == "BT+VER") {
    ble_write("BT+VER:P8");
  } else if (Command == "AT+VER") {
    ble_write("AT+VER:P8");
  } else if (Command == "AT+SN") {
    ble_write("AT+SN:P8");
  } else if (Command.substring(0, 12) == "AT+CONTRAST=") {
    String contrastTemp = Command.substring(12);
    if (contrastTemp == "100")
      set_backlight(1);
    else if (contrastTemp == "175")
      set_backlight(3);
    else set_backlight(7);
    ble_write("AT+CONTRAST:" + Command.substring(12));
  } else if (Command.substring(0, 10) == "AT+MOTOR=1") {
    String motor_power = Command.substring(10);
    if (motor_power == "1")
      set_motor_power(50);
    else if (motor_power == "2")
      set_motor_power(200);
    else set_motor_power(350);
    ble_write("AT+MOTOR:1" + Command.substring(10));
    set_motor_ms();
  } else if (Command.substring(0, 6) == "AT+DT=") {
    SetDateTimeString(Command.substring(6));
    ble_write("AT+DT:" + GetDateTimeString());
  } else if (Command.substring(0, 5) == "AT+DT") {
    ble_write("AT+DT:" + GetDateTimeString());
  } else if (Command.substring(0, 8) == "AT+HTTP=") {
    show_http(Command.substring(8));
  } else if (Command.substring(0, 8) == "AT+NAME=") {
    show_appName(Command.substring(8));
  } else if (Command.substring(0, 8) == "AT+TITL=") {
    show_titl(Command.substring(8));
  } else if (Command.substring(0, 8) == "AT+BODY=") {
    show_msgBody(Command.substring(8));
  } else if (Command.substring(0, 8) == "AT+TICK=") {
    show_msgBody(Command.substring(8));
  }
}
#endif // D6NOTIFICATION
