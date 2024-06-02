#include "displayapp/screens/settings/SettingDisplay.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t* event) {
    auto* obj = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    auto* screen = static_cast<SettingDisplay*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

constexpr std::array<uint16_t, 6> SettingDisplay::options;

SettingDisplay::SettingDisplay(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app}, settingsController {settingsController} {

  lv_obj_t* container1 = lv_obj_create(lv_screen_active());
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(container1, 5, LV_PART_MAIN);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);

  lv_obj_t* title = lv_label_create(lv_screen_active());
  lv_label_set_text_static(title, "Display timeout");
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);
  lv_label_set_text_static(icon, Symbols::sun);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  char buffer[4];
  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1);
    snprintf(buffer, sizeof(buffer), "%2" PRIu16 "s", options[i] / 1000);
    lv_checkbox_set_text(cbOption[i], buffer);
    cbOption[i]->user_data = this;
    lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_CLICKED, cbOption[i]);
    SetRadioButtonStyle(cbOption[i]);

    if (settingsController.GetScreenTimeOut() == options[i]) {
      lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
    }
  }
}

SettingDisplay::~SettingDisplay() {
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void SettingDisplay::UpdateSelected(lv_obj_t* object, [[maybe_unused]] lv_event_t* event) {
  for (unsigned int i = 0; i < options.size(); i++) {
    if (object == cbOption[i]) {
      lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
      settingsController.SetScreenTimeOut(options[i]);
    } else {
      lv_obj_remove_state(cbOption[i], LV_STATE_CHECKED);
    }
  }
}
