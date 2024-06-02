/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "displayapp/screens/Alarm.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/settings/Settings.h"
#include "components/alarm/AlarmController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::AlarmController;

namespace {
  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<Alarm*>(userData);
    screen->OnValueChanged();
  }
}

static void btnStopEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Alarm*>(lv_event_get_user_data(event));
  screen->OnBtnStopEvent();
}

static void btnInfoEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Alarm*>(lv_event_get_user_data(event));
  screen->OnBtnInfoEvent();
}

static void btnMessageEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Alarm*>(lv_event_get_user_data(event));
  screen->OnBtnMessageEvent();
}

static void enableSwitchEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Alarm*>(lv_event_get_user_data(event));
  screen->OnEnableSwitchEvent();
}

static void btnRecurEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Alarm*>(lv_event_get_user_data(event));
  screen->OnBtnRecurEvent();
}

static void StopAlarmTaskCallback(lv_timer_t* task) {
  auto* screen = static_cast<Alarm*>(task->user_data);
  screen->StopAlerting();
}

Alarm::Alarm(Controllers::AlarmController& alarmController,
             Controllers::Settings::ClockType clockType,
             System::SystemTask& systemTask,
             Controllers::MotorController& motorController)
  : alarmController {alarmController}, systemTask {systemTask}, motorController {motorController} {

  hourCounter.Create();
  lv_obj_align_to(hourCounter.GetObject(), nullptr, LV_ALIGN_TOP_LEFT, 0, 0);
  if (clockType == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();

    lblampm = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lblampm, &jetbrains_mono_bold_20, LV_PART_MAIN);
    lv_label_set_text_static(lblampm, "AM");
    lv_obj_set_style_text_align(lblampm, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align_to(lblampm, lv_screen_active(), LV_ALIGN_CENTER, 0, 30);
  }
  hourCounter.SetValue(alarmController.Hours());
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  lv_obj_align_to(minuteCounter.GetObject(), nullptr, LV_ALIGN_TOP_RIGHT, 0, 0);
  minuteCounter.SetValue(alarmController.Minutes());
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  lv_obj_t* colonLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(colonLabel, &jetbrains_mono_76, LV_PART_MAIN);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align_to(colonLabel, lv_screen_active(), LV_ALIGN_CENTER, 0, -29);

  btnStop = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btnStop, btnStopEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnStop, 115, 50);
  lv_obj_align_to(btnStop, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_bg_color(btnStop, LV_COLOR_RED, LV_PART_MAIN);
  txtStop = lv_label_create(btnStop);
  lv_label_set_text_static(txtStop, Symbols::stop);
  lv_obj_add_flag(btnStop, LV_OBJ_FLAG_HIDDEN);

  static lv_color_t bgColor = Colors::bgAlt;

  btnRecur = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btnRecur, btnRecurEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnRecur, 115, 50);
  lv_obj_align_to(btnRecur, lv_screen_active(), LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  txtRecur = lv_label_create(btnRecur);
  SetRecurButtonState();
  lv_obj_set_style_bg_color(btnRecur, bgColor, LV_PART_MAIN);

  btnInfo = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btnInfo, btnInfoEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnInfo, 50, 50);
  lv_obj_align_to(btnInfo, lv_screen_active(), LV_ALIGN_TOP_MID, 0, -4);
  lv_obj_set_style_bg_color(btnInfo, bgColor, LV_PART_MAIN);
  lv_obj_set_style_border_width(btnInfo, 4, LV_PART_MAIN);
  lv_obj_set_style_border_color(btnInfo, LV_COLOR_BLACK, LV_PART_MAIN);

  lv_obj_t* txtInfo = lv_label_create(btnInfo);
  lv_label_set_text_static(txtInfo, "i");

  enableSwitch = lv_switch_create(lv_screen_active());
  enableSwitch->user_data = this;
  lv_obj_add_event_cb(enableSwitch, enableSwitchEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(enableSwitch, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align_to(enableSwitch, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 7, 0);
  lv_obj_set_style_bg_color(enableSwitch, bgColor, LV_PART_INDICATOR);

  UpdateAlarmTime();

  if (alarmController.State() == Controllers::AlarmController::AlarmState::Alerting) {
    SetAlerting();
  } else {
    SetSwitchState();
  }
}

Alarm::~Alarm() {
  if (alarmController.State() == AlarmController::AlarmState::Alerting) {
    StopAlerting();
  }
  lv_obj_clean(lv_screen_active());
}

void Alarm::DisableAlarm() {
  if (alarmController.State() == AlarmController::AlarmState::Set) {
    alarmController.DisableAlarm();
    lv_obj_add_flag(enableSwitch, LV_OBJ_FLAG_EVENT_BUBBLE);
  }
}

void Alarm::OnBtnStopEvent() {
  StopAlerting();
}

void Alarm::OnBtnInfoEvent() {
  ShowInfo();
}

void Alarm::OnBtnMessageEvent() {
  HideInfo();
}

void Alarm::OnEnableSwitchEvent() {
  if (lv_obj_has_flag(enableSwitch, LV_STATE_CHECKED)) {
    alarmController.ScheduleAlarm();
  } else {
    alarmController.DisableAlarm();
  }
}

void Alarm::OnBtnRecurEvent() {
  DisableAlarm();
  ToggleRecurrence();
}

bool Alarm::OnButtonPushed() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideInfo();
    return true;
  }
  if (alarmController.State() == AlarmController::AlarmState::Alerting) {
    StopAlerting();
    return true;
  }
  return false;
}

bool Alarm::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // Don't allow closing the screen by swiping while the alarm is alerting
  return alarmController.State() == AlarmController::AlarmState::Alerting && event == TouchEvents::SwipeDown;
}

void Alarm::OnValueChanged() {
  DisableAlarm();
  UpdateAlarmTime();
}

void Alarm::UpdateAlarmTime() {
  if (lblampm != nullptr) {
    if (hourCounter.GetValue() >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
  alarmController.SetAlarmTime(hourCounter.GetValue(), minuteCounter.GetValue());
}

void Alarm::SetAlerting() {
  lv_obj_add_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(btnStop, LV_OBJ_FLAG_HIDDEN);
  taskStopAlarm = lv_timer_create(StopAlarmTaskCallback, pdMS_TO_TICKS(60 * 1000), this);
  motorController.StartRinging();
  systemTask.PushMessage(System::Messages::DisableSleeping);
}

void Alarm::StopAlerting() {
  alarmController.StopAlerting();
  motorController.StopRinging();
  SetSwitchState();
  if (taskStopAlarm != nullptr) {
    lv_timer_set_repeat_count(taskStopAlarm, 0);
    taskStopAlarm = nullptr;
  }
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_remove_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(enableSwitch, LV_OBJ_FLAG_HIDDEN);
}

void Alarm::SetSwitchState() {
  switch (alarmController.State()) {
    case AlarmController::AlarmState::Set:
      lv_obj_add_state(enableSwitch, LV_STATE_CHECKED);
      break;
    case AlarmController::AlarmState::Not_Set:
      lv_obj_remove_state(enableSwitch, LV_STATE_CHECKED);
      lv_obj_add_flag(enableSwitch, LV_OBJ_FLAG_EVENT_BUBBLE);
      break;
    default:
      break;
  }
}

void Alarm::ShowInfo() {
  if (btnMessage != nullptr) {
    return;
  }
  btnMessage = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(btnMessage, btnMessageEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_height(btnMessage, 200);
  lv_obj_set_width(btnMessage, 150);
  lv_obj_align_to(btnMessage, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage);
  lv_obj_set_style_bg_color(btnMessage, LV_COLOR_NAVY, LV_PART_MAIN);

  if (alarmController.State() == AlarmController::AlarmState::Set) {
    auto timeToAlarm = alarmController.SecondsToAlarm();

    auto daysToAlarm = timeToAlarm / 86400;
    auto hrsToAlarm = (timeToAlarm % 86400) / 3600;
    auto minToAlarm = (timeToAlarm % 3600) / 60;
    auto secToAlarm = timeToAlarm % 60;

    lv_label_set_text_fmt(txtMessage,
                          "Time to\nalarm:\n%2lu Days\n%2lu Hours\n%2lu Minutes\n%2lu Seconds",
                          daysToAlarm,
                          hrsToAlarm,
                          minToAlarm,
                          secToAlarm);
  } else {
    lv_label_set_text_static(txtMessage, "Alarm\nis not\nset.");
  }
}

void Alarm::HideInfo() {
  lv_obj_del(btnMessage);
  txtMessage = nullptr;
  btnMessage = nullptr;
}

void Alarm::SetRecurButtonState() {
  using Pinetime::Controllers::AlarmController;
  switch (alarmController.Recurrence()) {
    case AlarmController::RecurType::None:
      lv_label_set_text_static(txtRecur, "ONCE");
      break;
    case AlarmController::RecurType::Daily:
      lv_label_set_text_static(txtRecur, "DAILY");
      break;
    case AlarmController::RecurType::Weekdays:
      lv_label_set_text_static(txtRecur, "MON-FRI");
  }
}

void Alarm::ToggleRecurrence() {
  using Pinetime::Controllers::AlarmController;
  switch (alarmController.Recurrence()) {
    case AlarmController::RecurType::None:
      alarmController.SetRecurrence(AlarmController::RecurType::Daily);
      break;
    case AlarmController::RecurType::Daily:
      alarmController.SetRecurrence(AlarmController::RecurType::Weekdays);
      break;
    case AlarmController::RecurType::Weekdays:
      alarmController.SetRecurrence(AlarmController::RecurType::None);
  }
  SetRecurButtonState();
}
