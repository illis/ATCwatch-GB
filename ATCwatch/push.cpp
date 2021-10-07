/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Arduino.h"
#include "sleep.h"
#include "menu.h"
#include "display.h"
#include "inputoutput.h"
#include "push.h"

String msgText = " ";
String httpText = " ";
String nameText = " ";
String msgBodyText = " ";
String titlText = " ";
String tickText = " ";

Notf notfs[NOTF_MAX];
uintptr_t notf_used = 0;

void init_push() {
}


String filter_string_len(String str, int len)
{
  int i = 0;
  while (i < len)
  {
    char c = str[i];
    if ((c >= 0x20))
    {
      ++i;
    }
    else
    {
      str.remove(i, 1);
      --len;
    }
  }
  return str;
}

String filter_string(String str)
{
  return filter_string_len(str, str.length());
}

void show_push(String pushMSG) {
  int commaIndex = pushMSG.indexOf(',');
  int secondCommaIndex = pushMSG.indexOf(',', commaIndex + 1);
  int lastCommaIndex = pushMSG.indexOf(',', secondCommaIndex + 1);
  String MsgText = pushMSG.substring(commaIndex + 1, secondCommaIndex);
  int timeShown = pushMSG.substring(secondCommaIndex + 1, lastCommaIndex).toInt();
  int SymbolNr = pushMSG.substring(lastCommaIndex + 1).toInt();
  msgText = filter_string(MsgText);
  sleep_up(WAKEUP_BLEPUSH);
  display_notify();
  set_motor_ms();
  set_led_ms(100);
  set_sleep_time();
}

void show_http(String httpMSG) {
  httpText = filter_string("http: " + httpMSG);
  set_motor_ms();
  set_sleep_time();
}

void show_appName(String nameMSG) {
  nameText = filter_string("App: " + nameMSG);
  set_sleep_time();
}

void show_tick(String tickMSG) {
  tickText = filter_string("Text: " + tickMSG);
  set_sleep_time();
}

void show_msgBody(String bodyMSG) {
  msgBodyText = filter_string(bodyMSG);
  set_sleep_time();
}

void show_titl(String titlMSG) {
  titlText = filter_string(titlMSG);
  set_sleep_time();
}

char *strncpy_filtered(char *dest, const char *src, size_t n)
{
  int is,id = 0;
  while (is < n) {
    if (src[is] >= 0x20) {
      dest[id] = src[is];
      id++;
    }
    is++;
  }

  return dest;
}

void show_notf(String notfString) {
  read_notification(&notfs, &notf_used, (const uint8_t *) notfString.c_str());

  set_sleep_time();
}

int get_notf_total() {
  return notf_used;
}

Notf *get_notf(int idx) {
  return &notfs[idx];
}


String get_http_msg(int returnLength) {
  if (returnLength != 0 || httpText.length() == returnLength) {
    if (httpText.length() < returnLength) {
      String tempText = httpText;
      int toSmall = returnLength - httpText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (httpText.length() > returnLength)
      return httpText.substring(0, returnLength - 3) + "...";
  }
  return httpText;
}

String get_push_msg(int returnLength) {
  if (returnLength != 0 || msgText.length() == returnLength) {
    if (msgText.length() < returnLength) {
      String tempText = msgText;
      int toSmall = returnLength - msgText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (msgText.length() > returnLength)
      return msgText.substring(0, returnLength - 3) + "...";
  }
  return msgText;
}

String get_name_msg(int returnLength) {
  if (returnLength != 0 || nameText.length() == returnLength) {
    if (nameText.length() < returnLength) {
      String tempText = nameText;
      int toSmall = returnLength - nameText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (nameText.length() > returnLength)
      return nameText.substring(0, returnLength - 3) + "...";
  }
  return nameText;
}

String get_body_msg(int returnLength) {
  if (returnLength != 0 || msgBodyText.length() == returnLength) {
    if (msgBodyText.length() < returnLength) {
      String tempText = msgBodyText;
      int toSmall = returnLength - msgBodyText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (msgBodyText.length() > returnLength)
      return msgBodyText.substring(0, returnLength - 3) + "...";
  }
  return msgBodyText;
}

String get_titl_msg(int returnLength) {
  if (returnLength != 0 || titlText.length() == returnLength) {
    if (titlText.length() < returnLength) {
      String tempText = titlText;
      int toSmall = returnLength - titlText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (titlText.length() > returnLength)
      return titlText.substring(0, returnLength - 3) + "...";
  }
  return titlText;
}

String get_tick_msg(int returnLength) {
  if (returnLength != 0 || tickText.length() == returnLength) {
    if (tickText.length() < returnLength) {
      String tempText = tickText;
      int toSmall = returnLength - tickText.length();
      for (int i = 0; i < toSmall; i++) {
        tempText += " ";
      }
      return tempText;
    } else if (tickText.length() > returnLength)
      return tickText.substring(0, returnLength - 3) + "...";
  }
  return tickText;
}
