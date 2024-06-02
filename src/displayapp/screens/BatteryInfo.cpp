#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/DisplayApp.h"
#include "components/battery/BatteryController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

BatteryInfo::BatteryInfo(const Pinetime::Controllers::Battery& batteryController) : batteryController {batteryController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  charging_bar = lv_bar_create(lv_screen_active());
  lv_obj_set_size(charging_bar, 200, 15);
  lv_bar_set_range(charging_bar, 0, 100);
  lv_obj_align_to(charging_bar, nullptr, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_anim_duration(charging_bar, 1000, LV_PART_MAIN);
  lv_obj_set_style_radius(charging_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(charging_bar, Colors::bgAlt, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(charging_bar, LV_OPA_100, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(charging_bar, LV_COLOR_RED, LV_PART_MAIN);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);

  status = lv_label_create(lv_screen_active());
  lv_label_set_text_static(status, "Reading Battery status");
  lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  percent = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(percent, &jetbrains_mono_76, LV_PART_MAIN);
  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);
  lv_obj_set_style_text_align(percent, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
  lv_obj_align_to(percent, nullptr, LV_ALIGN_CENTER, 0, -60);

  voltage = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(voltage, Colors::orange, LV_PART_MAIN);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_obj_set_style_text_align(voltage, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(voltage, nullptr, LV_ALIGN_CENTER, 0, 95);

  taskRefresh = lv_timer_create(RefreshTaskCallback, 5000, this);
  Refresh();
}

BatteryInfo::~BatteryInfo() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void BatteryInfo::Refresh() {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  if (batteryController.IsCharging()) {
    lv_obj_set_style_bg_color(charging_bar, LV_COLOR_RED, LV_PART_MAIN);
    lv_label_set_text_static(status, "Charging");
  } else if (batteryPercent == 100) {
    lv_obj_set_style_bg_color(charging_bar, LV_COLOR_BLUE, LV_PART_MAIN);
    lv_label_set_text_static(status, "Fully charged");
  } else if (batteryPercent < 10) {
    lv_obj_set_style_bg_color(charging_bar, LV_COLOR_YELLOW, LV_PART_MAIN);
    lv_label_set_text_static(status, "Battery low");
  } else {
    lv_obj_set_style_bg_color(charging_bar, Colors::highlight, LV_PART_MAIN);
    lv_label_set_text_static(status, "Discharging");
  }

  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);

  lv_obj_align_to(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);
}
