#include "displayapp/screens/settings/SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingWakeUp::Option, 5> SettingWakeUp::options;

namespace {
  void event_handler(lv_event_t* event) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(event));
    auto* screen = static_cast<SettingWakeUp*>(obj->user_data);
    screen->UpdateSelected(obj);
  }
}

SettingWakeUp::SettingWakeUp(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container1 = lv_obj_create(lv_screen_active());
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(container1, 5, LV_PART_MAIN);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);

  lv_obj_set_pos(container1, 10, 35);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Wake Up");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);
  lv_label_set_text_static(icon, Symbols::eye);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    if (settingsController.isWakeUpModeOn(static_cast<Controllers::Settings::WakeUpMode>(i))) {
      lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
    }
    cbOption[i]->user_data = this;
    lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_VALUE_CHANGED, cbOption[i]);
  }
}

SettingWakeUp::~SettingWakeUp() {
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void SettingWakeUp::UpdateSelected(lv_obj_t* object) {
  // Find the index of the checkbox that triggered the event
  for (size_t i = 0; i < options.size(); i++) {
    if (cbOption[i] == object) {
      bool currentState = settingsController.isWakeUpModeOn(options[i].wakeUpMode);
      settingsController.setWakeUpMode(options[i].wakeUpMode, !currentState);
      break;
    }
  }

  // Update checkbox according to current wakeup modes.
  // This is needed because we can have extra logic when setting or unsetting wakeup modes,
  // for example, when setting SingleTap, DoubleTap is unset and vice versa.
  auto modes = settingsController.getWakeUpModes();
  for (size_t i = 0; i < options.size(); ++i) {
    if (modes[i]) {
      lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
    } else {
      lv_obj_remove_state(cbOption[i], LV_STATE_CHECKED);
    }
  }
}
