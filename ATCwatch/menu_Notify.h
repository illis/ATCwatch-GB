/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
// #include "rust_lib_for_arduino_example.h"
#include "Arduino.h"
#include "class.h"
#include "images.h"
#include "menu.h"
#include "display.h"
#include "ble.h"
#include "time.h"
#include "battery.h"
#include "accl.h"
#include "push.h"
#include "heartrate.h"
#include "fonts.h"


class NotifyScreen : public Screen
{
  public:
    virtual void pre()
    {
      set_gray_screen_style(&sans_regular);

      lv_obj_t * img1 = lv_img_create(lv_scr_act(), NULL);
      lv_img_set_src(img1, &IsymbolMsg);
      lv_obj_align(img1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

      /*
      label_msg = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_long_mode(label_msg, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(label_msg,240);
      lv_label_set_text(label_msg, "");
      lv_label_set_text(label_msg, string2char(get_push_msg()));
      lv_obj_align(label_msg, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
      */

      label_msg_count = lv_label_create(lv_scr_act(), NULL);
      lv_obj_set_width(label_msg_count,240);
      lv_label_set_text(label_msg_count, "0/0\n");
      lv_obj_align(label_msg_count, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);


      label_msg_name = lv_label_create(lv_scr_act(), NULL);
      //lv_label_set_long_mode(label_msg_name, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(label_msg_name,240);
      lv_label_set_text(label_msg_name, "");
      lv_obj_align(label_msg_name, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

      label_msg_title = lv_label_create(lv_scr_act(), NULL);
      //lv_label_set_long_mode(label_msg_title, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(label_msg_title,240);
      lv_label_set_text(label_msg_title, "");
      lv_obj_align(label_msg_title, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 25);

      label_msg_body = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_long_mode(label_msg_body, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(label_msg_body,240);
      lv_label_set_text(label_msg_body, "");
      lv_obj_align(label_msg_body, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 50);

      notf_count = -1;
    }

    virtual void main()
    {
      // lv_label_set_text(label_msg_name, string2char(get_name_msg()));
      // lv_label_set_text(label_msg_title, string2char(get_titl_msg()));
      // lv_label_set_text(label_msg_body, string2char(get_body_msg()));

      int notf_total = get_notf_total();
      if (notf_total == 0) {
        notf_count = 0;

        set_labels(notf_count, notf_total, "", "", "");
      } else {
        // check bounds
        if (notf_count <= 0) {
          notf_count = notf_total;
        } else if (notf_count > notf_total) {
          notf_count = 1;
        }

        Notf *notf = get_notf(notf_count - 1);
        set_labels(notf_count, notf_total, (char *) notf->app_name, (char *) notf->title, (char *) notf->body);
      }
    }

    virtual void long_click()
    {
      display_home();
    }

    virtual void left()
    {
      display_home();
    }

    virtual void right()
    {
      display_home();
    }

    virtual void up()
    {
      notf_count--;
    }
    virtual void down()
    {
      notf_count++;
    }

    virtual void click(touch_data_struct touch_data)
    {
      display_home();
    }

  private:
    lv_obj_t *label, *label_msg_body, *label_msg_name, *label_msg_title, *label_msg, *label_msg_count;
    int notf_count;

    char* string2char(String command) {
      if (command.length() != 0) {
        char *p = const_cast<char*>(command.c_str());
        return p;
      }
    }

    void set_labels(int count, int total, char *appName, char *title, char *body) {
      lv_label_set_text_fmt(label_msg_count, "%d/%d\n", count, total);
      lv_label_set_text(label_msg_name, appName);
      lv_label_set_text(label_msg_title, title);
      lv_label_set_text(label_msg_body, body);
    }
};

NotifyScreen notifyScreen;
