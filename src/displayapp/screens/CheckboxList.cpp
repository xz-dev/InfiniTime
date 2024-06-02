#include "displayapp/DisplayApp.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t* event) {
    auto* user_data = static_cast<std::pair<lv_obj_t*, CheckboxList*>*>(lv_event_get_user_data(event));
    lv_obj_t* obj = user_data->first;
    CheckboxList* screen = user_data->second;
    screen->UpdateSelected(obj, event);
  }
}

CheckboxList::CheckboxList(const uint8_t screenID,
                           const uint8_t numScreens,
                           const char* optionsTitle,
                           const char* optionsSymbol,
                           uint32_t originalValue,
                           std::function<void(uint32_t)> OnValueChanged,
                           std::array<Item, MaxItems> options)
  : screenID {screenID},
    OnValueChanged {std::move(OnValueChanged)},
    options {options},
    value {originalValue},
    pageIndicator(screenID, numScreens) {
  // Set the background to Black
  lv_obj_set_style_bg_color(lv_screen_active(), LV_COLOR_BLACK, LV_PART_MAIN);

  if (numScreens > 1) {
    pageIndicator.Create();
  }

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
  lv_label_set_text_static(title, optionsTitle);
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(title, lv_screen_active(), LV_ALIGN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(icon, LV_COLOR_ORANGE, LV_PART_MAIN);
  lv_label_set_text_static(icon, optionsSymbol);
  lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    if (strcmp(options[i].name, "")) {
      cbOption[i] = lv_checkbox_create(container1);
      lv_checkbox_set_text(cbOption[i], options[i].name);
      if (!options[i].enabled) {
        lv_obj_add_state(cbOption[i], LV_STATE_DISABLED);
      }
      cbOption[i]->user_data = this;
      auto user_data = new std::pair<lv_obj_t*, CheckboxList*>(cbOption[i], this);
      lv_obj_add_event_cb(cbOption[i], event_handler, LV_EVENT_VALUE_CHANGED, user_data);
      SetRadioButtonStyle(cbOption[i]);

      if (static_cast<unsigned int>(originalValue - MaxItems * screenID) == i) {
        lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
      }
    }
  }
}

CheckboxList::~CheckboxList() {
  lv_obj_clean(lv_screen_active());
  OnValueChanged(value);
}

void CheckboxList::UpdateSelected(lv_obj_t* object, [[maybe_unused]] lv_event_t* event) {
  for (unsigned int i = 0; i < options.size(); i++) {
    if (strcmp(options[i].name, "")) {
      if (object == cbOption[i]) {
        lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
        value = MaxItems * screenID + i;
      } else {
        lv_obj_clear_state(cbOption[i], LV_STATE_CHECKED);
      }
      if (!options[i].enabled) {
        lv_obj_add_state(cbOption[i], LV_STATE_DISABLED);
      }
    }
  }
}
