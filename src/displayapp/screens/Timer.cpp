#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_event_t* event) {
  auto* screen = static_cast<Timer*>(lv_event_get_user_data(event));
  lv_event_code_t code = lv_event_get_code(event);
  switch (code) {
    case LV_EVENT_PRESSED:
      screen->ButtonPressed();
      break;
    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
      screen->MaskReset();
      break;
    case LV_EVENT_SHORT_CLICKED:
      screen->ToggleRunning();
      break;
    default:
      break;
  }
}

Timer::Timer(Controllers::Timer& timerController) : timer {timerController} {

  lv_obj_t* colonLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(colonLabel, &jetbrains_mono_76, LV_PART_MAIN);
  lv_obj_set_style_text_color(colonLabel, LV_COLOR_WHITE, LV_PART_MAIN);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align_to(colonLabel, lv_screen_active(), LV_ALIGN_CENTER, 0, -29);

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align_to(minuteCounter.GetObject(), nullptr, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_align_to(secondCounter.GetObject(), nullptr, LV_ALIGN_TOP_RIGHT, 0, 0);

  // lv_obj_set_size(highlightObjectMask, 240, 50);
  LV_DRAW_BUF_DEFINE(highlightObjectMask, 240, 50, LV_COLOR_FORMAT_ARGB8888);
  lv_obj_t* canvas = lv_canvas_create(lv_screen_active());
  lv_canvas_set_draw_buf(canvas, &highlightObjectMask);
  lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
  lv_obj_align_to(canvas, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);

  btnPlayPause = lv_btn_create(btnObjectMask);
  btnPlayPause->user_data = this;
  lv_obj_set_style_radius(btnPlayPause, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_bg_color(btnPlayPause, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(btnPlayPause, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_add_event_cb(btnPlayPause, btnEventHandler, LV_EVENT_ALL, this);
  lv_obj_set_size(btnPlayPause, LV_HOR_RES, 50);

  btnMask = lv_bar_create(btnPlayPause);
  lv_obj_center(btnMask);
  lv_obj_set_size(btnMask, LV_HOR_RES, 50);
  lv_obj_set_style_radius(btnMask, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_bar_set_range(btnMask, 0, 240);
  lv_obj_set_style_bg_color(btnMask, LV_COLOR_ORANGE, LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_opa(btnMask, LV_OPA_TRANSP, LV_PART_INDICATOR);

  txtPlayPause = lv_label_create(lv_screen_active());
  lv_obj_align_to(txtPlayPause, btnPlayPause, LV_ALIGN_CENTER, 0, 0);

  if (timer.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Timer::~Timer() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  if (!timer.IsRunning()) {
    lv_label_set_text_static(txtPlayPause, "Start");
  }
  maskPosition = 0;
  UpdateMask();
}

void Timer::UpdateMask() {
  lv_bar_set_value(btnMask, maskPosition, LV_ANIM_ON);
}

void Timer::Refresh() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    minuteCounter.SetValue(secondsRemaining.count() / 60);
    secondCounter.SetValue(secondsRemaining.count() % 60);
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    } else {
      UpdateMask();
    }
  }
}

void Timer::SetTimerRunning() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
}

void Timer::SetTimerStopped() {
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
}

void Timer::ToggleRunning() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    minuteCounter.SetValue(secondsRemaining.count() / 60);
    secondCounter.SetValue(secondsRemaining.count() % 60);
    timer.StopTimer();
    SetTimerStopped();
  } else if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
    auto timerDuration = std::chrono::minutes(minuteCounter.GetValue()) + std::chrono::seconds(secondCounter.GetValue());
    timer.StartTimer(timerDuration);
    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  minuteCounter.SetValue(0);
  secondCounter.SetValue(0);
  SetTimerStopped();
}
