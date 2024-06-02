#include "displayapp/screens/settings/SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_Y_TEXT = -7;

  void SetTimeEventHandler(lv_event_t* event) {
    auto* screen = static_cast<SettingSetTime*>(lv_event_get_user_data(event));
    screen->SetTime();
  }

  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetTime*>(userData);
    screen->UpdateScreen();
  }
}

SettingSetTime::SettingSetTime(Pinetime::Controllers::DateTime& dateTimeController,
                               Pinetime::Controllers::Settings& settingsController,
                               Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, settingSetDateTime {settingSetDateTime} {

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Set current time");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* staticLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(staticLabel, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_static(staticLabel, "00:00:00");
  lv_obj_align_to(staticLabel, lv_screen_active(), LV_ALIGN_CENTER, 0, POS_Y_TEXT);

  hourCounter.Create();
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();
  }
  hourCounter.SetValue(dateTimeController.Hours());
  lv_obj_align_to(hourCounter.GetObject(), nullptr, LV_ALIGN_CENTER, -75, POS_Y_TEXT);
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  minuteCounter.SetValue(dateTimeController.Minutes());
  lv_obj_align_to(minuteCounter.GetObject(), nullptr, LV_ALIGN_CENTER, 0, POS_Y_TEXT);
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  lblampm = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(lblampm, &jetbrains_mono_bold_20, LV_PART_MAIN);
  lv_label_set_text_static(lblampm, "  ");
  lv_obj_align_to(lblampm, lv_screen_active(), LV_ALIGN_CENTER, 75, -50);

  btnSetTime = lv_btn_create(lv_screen_active());
  lv_obj_set_size(btnSetTime, 120, 50);
  lv_obj_align_to(btnSetTime, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);
  lblSetTime = lv_label_create(btnSetTime);
  lv_label_set_text_static(lblSetTime, "Set");
  lv_obj_set_style_bg_color(btnSetTime, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_style_text_color(lblSetTime, LV_COLOR_GRAY, LV_STATE_DISABLED);
  lv_obj_add_event_cb(btnSetTime, SetTimeEventHandler, LV_EVENT_CLICKED, this);

  UpdateScreen();
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_screen_active());
}

void SettingSetTime::UpdateScreen() {
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (hourCounter.GetValue() >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
}

void SettingSetTime::SetTime() {
  const int hoursValue = hourCounter.GetValue();
  const int minutesValue = minuteCounter.GetValue();
  NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
  dateTimeController.SetTime(dateTimeController.Year(),
                             static_cast<uint8_t>(dateTimeController.Month()),
                             dateTimeController.Day(),
                             static_cast<uint8_t>(hoursValue),
                             static_cast<uint8_t>(minutesValue),
                             0);
  settingSetDateTime.Quit();
}
