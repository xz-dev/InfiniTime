#include "displayapp/screens/WatchFaceCasioStyleG7710.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
using namespace Pinetime::Applications::Screens;

WatchFaceCasioStyleG7710::WatchFaceCasioStyleG7710(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificatioManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::FS& filesystem)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_dot40 = lv_binfont_create("F:/fonts/lv_font_dots_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment40 = lv_binfont_create("F:/fonts/7segments_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_115.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment115 = lv_binfont_create("F:/fonts/7segments_115.bin");
  }

  label_battery_value = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_battery_value, lv_screen_active(), LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_text_color(label_battery_value, color_text, LV_PART_MAIN);
  lv_label_set_text_static(label_battery_value, "00%");

  batteryIcon.Create(lv_screen_active());
  batteryIcon.SetColor(color_text);
  lv_obj_align_to(batteryIcon.GetObject(), label_battery_value, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  batteryPlug = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(batteryPlug, color_text, LV_PART_MAIN);
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align_to(batteryPlug, batteryIcon.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(bleIcon, color_text, LV_PART_MAIN);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align_to(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(notificationIcon, color_text, LV_PART_MAIN);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align_to(notificationIcon, bleIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  label_day_of_week = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_day_of_week, lv_screen_active(), LV_ALIGN_TOP_LEFT, 10, 64);
  lv_obj_set_style_text_color(label_day_of_week, color_text, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_day_of_week, font_dot40, LV_PART_MAIN);
  lv_label_set_text_static(label_day_of_week, "SUN");

  label_week_number = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_week_number, lv_screen_active(), LV_ALIGN_TOP_LEFT, 5, 22);
  lv_obj_set_style_text_color(label_week_number, color_text, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_week_number, font_dot40, LV_PART_MAIN);
  lv_label_set_text_static(label_week_number, "WK26");

  label_day_of_year = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_day_of_year, lv_screen_active(), LV_ALIGN_TOP_LEFT, 100, 30);
  lv_obj_set_style_text_color(label_day_of_year, color_text, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_day_of_year, font_segment40, LV_PART_MAIN);
  lv_label_set_text_static(label_day_of_year, "181-184");

  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 2);
  lv_style_set_line_color(&style_line, color_text);
  lv_style_set_line_rounded(&style_line, true);

  lv_style_init(&style_border);
  lv_style_set_line_width(&style_border, 6);
  lv_style_set_line_color(&style_border, color_text);
  lv_style_set_line_rounded(&style_border, true);

  line_icons = lv_line_create(lv_screen_active());
  lv_line_set_points(line_icons, line_icons_points, 3);
  lv_obj_add_style(line_icons, &style_line, LV_PART_MAIN);
  lv_obj_align_to(line_icons, nullptr, LV_ALIGN_TOP_RIGHT, -10, 18);

  line_day_of_week_number = lv_line_create(lv_screen_active());
  lv_line_set_points(line_day_of_week_number, line_day_of_week_number_points, 4);
  lv_obj_add_style(line_day_of_week_number, &style_border, LV_PART_MAIN);
  lv_obj_align_to(line_day_of_week_number, nullptr, LV_ALIGN_TOP_LEFT, 0, 8);

  line_day_of_year = lv_line_create(lv_screen_active());
  lv_line_set_points(line_day_of_year, line_day_of_year_points, 3);
  lv_obj_add_style(line_day_of_year, &style_line, LV_PART_MAIN);
  lv_obj_align_to(line_day_of_year, nullptr, LV_ALIGN_TOP_RIGHT, 0, 60);

  label_date = lv_label_create(lv_screen_active());
  lv_obj_align_to(label_date, lv_screen_active(), LV_ALIGN_TOP_LEFT, 100, 70);
  lv_obj_set_style_text_color(label_date, color_text, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_date, font_segment40, LV_PART_MAIN);
  lv_label_set_text_static(label_date, "6-30");

  line_date = lv_line_create(lv_screen_active());
  lv_line_set_points(line_date, line_date_points, 3);
  lv_obj_add_style(line_date, &style_line, LV_PART_MAIN);
  lv_obj_align_to(line_date, nullptr, LV_ALIGN_TOP_RIGHT, 0, 100);

  label_time = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_time, color_text, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_time, font_segment115, LV_PART_MAIN);
  lv_obj_align_to(label_time, lv_screen_active(), LV_ALIGN_CENTER, 0, 40);

  line_time = lv_line_create(lv_screen_active());
  lv_line_set_points(line_time, line_time_points, 3);
  lv_obj_add_style(line_time, &style_line, LV_PART_MAIN);
  lv_obj_align_to(line_time, nullptr, LV_ALIGN_BOTTOM_RIGHT, 0, -25);

  label_time_ampm = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_time_ampm, color_text, LV_PART_MAIN);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align_to(label_time_ampm, lv_screen_active(), LV_ALIGN_LEFT_MID, 5, -5);

  backgroundLabel = lv_label_create(lv_screen_active());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_screen_active());
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_text_color(heartbeatIcon, color_text, LV_PART_MAIN);
  lv_obj_align_to(heartbeatIcon, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(heartbeatValue, color_text, LV_PART_MAIN);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align_to(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepValue, color_text, LV_PART_MAIN);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align_to(stepValue, lv_screen_active(), LV_ALIGN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(stepIcon, color_text, LV_PART_MAIN);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceCasioStyleG7710::~WatchFaceCasioStyleG7710() {
  lv_timer_set_repeat_count(taskRefresh, 0);

  lv_style_reset(&style_line);
  lv_style_reset(&style_border);

  if (font_dot40 != nullptr) {
    lv_binfont_destroy(font_dot40);
  }

  if (font_segment40 != nullptr) {
    lv_binfont_destroy(font_segment40);
  }

  if (font_segment115 != nullptr) {
    lv_binfont_destroy(font_segment115);
  }

  lv_obj_clean(lv_screen_active());
}

void WatchFaceCasioStyleG7710::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
    lv_label_set_text_fmt(label_battery_value, "%d%%", batteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_align_to(label_battery_value, lv_screen_active(), LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_align_to(batteryIcon.GetObject(), label_battery_value, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align_to(batteryPlug, batteryIcon.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align_to(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align_to(notificationIcon, bleIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[2] = "A";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_align_to(label_time, lv_screen_active(), LV_ALIGN_CENTER, 0, 40);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      const char* weekNumberFormat = "%V";

      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t month_value = static_cast<uint8_t>(month);
      uint8_t day = dateTimeController.Day();
      int dayOfYear = dateTimeController.DayOfYear();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        // 24h mode: ddmmyyyy, first DOW=Monday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", day, month_value);
        weekNumberFormat = "%V"; // Replaced by the week number of the year (Monday as the first day of the week) as a decimal number
                                 // [01,53]. If the week containing 1 January has four or more days in the new year, then it is considered
                                 // week 1. Otherwise, it is the last week of the previous year, and the next week is week 1. Both January
                                 // 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
      } else {
        // 12h mode: mmddyyyy, first DOW=Sunday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", month_value, day);
        weekNumberFormat = "%U"; // Replaced by the week number of the year as a decimal number [00,53]. The first Sunday of January is the
                                 // first day of week 1; days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
      }

      time_t ttTime =
        std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDateTime.Get()));
      tm* tmTime = std::localtime(&ttTime);

      // TODO: When we start using C++20, use std::chrono::year::is_leap
      int daysInCurrentYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 366 : 365;
      uint16_t daysTillEndOfYearNumber = daysInCurrentYear - dayOfYear;

      char buffer[8];
      strftime(buffer, 8, weekNumberFormat, tmTime);
      uint8_t weekNumber = atoi(buffer);

      lv_label_set_text_fmt(label_day_of_week, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(label_day_of_year, "%3d-%3d", dayOfYear, daysTillEndOfYearNumber);
      lv_label_set_text_fmt(label_week_number, "WK%02d", weekNumber);

      lv_obj_align_to(label_day_of_week, lv_screen_active(), LV_ALIGN_TOP_LEFT, 10, 64);
      lv_obj_align_to(label_day_of_year, lv_screen_active(), LV_ALIGN_TOP_LEFT, 100, 30);
      lv_obj_align_to(label_week_number, lv_screen_active(), LV_ALIGN_TOP_LEFT, 5, 22);
      lv_obj_align_to(label_date, lv_screen_active(), LV_ALIGN_TOP_LEFT, 100, 70);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_text_color(heartbeatIcon, color_text, LV_PART_MAIN);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0x1B1B1B), LV_PART_MAIN);
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_align_to(heartbeatIcon, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 5, -2);
    lv_obj_align_to(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align_to(stepValue, lv_screen_active(), LV_ALIGN_BOTTOM_RIGHT, -5, -2);
    lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }
}

bool WatchFaceCasioStyleG7710::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_115.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
