#include "displayapp/screens/settings/SettingSteps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    SettingSteps* screen = static_cast<SettingSteps*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingSteps::SettingSteps(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* container1 = lv_obj_create(lv_screen_active());
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(container1, 5, LV_PART_MAIN);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);
  lv_obj_set_pos(container1, 30, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 50);
  lv_obj_set_height(container1, LV_VER_RES - 60);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Daily steps goal");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);

  lv_label_set_text_static(icon, Symbols::shoe);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  stepValue = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(stepValue, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
  lv_obj_set_style_text_align(stepValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(stepValue, lv_screen_active(), LV_ALIGN_CENTER, 0, -20);

  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;

  btnPlus = lv_btn_create(lv_screen_active());
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, btnWidth, btnHeight);
  lv_obj_align_to(btnPlus, lv_screen_active(), LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(btnPlus, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_t* lblPlus = lv_label_create(btnPlus);
  lv_obj_set_style_text_font(lblPlus, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_static(lblPlus, "+");
  lv_obj_add_event_cb(btnPlus, event_handler, LV_EVENT_ALL, btnPlus);

  btnMinus = lv_btn_create(lv_screen_active());
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, btnWidth, btnHeight);
  lv_obj_add_event_cb(btnMinus, event_handler, LV_EVENT_ALL, btnMinus);
  lv_obj_align_to(btnMinus, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_bg_color(btnMinus, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_t* lblMinus = lv_label_create(btnMinus);
  lv_obj_set_style_text_font(lblMinus, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_static(lblMinus, "-");
}

SettingSteps::~SettingSteps() {
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void SettingSteps::UpdateSelected(lv_obj_t* object, lv_event_t* event) {
  uint32_t value = settingsController.GetStepsGoal();
  lv_event_code_t code = lv_event_get_code(event);

  int valueChange = 0;
  if (code == LV_EVENT_SHORT_CLICKED) {
    valueChange = 500;
  } else if (code == LV_EVENT_LONG_PRESSED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    valueChange = 1000;
  } else {
    return;
  }

  if (object == btnPlus) {
    value += valueChange;
  } else if (object == btnMinus) {
    value -= valueChange;
  }

  if (value >= 1000 && value <= 500000) {
    settingsController.SetStepsGoal(value);
    lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
    lv_obj_align_to(stepValue, lv_screen_active(), LV_ALIGN_CENTER, 0, -20);
  }
}
