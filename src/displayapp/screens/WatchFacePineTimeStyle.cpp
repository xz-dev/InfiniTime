/*
 * This file is part of the Infinitime distribution (https://github.com/InfiniTimeOrg/Infinitime).
 * Copyright (c) 2021 Kieran Cawthray.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * PineTimeStyle watch face for Infinitime created by Kieran Cawthray
 * Based on WatchFaceDigital
 * Style/layout copied from TimeStyle for Pebble by Dan Tilden (github.com/tilden)
 */

#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include <lvgl/lvgl.h>
#include <cstdio>
#include <displayapp/Colors.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/SimpleWeatherService.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    auto* screen = static_cast<WatchFacePineTimeStyle*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFacePineTimeStyle::WatchFacePineTimeStyle(Controllers::DateTime& dateTimeController,
                                               const Controllers::Battery& batteryController,
                                               const Controllers::Ble& bleController,
                                               Controllers::NotificationManager& notificationManager,
                                               Controllers::Settings& settingsController,
                                               Controllers::MotionController& motionController,
                                               Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController},
    weatherService {weatherService} {

  // Create a 200px wide background rectangle
  timebar = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(timebar, Convert(settingsController.GetPTSColorBG()), LV_PART_MAIN);
  lv_obj_set_style_radius(timebar, 0, LV_PART_MAIN);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align_to(timebar, lv_screen_active(), LV_ALIGN_TOP_LEFT, 0, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(timeDD1, &open_sans_light, LV_PART_MAIN);
  lv_obj_set_style_text_color(timeDD1, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN);
  lv_label_set_text_static(timeDD1, "00");
  lv_obj_align_to(timeDD1, timebar, LV_ALIGN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(timeDD2, &open_sans_light, LV_PART_MAIN);
  lv_obj_set_style_text_color(timeDD2, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN);
  lv_label_set_text_static(timeDD2, "00");
  lv_obj_align_to(timeDD2, timebar, LV_ALIGN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(timeAMPM, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN);
  lv_obj_set_style_text_line_space(timeAMPM, -3, LV_PART_MAIN);
  lv_label_set_text_static(timeAMPM, "");
  lv_obj_align_to(timeAMPM, timebar, LV_ALIGN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(sidebar, Convert(settingsController.GetPTSColorBar()), LV_PART_MAIN);
  lv_obj_set_style_radius(sidebar, 0, LV_PART_MAIN);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align_to(sidebar, lv_screen_active(), LV_ALIGN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon.Create(sidebar);
  batteryIcon.SetColor(LV_COLOR_BLACK);
  lv_obj_align_to(batteryIcon.GetObject(), nullptr, LV_ALIGN_TOP_MID, 10, 2);

  plugIcon = lv_label_create(lv_screen_active());
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_set_style_text_color(plugIcon, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_align_to(plugIcon, sidebar, LV_ALIGN_TOP_MID, 10, 2);

  bleIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(bleIcon, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_align_to(bleIcon, sidebar, LV_ALIGN_TOP_MID, -10, 2);

  notificationIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(notificationIcon, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN);
  lv_obj_align_to(notificationIcon, timebar, LV_ALIGN_TOP_LEFT, 5, 5);

  weatherIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(weatherIcon, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_text_font(weatherIcon, &fontawesome_weathericons, LV_PART_MAIN);
  lv_label_set_text(weatherIcon, Symbols::ban);
  lv_obj_align_to(weatherIcon, sidebar, LV_ALIGN_TOP_MID, 0, 35);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_remove_flag(weatherIcon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(weatherIcon, LV_OBJ_FLAG_HIDDEN);
  }

  temperature = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(temperature, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text(temperature, "--");
  lv_obj_align_to(temperature, sidebar, LV_ALIGN_TOP_MID, 0, 65);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_remove_flag(temperature, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(temperature, LV_OBJ_FLAG_HIDDEN);
  }

  // Calendar icon
  calendarOuter = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarOuter, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarOuter, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarOuter, 34, 34);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 20);
  } else {
    lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);
  }

  calendarInner = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarInner, LV_COLOR_WHITE, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarInner, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarBar1, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarBar1, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarBar2, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarBar2, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarCrossBar1, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarCrossBar1, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(calendarCrossBar2, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_radius(calendarCrossBar2, 0, LV_PART_MAIN);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Display date
  dateDayOfWeek = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(dateDayOfWeek, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(dateDayOfWeek, "THU");
  lv_obj_align_to(dateDayOfWeek, calendarOuter, LV_ALIGN_CENTER, 0, -32);

  dateDay = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(dateDay, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(dateDay, "25");
  lv_obj_align_to(dateDay, calendarOuter, LV_ALIGN_CENTER, 0, 3);

  dateMonth = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(dateMonth, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(dateMonth, "MAR");
  lv_obj_align_to(dateMonth, calendarOuter, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  if (settingsController.GetPTSColorBar() == Pinetime::Controllers::Settings::Colors::White) {
    needle_colors[0] = LV_COLOR_BLACK;
  } else {
    needle_colors[0] = LV_COLOR_WHITE;
  }
  stepGauge = lv_scale_create(lv_screen_active());
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Full) {
    lv_obj_set_size(stepGauge, 40, 40);
    lv_obj_align_to(stepGauge, sidebar, LV_ALIGN_BOTTOM_MID, 0, 0);
  } else if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Half) {
    lv_obj_set_size(stepGauge, 37, 37);
    lv_obj_align_to(stepGauge, sidebar, LV_ALIGN_BOTTOM_MID, 0, -10);
  } else if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_add_flag(stepGauge, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(stepGauge, LV_OBJ_FLAG_HIDDEN);
  }

  lv_obj_set_style_pad_right(stepGauge, 3, LV_PART_MAIN);
  lv_obj_set_style_pad_left(stepGauge, 3, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(stepGauge, 3, LV_PART_MAIN);
  lv_obj_set_style_line_opa(stepGauge, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_line_width(stepGauge, 4, LV_PART_MAIN);
  lv_obj_set_style_line_color(stepGauge, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_obj_set_style_line_opa(stepGauge, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_line_width(stepGauge, 3, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(stepGauge, 4, LV_PART_MAIN);

  stepValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepValue, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align_to(stepValue, sidebar, LV_ALIGN_BOTTOM_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_remove_flag(stepValue, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(stepValue, LV_OBJ_FLAG_HIDDEN);
  }

  stepIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepIcon, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_TOP_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_remove_flag(stepIcon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(stepIcon, LV_OBJ_FLAG_HIDDEN);
  }

  // Display seconds
  timeDD3 = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(timeDD3, LV_COLOR_BLACK, LV_PART_MAIN);
  lv_label_set_text_static(timeDD3, ":00");
  lv_obj_align_to(timeDD3, sidebar, LV_ALIGN_BOTTOM_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Half) {
    lv_obj_remove_flag(timeDD3, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(timeDD3, LV_OBJ_FLAG_HIDDEN);
  }

  btnNextTime = lv_btn_create(lv_screen_active());
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align_to(btnNextTime, lv_screen_active(), LV_ALIGN_RIGHT_MID, -15, -80);
  lv_obj_set_style_bg_opa(btnNextTime, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblNextTime = lv_label_create(btnNextTime);
  lv_label_set_text_static(lblNextTime, ">");
  lv_obj_add_event_cb(btnNextTime, event_handler, LV_EVENT_CLICKED, btnNextTime);
  lv_obj_add_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);

  btnPrevTime = lv_btn_create(lv_screen_active());
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align_to(btnPrevTime, lv_screen_active(), LV_ALIGN_LEFT_MID, 15, -80);
  lv_obj_set_style_bg_opa(btnPrevTime, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblPrevTime = lv_label_create(btnPrevTime);
  lv_label_set_text_static(lblPrevTime, "<");
  lv_obj_add_event_cb(btnPrevTime, event_handler, LV_EVENT_CLICKED, btnPrevTime);
  lv_obj_add_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);

  btnNextBar = lv_btn_create(lv_screen_active());
  btnNextBar->user_data = this;
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align_to(btnNextBar, lv_screen_active(), LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_set_style_bg_opa(btnNextBar, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblNextBar = lv_label_create(btnNextBar);
  lv_label_set_text_static(lblNextBar, ">");
  lv_obj_add_event_cb(btnNextBar, event_handler, LV_EVENT_CLICKED, btnNextBar);
  lv_obj_add_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);

  btnPrevBar = lv_btn_create(lv_screen_active());
  btnPrevBar->user_data = this;
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align_to(btnPrevBar, lv_screen_active(), LV_ALIGN_LEFT_MID, 15, 0);
  lv_obj_set_style_bg_opa(btnPrevBar, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblPrevBar = lv_label_create(btnPrevBar);
  lv_label_set_text_static(lblPrevBar, "<");
  lv_obj_add_event_cb(btnPrevBar, event_handler, LV_EVENT_CLICKED, btnPrevBar);
  lv_obj_add_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);

  btnNextBG = lv_btn_create(lv_screen_active());
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align_to(btnNextBG, lv_screen_active(), LV_ALIGN_RIGHT_MID, -15, 80);
  lv_obj_set_style_bg_opa(btnNextBG, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblNextBG = lv_label_create(btnNextBG);
  lv_label_set_text_static(lblNextBG, ">");
  lv_obj_add_event_cb(btnNextBG, event_handler, LV_EVENT_CLICKED, btnNextBG);
  lv_obj_add_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);

  btnPrevBG = lv_btn_create(lv_screen_active());
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align_to(btnPrevBG, lv_screen_active(), LV_ALIGN_LEFT_MID, 15, 80);
  lv_obj_set_style_bg_opa(btnPrevBG, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblPrevBG = lv_label_create(btnPrevBG);
  lv_label_set_text_static(lblPrevBG, "<");
  lv_obj_add_event_cb(btnPrevBG, event_handler, LV_EVENT_CLICKED, btnPrevBG);
  lv_obj_add_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);

  btnReset = lv_btn_create(lv_screen_active());
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align_to(btnReset, lv_screen_active(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_opa(btnReset, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblReset = lv_label_create(btnReset);
  lv_label_set_text_static(lblReset, "Rst");
  lv_obj_add_event_cb(btnReset, event_handler, LV_EVENT_CLICKED, btnReset);
  lv_obj_add_flag(btnReset, LV_OBJ_FLAG_HIDDEN);

  btnRandom = lv_btn_create(lv_screen_active());
  btnRandom->user_data = this;
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align_to(btnRandom, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(btnRandom, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblRandom = lv_label_create(btnRandom);
  lv_label_set_text_static(lblRandom, "Rnd");
  lv_obj_add_event_cb(btnRandom, event_handler, LV_EVENT_CLICKED, btnRandom);
  lv_obj_add_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);

  btnClose = lv_btn_create(lv_screen_active());
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align_to(btnClose, lv_screen_active(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_bg_opa(btnClose, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblClose = lv_label_create(btnClose);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_add_event_cb(btnClose, event_handler, LV_EVENT_CLICKED, btnClose);
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);

  btnSteps = lv_btn_create(lv_screen_active());
  btnSteps->user_data = this;
  lv_obj_set_size(btnSteps, 160, 60);
  lv_obj_align_to(btnSteps, lv_screen_active(), LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_bg_opa(btnSteps, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblSteps = lv_label_create(btnSteps);
  lv_label_set_text_static(lblSteps, "Steps style");
  lv_obj_add_event_cb(btnSteps, event_handler, LV_EVENT_CLICKED, btnSteps);
  lv_obj_add_flag(btnSteps, LV_OBJ_FLAG_HIDDEN);

  btnWeather = lv_btn_create(lv_screen_active());
  btnWeather->user_data = this;
  lv_obj_set_size(btnWeather, 160, 60);
  lv_obj_align_to(btnWeather, lv_screen_active(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_bg_opa(btnWeather, LV_OPA_50, LV_PART_MAIN);
  lv_obj_t* lblWeather = lv_label_create(btnWeather);
  lv_label_set_text_static(lblWeather, "Weather");
  lv_obj_add_event_cb(btnWeather, event_handler, LV_EVENT_CLICKED, btnWeather);
  lv_obj_add_flag(btnWeather, LV_OBJ_FLAG_HIDDEN);

  btnSetColor = lv_btn_create(lv_screen_active());
  btnSetColor->user_data = this;
  lv_obj_set_size(btnSetColor, 150, 60);
  lv_obj_align_to(btnSetColor, lv_screen_active(), LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_radius(btnSetColor, 20, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(btnSetColor, LV_OPA_50, LV_PART_MAIN);
  lv_obj_add_event_cb(btnSetColor, event_handler, LV_EVENT_CLICKED, btnSetColor);
  lv_obj_t* lblSetColor = lv_label_create(btnSetColor);
  lv_obj_set_style_text_font(lblSetColor, &lv_font_sys_48, LV_PART_MAIN);
  lv_label_set_text_static(lblSetColor, Symbols::paintbrushLg);
  lv_obj_add_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN);

  btnSetOpts = lv_btn_create(lv_screen_active());
  btnSetOpts->user_data = this;
  lv_obj_set_size(btnSetOpts, 150, 60);
  lv_obj_align_to(btnSetOpts, lv_screen_active(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_radius(btnSetOpts, 20, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(btnSetOpts, LV_OPA_50, LV_PART_MAIN);
  lv_obj_add_event_cb(btnSetOpts, event_handler, LV_EVENT_CLICKED, btnSetOpts);
  lv_obj_t* lblSetOpts = lv_label_create(btnSetOpts);
  lv_obj_set_style_text_font(lblSetOpts, &lv_font_sys_48, LV_PART_MAIN);
  lv_label_set_text_static(lblSetOpts, Symbols::settings);
  lv_obj_add_flag(btnSetOpts, LV_OBJ_FLAG_HIDDEN);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFacePineTimeStyle::~WatchFacePineTimeStyle() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

bool WatchFacePineTimeStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_has_flag(btnClose, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_remove_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnSetOpts, LV_OBJ_FLAG_HIDDEN);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_has_flag(btnClose, LV_OBJ_FLAG_HIDDEN) == false)) {
    return true;
  }
  return false;
}

void WatchFacePineTimeStyle::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_add_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnReset, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnSteps, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnWeather, LV_OBJ_FLAG_HIDDEN);
}

bool WatchFacePineTimeStyle::OnButtonPushed() {
  if (!lv_obj_has_flag(btnClose, LV_OBJ_FLAG_HIDDEN)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFacePineTimeStyle::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFacePineTimeStyle::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_add_flag(batteryIcon.GetObject(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(plugIcon, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_flag(batteryIcon.GetObject(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(plugIcon, LV_OBJ_FLAG_HIDDEN);
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_align_to(bleIcon, sidebar, LV_ALIGN_TOP_MID, -10, 2);
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto hour = dateTimeController.Hours();
    auto minute = dateTimeController.Minutes();
    auto second = dateTimeController.Seconds();
    auto year = dateTimeController.Year();
    auto month = dateTimeController.Month();
    auto dayOfWeek = dateTimeController.DayOfWeek();
    auto day = dateTimeController.Day();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[4] = "A\nM";
        if (hour == 0) {
          hour = 12;
        } else if (hour == 12) {
          ampmChar[0] = 'P';
        } else if (hour > 12) {
          hour = hour - 12;
          ampmChar[0] = 'P';
        }
        lv_label_set_text(timeAMPM, ampmChar);
        // Should be padded with blank spaces, but the space character doesn't exist in the font
        lv_label_set_text_fmt(timeDD1, "%02d", hour);
        lv_label_set_text_fmt(timeDD2, "%02d", minute);
      } else {
        lv_label_set_text_fmt(timeDD1, "%02d", hour);
        lv_label_set_text_fmt(timeDD2, "%02d", minute);
      }
    }

    if (displayedSecond != second) {
      displayedSecond = second;
      lv_label_set_text_fmt(timeDD3, ":%02d", second);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_static(dateDayOfWeek, dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(dateDay, "%d", day);
      lv_obj_align_to(dateDay, calendarOuter, LV_ALIGN_CENTER, 0, 3);
      lv_label_set_text_static(dateMonth, dateTimeController.MonthShortToString());

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_obj_align_to(stepGauge, sidebar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_label_set_text_fmt(stepValue, "%luK", (stepCount.Get() / 1000));
    lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_TOP_MID, 0, 0);
    if (stepCount.Get() > settingsController.GetStepsGoal()) {
      lv_obj_set_style_line_color(stepGauge, LV_COLOR_WHITE, LV_PART_MAIN);
      lv_obj_set_style_line_color(stepGauge, LV_COLOR_WHITE, LV_PART_CURSOR);
    }
  }

  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
      }
      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
      lv_label_set_text_fmt(temperature, "%d°", temp);
      lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId));
    } else {
      lv_label_set_text(temperature, "--");
      lv_label_set_text(weatherIcon, Symbols::ban);
    }
    lv_obj_align_to(temperature, sidebar, LV_ALIGN_TOP_MID, 0, 65);
    lv_obj_align_to(weatherIcon, sidebar, LV_ALIGN_TOP_MID, 0, 35);
  }

  if (!lv_obj_has_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_add_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(btnSetOpts, LV_OBJ_FLAG_HIDDEN);
      savedTick = 0;
    }
  }
}

void WatchFacePineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t* /*event*/) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (object == btnNextTime) {
    valueTime = GetNext(valueTime);
    if (valueTime == valueBG) {
      valueTime = GetNext(valueTime);
    }
    settingsController.SetPTSColorTime(valueTime);
    lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN);
  }
  if (object == btnPrevTime) {
    valueTime = GetPrevious(valueTime);
    if (valueTime == valueBG) {
      valueTime = GetPrevious(valueTime);
    }
    settingsController.SetPTSColorTime(valueTime);
    lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN);
  }
  if (object == btnNextBar) {
    valueBar = GetNext(valueBar);
    if (valueBar == Controllers::Settings::Colors::Black) {
      valueBar = GetNext(valueBar);
    }
    if (valueBar == Controllers::Settings::Colors::White) {
      needle_colors[0] = LV_COLOR_BLACK;
    } else {
      needle_colors[0] = LV_COLOR_WHITE;
    }
    settingsController.SetPTSColorBar(valueBar);
    lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN);
  }
  if (object == btnPrevBar) {
    valueBar = GetPrevious(valueBar);
    if (valueBar == Controllers::Settings::Colors::Black) {
      valueBar = GetPrevious(valueBar);
    }
    if (valueBar == Controllers::Settings::Colors::White) {
      needle_colors[0] = LV_COLOR_BLACK;
    } else {
      needle_colors[0] = LV_COLOR_WHITE;
    }
    settingsController.SetPTSColorBar(valueBar);
    lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN);
  }
  if (object == btnNextBG) {
    valueBG = GetNext(valueBG);
    if (valueBG == valueTime) {
      valueBG = GetNext(valueBG);
    }
    settingsController.SetPTSColorBG(valueBG);
    lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN);
  }
  if (object == btnPrevBG) {
    valueBG = GetPrevious(valueBG);
    if (valueBG == valueTime) {
      valueBG = GetPrevious(valueBG);
    }
    settingsController.SetPTSColorBG(valueBG);
    lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN);
  }
  if (object == btnReset) {
    needle_colors[0] = LV_COLOR_WHITE;
    settingsController.SetPTSColorTime(Controllers::Settings::Colors::Teal);
    lv_obj_set_style_text_color(timeDD1, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeDD2, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeAMPM, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN);
    settingsController.SetPTSColorBar(Controllers::Settings::Colors::Teal);
    lv_obj_set_style_bg_color(sidebar, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN);
    settingsController.SetPTSColorBG(Controllers::Settings::Colors::Black);
    lv_obj_set_style_bg_color(timebar, Convert(Controllers::Settings::Colors::Black), LV_PART_MAIN);
  }
  if (object == btnRandom) {
    valueTime = static_cast<Controllers::Settings::Colors>(rand() % 17);
    valueBar = static_cast<Controllers::Settings::Colors>(rand() % 17);
    valueBG = static_cast<Controllers::Settings::Colors>(rand() % 17);
    if (valueTime == valueBG) {
      valueBG = GetNext(valueBG);
    }
    if (valueBar == Controllers::Settings::Colors::Black) {
      valueBar = GetPrevious(valueBar);
    }
    if (valueBar == Controllers::Settings::Colors::White) {
      needle_colors[0] = LV_COLOR_BLACK;
    } else {
      needle_colors[0] = LV_COLOR_WHITE;
    }
    settingsController.SetPTSColorTime(static_cast<Controllers::Settings::Colors>(valueTime));
    lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN);
    lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN);
    settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(valueBar));
    lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN);
    settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(valueBG));
    lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN);
  }
  if (object == btnClose) {
    CloseMenu();
  }
  if (object == btnSteps) {
    if (!lv_obj_has_flag(stepGauge, LV_OBJ_FLAG_HIDDEN) && (lv_obj_has_flag(timeDD3, LV_OBJ_FLAG_HIDDEN))) {
      // show half gauge & seconds
      lv_obj_remove_flag(timeDD3, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_size(stepGauge, 37, 37);
      lv_obj_align_to(stepGauge, sidebar, LV_ALIGN_BOTTOM_MID, 0, -10);
      settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Half);
    } else if (!lv_obj_has_flag(timeDD3, LV_OBJ_FLAG_HIDDEN) && (lv_obj_has_flag(stepValue, LV_OBJ_FLAG_HIDDEN))) {
      // show step count & icon
      lv_obj_add_flag(timeDD3, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(stepGauge, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(stepValue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(stepIcon, LV_OBJ_FLAG_HIDDEN);
      settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Numeric);
    } else {
      // show full gauge
      lv_obj_remove_flag(stepGauge, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(stepValue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(stepIcon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_size(stepGauge, 40, 40);
      lv_obj_align_to(stepGauge, sidebar, LV_ALIGN_BOTTOM_MID, 0, 0);
      settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Full);
    }
  }
  if (object == btnWeather) {
    if (lv_obj_has_flag(weatherIcon, LV_OBJ_FLAG_HIDDEN)) {
      // show weather icon and temperature
      lv_obj_remove_flag(weatherIcon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(temperature, LV_OBJ_FLAG_HIDDEN);
      lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 20);
      lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);
      lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);
      lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);
      lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);
      lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);
      lv_obj_align_to(dateDayOfWeek, calendarOuter, LV_ALIGN_CENTER, 0, -32);
      lv_obj_align_to(dateDay, calendarOuter, LV_ALIGN_CENTER, 0, 3);
      lv_obj_align_to(dateMonth, calendarOuter, LV_ALIGN_CENTER, 0, 32);
      settingsController.SetPTSWeather(Controllers::Settings::PTSWeather::On);
    } else {
      // hide weather
      lv_obj_add_flag(weatherIcon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(temperature, LV_OBJ_FLAG_HIDDEN);
      lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);
      lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);
      lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);
      lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);
      lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);
      lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);
      lv_obj_align_to(dateDayOfWeek, calendarOuter, LV_ALIGN_CENTER, 0, -32);
      lv_obj_align_to(dateDay, calendarOuter, LV_ALIGN_CENTER, 0, 3);
      lv_obj_align_to(dateMonth, calendarOuter, LV_ALIGN_CENTER, 0, 32);
      settingsController.SetPTSWeather(Controllers::Settings::PTSWeather::Off);
    }
  }
  if (object == btnSetColor) {
    lv_obj_add_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btnSetOpts, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnReset, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
  }
  if (object == btnSetOpts) {
    lv_obj_add_flag(btnSetColor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btnSetOpts, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnSteps, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnWeather, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
  }
}

Pinetime::Controllers::Settings::Colors WatchFacePineTimeStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 17) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors WatchFacePineTimeStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(17);
  }
  return prevColor;
}
