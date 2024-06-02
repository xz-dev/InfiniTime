#include "displayapp/screens/settings/QuickSettings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/ble/BleController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void Button1EventHandler(lv_event_t* event) {
    auto* screen = static_cast<QuickSettings*>(lv_event_get_user_data(event));
    screen->OnButton1Event();
  }

  void Button2EventHandler(lv_event_t* event) {
    auto* screen = static_cast<QuickSettings*>(lv_event_get_user_data(event));
    screen->OnButton2Event();
  }

  void Button3EventHandler(lv_event_t* event) {
    auto* screen = static_cast<QuickSettings*>(lv_event_get_user_data(event));
    screen->OnButton3Event();
  }

  void Button4EventHandler(lv_event_t* event) {
    auto* screen = static_cast<QuickSettings*>(lv_event_get_user_data(event));
    screen->OnButton4Event();
  }

  void lv_update_task(struct _lv_timer_t* task) {
    auto* user_data = static_cast<QuickSettings*>(task->user_data);
    user_data->UpdateScreen();
  }

  enum class ButtonState : lv_state_t {
    NotificationsOn = LV_STATE_CHECKED,
    NotificationsOff = LV_STATE_DEFAULT,
    Sleep = 0x40,
  };
}

QuickSettings::QuickSettings(Pinetime::Applications::DisplayApp* app,
                             const Pinetime::Controllers::Battery& batteryController,
                             Controllers::DateTime& dateTimeController,
                             Controllers::BrightnessController& brightness,
                             Controllers::MotorController& motorController,
                             Pinetime::Controllers::Settings& settingsController,
                             const Controllers::Ble& bleController)
  : app {app},
    dateTimeController {dateTimeController},
    brightness {brightness},
    motorController {motorController},
    settingsController {settingsController},
    statusIcons(batteryController, bleController) {

  statusIcons.Create();

  // This is the distance (padding) between all objects on this screen.
  static constexpr uint8_t innerDistance = 10;

  // Time
  label_time = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_align(label_time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(label_time, lv_screen_active(), LV_ALIGN_TOP_LEFT, 0, 0);

  static constexpr uint8_t barHeight = 20 + innerDistance;
  static constexpr uint8_t buttonHeight = (LV_VER_RES_MAX - barHeight - innerDistance) / 2;
  static constexpr uint8_t buttonWidth = (LV_HOR_RES_MAX - innerDistance) / 2; // wide buttons
  // static constexpr uint8_t buttonWidth = buttonHeight; // square buttons
  static constexpr uint8_t buttonXOffset = (LV_HOR_RES_MAX - buttonWidth * 2 - innerDistance) / 2;

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, buttonHeight / 4);
  lv_style_set_bg_color(&btn_style, Colors::bgAlt);

  btn1 = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, Button1EventHandler, LV_EVENT_CLICKED, this);
  lv_obj_add_style(btn1, &btn_style, LV_PART_MAIN);
  lv_obj_set_size(btn1, buttonWidth, buttonHeight);
  lv_obj_align_to(btn1, nullptr, LV_ALIGN_TOP_LEFT, buttonXOffset, barHeight);

  btn1_lvl = lv_label_create(btn1);
  lv_obj_set_style_text_font(btn1_lvl, &lv_font_sys_48, LV_PART_MAIN);
  lv_label_set_text_static(btn1_lvl, brightness.GetIcon());

  btn2 = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, Button2EventHandler, LV_EVENT_CLICKED, this);
  lv_obj_add_style(btn2, &btn_style, LV_PART_MAIN);
  lv_obj_set_size(btn2, buttonWidth, buttonHeight);
  lv_obj_align_to(btn2, nullptr, LV_ALIGN_TOP_RIGHT, -buttonXOffset, barHeight);

  lv_obj_t* lbl_btn;
  lbl_btn = lv_label_create(btn2);
  lv_obj_set_style_text_font(lbl_btn, &lv_font_sys_48, LV_PART_MAIN);
  lv_label_set_text_static(lbl_btn, Symbols::flashlight);

  btn3 = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btn3, Button3EventHandler, LV_EVENT_CLICKED, this);
  lv_obj_add_style(btn3, &btn_style, LV_PART_MAIN);
  lv_obj_set_style_bg_color(btn3, LV_COLOR_RED, LV_PART_MAIN | static_cast<lv_state_t>(ButtonState::NotificationsOff));
  static lv_color_t violet = LV_COLOR_MAKE(0x60, 0x00, 0xff);
  lv_obj_set_style_bg_color(btn3, violet, LV_PART_MAIN | static_cast<lv_state_t>(ButtonState::Sleep));
  lv_obj_set_size(btn3, buttonWidth, buttonHeight);
  lv_obj_align_to(btn3, nullptr, LV_ALIGN_BOTTOM_LEFT, buttonXOffset, 0);

  btn3_lvl = lv_label_create(btn3);
  lv_obj_set_style_text_font(btn3_lvl, &lv_font_sys_48, LV_PART_MAIN);

  if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::On) {
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
    lv_obj_add_state(btn3, static_cast<lv_state_t>(ButtonState::NotificationsOn));
  } else if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::Off) {
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
  } else {
    lv_label_set_text_static(btn3_lvl, Symbols::sleep);
    lv_obj_add_state(btn3, static_cast<lv_state_t>(ButtonState::Sleep));
  }

  btn4 = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btn4, Button4EventHandler, LV_EVENT_CLICKED, this);
  lv_obj_add_style(btn4, &btn_style, LV_PART_MAIN);
  lv_obj_set_size(btn4, buttonWidth, buttonHeight);
  lv_obj_align_to(btn4, nullptr, LV_ALIGN_BOTTOM_RIGHT, -buttonXOffset, 0);

  lbl_btn = lv_label_create(btn4);
  lv_obj_set_style_text_font(lbl_btn, &lv_font_sys_48, LV_PART_MAIN);
  lv_label_set_text_static(lbl_btn, Symbols::settings);

  taskUpdate = lv_timer_create(lv_update_task, 5000, this);

  UpdateScreen();
}

QuickSettings::~QuickSettings() {
  lv_style_reset(&btn_style);
  lv_timer_set_repeat_count(taskUpdate, 0);
  lv_obj_clean(lv_screen_active());
  settingsController.SaveSettings();
}

void QuickSettings::UpdateScreen() {
  lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
  statusIcons.Update();
}

void QuickSettings::OnButton1Event() {
  brightness.Step();
  lv_label_set_text_static(btn1_lvl, brightness.GetIcon());
  settingsController.SetBrightness(brightness.Level());
}

void QuickSettings::OnButton2Event() {
  app->StartApp(Apps::FlashLight, DisplayApp::FullRefreshDirections::Up);
}

void QuickSettings::OnButton3Event() {
  if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::On) {
    settingsController.SetNotificationStatus(Controllers::Settings::Notification::Off);
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
    lv_obj_add_state(btn3, static_cast<lv_state_t>(ButtonState::NotificationsOff));
  } else if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::Off) {
    settingsController.SetNotificationStatus(Controllers::Settings::Notification::Sleep);
    lv_label_set_text_static(btn3_lvl, Symbols::sleep);
    lv_obj_add_state(btn3, static_cast<lv_state_t>(ButtonState::Sleep));
  } else {
    settingsController.SetNotificationStatus(Controllers::Settings::Notification::On);
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
    lv_obj_add_state(btn3, static_cast<lv_state_t>(ButtonState::NotificationsOn));
    motorController.RunForDuration(35);
  }
}

void QuickSettings::OnButton4Event() {
  settingsController.SetSettingsMenu(0);
  app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
}
