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
#include "TimeLib.h"
#include "atcrust.h"
#include "sys/_stdint.h"

char msgText_chars[MSGTEXT_MAX_LEN]; // = 0;
String httpText = " ";
String nameText = " ";
String msgBodyText = " ";
String titlText = " ";
String tickText = " ";


NotfData notfData = {
 {},
 0,
};

NotfData *get_notf_data() {
  return &notfData;
}

void show_notf(String notfString) {
  read_notification(&notfData.notfs, &notfData.notf_count, (const uint8_t *) notfString.c_str());

  set_sleep_time();
}

void show_notf_c(const char *notfString) {
  read_notification(&notfData.notfs, &notfData.notf_count, (const uint8_t *) notfString);

  set_sleep_time();
}

int get_notf_total() {
  return notfData.notf_count;
}

Notf *get_notf(int idx) {
  return &notfData.notfs[idx];
}



void init_push() {
  msgText_chars[0] = 0;
}

String filter_string(String str)
{
  int i = 0, len = str.length();
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

void set_text_c(char *dst, String src, uint8_t maxlen) {
  int max = min(maxlen, src.length());
  dst[max] = 0;
  for (; max >= 0; --max) {
    dst[max] = src[max];
  }
}

void show_push_wakeup() {
  sleep_up(WAKEUP_BLEPUSH);
  display_notify();
  set_motor_ms();
  set_led_ms(100);
  set_sleep_time();
}

const char* show_push_get_buffer() {
  return msgText_chars;
}

void show_push(const char *msg, uint8_t len) {
  memcpy(msgText_chars, msg, min(MSGTEXT_MAX_LEN, len));
  show_push_wakeup();
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
  msgBodyText = filter_string("Body: " + bodyMSG);
  set_sleep_time();
}

void show_titl(String titlMSG) {
  titlText = filter_string("Title: " + titlMSG);
  set_sleep_time();
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

char *get_push_msg_c() {
  return msgText_chars;
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