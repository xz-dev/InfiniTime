#include "displayapp/screens/settings/SettingShakeThreshold.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    SettingShakeThreshold* screen = static_cast<SettingShakeThreshold*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingShakeThreshold::SettingShakeThreshold(Controllers::Settings& settingsController,
                                             Controllers::MotionController& motionController,
                                             System::SystemTask& systemTask)
  : settingsController {settingsController}, motionController {motionController}, systemTask {systemTask} {

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Wake Sensitivity");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 0, 0);

  positionArc = lv_arc_create(lv_screen_active());
  positionArc->user_data = this;

  lv_obj_add_event_cb(positionArc, event_handler, LV_EVENT_VALUE_CHANGED, positionArc);
  lv_arc_set_bg_angles(positionArc, 180, 360);
  lv_arc_set_range(positionArc, 0, 4095);
  lv_obj_set_width(positionArc, lv_obj_get_width(lv_screen_active()) - 10);
  lv_obj_set_height(positionArc, 240);
  lv_obj_align_to(positionArc, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  animArc = lv_arc_create(positionArc);
  lv_obj_remove_style(animArc, NULL, LV_PART_KNOB);
  lv_obj_remove_flag(animArc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_width(animArc, lv_obj_get_width(positionArc));
  lv_obj_set_height(animArc, lv_obj_get_height(positionArc));
  lv_obj_align_to(animArc, positionArc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_line_opa(animArc, 0, LV_PART_MAIN);
  lv_obj_set_style_line_opa(animArc, LV_OPA_70, LV_PART_INDICATOR);
  lv_obj_set_style_line_opa(animArc, LV_OPA_0, LV_PART_KNOB);
  lv_obj_set_style_line_color(animArc, LV_COLOR_RED, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(animArc, LV_OPA_TRANSP, LV_STATE_CHECKED);

  animArc->user_data = this;
  lv_obj_remove_state(animArc, LV_STATE_CHECKED);

  calButton = lv_btn_create(lv_screen_active());
  calButton->user_data = this;
  lv_obj_add_event_cb(calButton, event_handler, LV_EVENT_VALUE_CHANGED, calButton);
  lv_obj_set_height(calButton, 80);
  lv_obj_set_width(calButton, 200);
  lv_obj_align_to(calButton, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(calButton, LV_OBJ_FLAG_CLICKABLE);
  calLabel = lv_label_create(calButton);
  lv_label_set_text_static(calLabel, "Calibrate");

  lv_arc_set_value(positionArc, settingsController.GetShakeThreshold());

  vDecay = xTaskGetTickCount();
  calibrating = false;
  EnableForCal = false;
  if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake)) {
    EnableForCal = true;
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, true);
  }
  refreshTask = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

SettingShakeThreshold::~SettingShakeThreshold() {
  settingsController.SetShakeThreshold(lv_arc_get_value(positionArc));

  if (EnableForCal) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, false);
    EnableForCal = false;
  }
  lv_timer_set_repeat_count(refreshTask, 0);
  settingsController.SaveSettings();
  lv_obj_clean(lv_screen_active());
}

void SettingShakeThreshold::Refresh() {

  if (calibrating == 1) {
    if (xTaskGetTickCount() - vCalTime > pdMS_TO_TICKS(2000)) {
      vCalTime = xTaskGetTickCount();
      calibrating = 2;
      lv_obj_set_style_bg_color(calButton, LV_COLOR_RED, LV_STATE_CHECKED);
      lv_label_set_text_static(calLabel, "Shake!");
    }
  }
  if (calibrating == 2) {

    if ((motionController.CurrentShakeSpeed() - 300) > lv_arc_get_value(positionArc)) {
      lv_arc_set_value(positionArc, (int16_t) motionController.CurrentShakeSpeed() - 300);
    }
    if (xTaskGetTickCount() - vCalTime > pdMS_TO_TICKS(7500)) {
      lv_obj_add_state(calButton, LV_STATE_DEFAULT);
      lv_obj_send_event(calButton, LV_EVENT_VALUE_CHANGED, nullptr);
    }
  }
  if (motionController.CurrentShakeSpeed() - 300 > lv_arc_get_value(animArc)) {
    lv_arc_set_value(animArc, (uint16_t) motionController.CurrentShakeSpeed() - 300);
    vDecay = xTaskGetTickCount();
  } else if ((xTaskGetTickCount() - vDecay) > pdMS_TO_TICKS(1500)) {
    lv_arc_set_value(animArc, lv_arc_get_value(animArc) - 25);
  }
}

void SettingShakeThreshold::UpdateSelected(lv_obj_t* object, [[maybe_unused]] lv_event_t* event) {
  if (object == calButton) {
    if (lv_obj_has_state(calButton, LV_STATE_CHECKED) && calibrating == 0) {
      lv_arc_set_value(positionArc, 0);
      calibrating = 1;
      vCalTime = xTaskGetTickCount();
      lv_label_set_text_static(calLabel, "Ready!");
      lv_obj_remove_flag(positionArc, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_style_bg_color(calButton, Colors::highlight, LV_STATE_CHECKED);
    } else if (lv_obj_has_state(calButton, LV_STATE_PRESSED)) {
      calibrating = 0;
      lv_obj_add_flag(positionArc, LV_OBJ_FLAG_CLICKABLE);
      lv_label_set_text_static(calLabel, "Calibrate");
    }
  }
}
