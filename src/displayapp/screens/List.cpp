#include "displayapp/screens/List.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_event_t* event) {
    auto* user_data = static_cast<std::pair<lv_obj_t*, List*>*>(lv_event_get_user_data(event));
    lv_obj_t* obj = user_data->first;
    List* screen = user_data->second;
    screen->OnButtonEvent(obj, event);
  }
}

List::List(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           std::array<Applications, MAXLISTITEMS>& applications)
  : app {app}, settingsController {settingsController}, pageIndicator(screenID, numScreens) {

  // Set the background to Black
  lv_obj_set_style_bg_color(lv_screen_active(), LV_COLOR_BLACK, 0);

  settingsController.SetSettingsMenu(screenID);

  pageIndicator.Create();

  lv_obj_t* container = lv_obj_create(lv_screen_active());

  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  static constexpr int innerPad = 4;
  lv_obj_set_style_pad_gap(container, innerPad, LV_PART_MAIN);
  lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);

  lv_obj_set_pos(container, 0, 0);
  lv_obj_set_width(container, LV_HOR_RES - 8);
  lv_obj_set_height(container, LV_VER_RES);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    if (applications[i].application != Apps::None) {

      static constexpr int btnHeight = (LV_HOR_RES_MAX - ((MAXLISTITEMS - 1) * innerPad)) / MAXLISTITEMS;
      itemApps[i] = lv_btn_create(container);
      lv_obj_set_style_radius(itemApps[i], btnHeight / 3, LV_PART_MAIN);
      lv_obj_set_style_bg_color(itemApps[i], Colors::bgAlt, LV_PART_MAIN);
      lv_obj_set_width(itemApps[i], LV_HOR_RES - 8);
      lv_obj_set_height(itemApps[i], btnHeight);
      auto handler_pair = new std::pair<lv_obj_t*, List*>(itemApps[i], this);
      lv_obj_add_event_cb(itemApps[i], ButtonEventHandler, LV_EVENT_CLICKED, handler_pair);
      // lv_btn_set_layout(itemApps[i], LV_LAYOUT_OFF);
      itemApps[i]->user_data = this;

      lv_obj_t* icon = lv_label_create(itemApps[i]);
      lv_obj_set_style_text_color(icon, LV_COLOR_YELLOW, LV_PART_MAIN);
      lv_label_set_text_static(icon, applications[i].icon);
      lv_label_set_long_mode(icon, LV_LABEL_LONG_CLIP);
      lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
      lv_obj_set_width(icon, btnHeight);
      lv_obj_align_to(icon, nullptr, LV_ALIGN_LEFT_MID, 0, 0);

      lv_obj_t* text = lv_label_create(itemApps[i]);
      lv_label_set_text_fmt(text, "%s", applications[i].name);
      lv_obj_align_to(text, icon, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }
  }
}

List::~List() {
  lv_obj_clean(lv_screen_active());
}

void List::OnButtonEvent(lv_obj_t* object, [[maybe_unused]] lv_event_t* event) {
  for (int i = 0; i < MAXLISTITEMS; i++) {
    if (apps[i] != Apps::None && object == itemApps[i]) {
      app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
      running = false;
      return;
    }
  }
}
