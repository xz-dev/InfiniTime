#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceTerminal::WatchFaceTerminal(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::HeartRateController& heartRateController,
                                     Controllers::MotionController& motionController)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {
  batteryValue = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(batteryValue, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, -20);

  connectState = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(connectState, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, 40);

  notificationIcon = lv_label_create(lv_screen_active());
  lv_obj_align_to(notificationIcon, nullptr, LV_ALIGN_LEFT_MID, 0, -100);

  label_date = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(label_date, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, -40);

  label_prompt_1 = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_prompt_1, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, -80);
  lv_label_set_text_static(label_prompt_1, "user@watch:~ $ now");

  label_prompt_2 = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_prompt_2, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, 60);
  lv_label_set_text_static(label_prompt_2, "user@watch:~ $");

  label_time = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(label_time, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, -60);

  heartbeatValue = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(heartbeatValue, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, 20);

  stepValue = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(stepValue, lv_screen_active(), LV_ALIGN_LEFT_MID, 0, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceTerminal::~WatchFaceTerminal() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void WatchFaceTerminal::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_span_t* span = lv_spangroup_new_span(batteryValue);
    lv_span_set_text(span, "[BATT] ");
    lv_span_t* span2 = lv_spangroup_new_span(batteryValue);
    char batteryTextFmt[13] = "%d%%";
    if (batteryController.IsPowerPresent()) {
      std::strcat(batteryTextFmt, " Charging");
    }
    char* batteryText = get_text_fmt(batteryTextFmt, batteryPercentRemaining.Get());
    lv_span_set_text(span2, batteryText);
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x38, 0x7b, 0x54));
    lv_spangroup_refr_mode(batteryValue);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_span_t* span = lv_spangroup_new_span(connectState);
    lv_span_set_text(span, "[STAT] ");
    lv_span_t* span2 = lv_spangroup_new_span(connectState);
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x00, 0x82, 0xfc));
    if (!bleRadioEnabled.Get()) {
      lv_span_set_text(span2, "Disabled");
    } else {
      if (bleState.Get()) {
        lv_span_set_text(span2, "Connected");
      } else {
        lv_span_set_text(span2, "Disconnected");
      }
    }
    lv_spangroup_refr_mode(connectState);
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have mail.");
    } else {
      lv_label_set_text_static(notificationIcon, "");
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();
    lv_span_t* span = lv_spangroup_new_span(label_time);
    lv_span_set_text(span, "[TIME] ");
    lv_span_t* span2 = lv_spangroup_new_span(label_time);
    char* text;

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      text = get_text_fmt("%02d:%02d:%02d %s", hour, minute, second, ampmChar);
    } else {
      text = get_text_fmt("%02d:%02d:%02d", hour, minute, second);
    }
    lv_span_set_text(span2, text);
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x11, 0xcc, 0x55));
    lv_spangroup_refr_mode(label_time);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      lv_span_t* span = lv_spangroup_new_span(label_date);
      lv_span_set_text(span, "[DATE] ");
      lv_span_t* span2 = lv_spangroup_new_span(label_date);
      lv_span_set_text(span2, get_text_fmt("%04d-%02d-%02d", short(year), char(month), char(day)));
      lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x00, 0x7f, 0xff));
      lv_spangroup_refr_mode(label_date);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    lv_span_t* span = lv_spangroup_new_span(heartbeatValue);
    lv_span_set_text(span, "[HR] ");
    lv_span_t* span2 = lv_spangroup_new_span(heartbeatValue);
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0xee, 0x33, 0x11));
    if (heartbeatRunning.Get()) {
      lv_span_set_text(span2, get_text_fmt("%d bpm", heartbeat.Get()));
    } else {
      lv_span_set_text(span2, "---");
    }
    lv_spangroup_refr_mode(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_span_t* span = lv_spangroup_new_span(stepValue);
    lv_span_set_text(span, "[STEP] ");
    lv_span_t* span2 = lv_spangroup_new_span(stepValue);
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0xee, 0x33, 0x77));
    lv_span_set_text(span2, get_text_fmt("%lu steps", stepCount.Get()));
    lv_spangroup_refr_mode(stepValue);
  }
}
