#include "displayapp/screens/settings/SettingSetDate.h"
#include "displayapp/screens/settings/SettingSetDateTime.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_DAY = -72;
  constexpr int16_t POS_X_MONTH = 0;
  constexpr int16_t POS_X_YEAR = 72;
  constexpr int16_t POS_Y_TEXT = -6;

  void event_handler(lv_event_t* event) {
    auto* screen = static_cast<SettingSetDate*>(lv_event_get_user_data(event));
    screen->HandleButtonPress();
  }

  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetDate*>(userData);
    screen->CheckDay();
  }

  int MaximumDayOfMonth(uint8_t month, uint16_t year) {
    switch (month) {
      case 2: {
        // TODO: When we start using C++20, use std::chrono::year::is_leap
        if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0)) {
          return 29;
        }
        return 28;
      }
      case 4:
      case 6:
      case 9:
      case 11:
        return 30;
      default:
        return 31;
    }
  }
}

SettingSetDate::SettingSetDate(Pinetime::Controllers::DateTime& dateTimeController,
                               Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime)
  : dateTimeController {dateTimeController}, settingSetDateTime {settingSetDateTime} {

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Set current date");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);

  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  dayCounter.SetValueChangedEventCallback(this, ValueChangedHandler);
  dayCounter.Create();
  dayCounter.SetValue(dateTimeController.Day());
  lv_obj_align_to(dayCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);

  monthCounter.EnableMonthMode();
  monthCounter.SetValueChangedEventCallback(this, ValueChangedHandler);
  monthCounter.Create();
  monthCounter.SetValue(static_cast<int>(dateTimeController.Month()));
  lv_obj_align_to(monthCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_TEXT);

  yearCounter.SetValueChangedEventCallback(this, ValueChangedHandler);
  yearCounter.Create();
  yearCounter.SetValue(dateTimeController.Year());
  lv_obj_align_to(yearCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_TEXT);

  btnSetTime = lv_btn_create(lv_screen_active());
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align_to(btnSetTime, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(btnSetTime, LV_COLOR_MAKE(0x38, 0x38, 0x38), LV_PART_MAIN);
  lblSetTime = lv_label_create(btnSetTime);
  lv_label_set_text_static(lblSetTime, "Set");
  lv_obj_add_event_cb(btnSetTime, event_handler, LV_EVENT_CLICKED, this);
}

SettingSetDate::~SettingSetDate() {
  lv_obj_clean(lv_screen_active());
}

void SettingSetDate::HandleButtonPress() {
  const uint16_t yearValue = yearCounter.GetValue();
  const uint8_t monthValue = monthCounter.GetValue();
  const uint8_t dayValue = dayCounter.GetValue();
  NRF_LOG_INFO("Setting date (manually) to %04d-%02d-%02d", yearValue, monthValue, dayValue);
  dateTimeController
    .SetTime(yearValue, monthValue, dayValue, dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds());
  settingSetDateTime.Advance();
}

void SettingSetDate::CheckDay() {
  const int maxDay = MaximumDayOfMonth(monthCounter.GetValue(), yearCounter.GetValue());
  dayCounter.SetMax(maxDay);
}
