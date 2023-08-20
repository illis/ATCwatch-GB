/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"
#include "atcrust.h"
#include "atczig.h"

#define MSGTEXT_MAX_LEN 30

void init_push();
void show_push_wakeup();
const char* show_push_get_buffer();
void show_push(const char *msg, uint8_t len);
void show_http(String httpMSG);
void show_appName(String nameMSG);
void show_msgBody(String bodyMSG);
void show_titl(String titlMSG);
void show_tick(String titlMSG);
void show_notf(String notf);
void show_notf_c(const char *notfString);
String get_http_msg(int returnLength=0);
char *get_push_msg_c();
String get_name_msg(int returnLength=0);
String get_body_msg(int returnLength=0);
String get_titl_msg(int returnLength=0);
String get_tick_msg(int returnLength=0);
int get_notf_total();
Notf *get_notf(int idx = 0);
void del_notf(int idx = 0);
NotfData *get_notf_data();
