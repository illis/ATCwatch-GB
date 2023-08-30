/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "Arduino.h"
#include "class.h"
#include "images.h"
#include "menu.h"
#include "display.h"
#include "ble.h"
#include "time.h"
#include "menu_Home.h"

class TrackerScreen : public Screen
{
  public:
    virtual void pre()
    {
      lv_obj_t * mbox1 = lv_mbox_create(lv_scr_act(), NULL);
      lv_mbox_set_text(mbox1, "Enable GPS?");
      update_button_text(mbox1);

      lv_obj_set_width(mbox1, 200);
      lv_obj_set_event_cb(mbox1, event_handler);
      lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0);
    }

    virtual void main()
    {

    }

    virtual void right()
    {
      set_last_menu();
    }

  private:
    static void update_button_text(lv_obj_t * obj)
    {
      if (enable_gps_view) {
        static const char * btns[] = {"No", ""};
        lv_mbox_add_btns(obj, btns);

      } else {
        static const char * btns[] = {"Yes", ""};
        lv_mbox_add_btns(obj, btns);
      }
    }

    static void event_handler(lv_obj_t * obj, lv_event_t event)
    {
      if (event == LV_EVENT_VALUE_CHANGED) {
        if (enable_gps_view) {
          const char* msg = "{ \"t\":\"gps_power\",\"status\":false } \0";
          ble_tx_bangle(msg, strlen(msg) + 1);
          enable_gps_view = false;
        } else {
          const char* msg = "{ \"t\":\"gps_power\",\"status\":true } \0";
          ble_tx_bangle(msg, strlen(msg));
          enable_gps_view = true;
        }

        update_button_text(obj);
      }
    }
};

TrackerScreen trackerScreen;
