#include "displayapp/screens/Steps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

static void lap_event_handler(lv_event_t* event) {
  auto* steps = static_cast<Steps*>(lv_event_get_user_data(event));
  steps->lapBtnEventHandler(event);
}

Steps::Steps(Controllers::MotionController& motionController, Controllers::Settings& settingsController)
  : motionController {motionController}, settingsController {settingsController} {

  stepsArc = lv_arc_create(lv_screen_active());

  lv_obj_set_style_bg_opa(stepsArc, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_line_color(stepsArc, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_set_style_border_width(stepsArc, 2, LV_PART_MAIN);
  lv_obj_set_style_radius(stepsArc, 0, LV_PART_MAIN);
  lv_obj_set_style_line_color(stepsArc, Colors::blue, LV_PART_INDICATOR);
  lv_arc_set_end_angle(stepsArc, 200);
  lv_obj_set_size(stepsArc, 240, 240);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align_to(stepsArc, nullptr, LV_ALIGN_CENTER, 0, 0);

  stepsCount = motionController.NbSteps();
  currentTripSteps = stepsCount - motionController.GetTripSteps();

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));

  lSteps = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(lSteps, LV_COLOR_LIME, LV_PART_MAIN);
  lv_obj_set_style_text_font(lSteps, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align_to(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t* lstepsL = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(lstepsL, Colors::lightGray, LV_PART_MAIN);
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align_to(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  lv_obj_t* lstepsGoal = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(lstepsGoal, LV_COLOR_CYAN, LV_PART_MAIN);
  lv_label_set_text_fmt(lstepsGoal, "Goal: %5lu", settingsController.GetStepsGoal());
  lv_obj_set_style_text_align(lstepsGoal, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(lstepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);

  resetBtn = lv_btn_create(lv_screen_active());
  lv_obj_add_event_cb(resetBtn, lap_event_handler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(resetBtn, 120, 50);
  lv_obj_set_style_radius(resetBtn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_bg_color(resetBtn, Colors::bgAlt, LV_PART_MAIN);
  lv_obj_align_to(resetBtn, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);
  resetButtonLabel = lv_label_create(resetBtn);
  lv_label_set_text_static(resetButtonLabel, "Reset");

  currentTripSteps = motionController.GetTripSteps();

  tripLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(tripLabel, LV_COLOR_YELLOW, LV_PART_MAIN);
  lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  lv_obj_align_to(tripLabel, lstepsGoal, LV_ALIGN_LEFT_MID, 0, 20);

  taskRefresh = lv_timer_create(RefreshTaskCallback, 100, this);
}

Steps::~Steps() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void Steps::Refresh() {
  stepsCount = motionController.NbSteps();
  currentTripSteps = motionController.GetTripSteps();

  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align_to(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  if (currentTripSteps < 100000) {
    lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  } else {
    lv_label_set_text_fmt(tripLabel, "Trip: 99999+");
  }
  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));
}

void Steps::lapBtnEventHandler([[maybe_unused]] lv_event_t* event) {
  stepsCount = motionController.NbSteps();
  motionController.ResetTrip();
  Refresh();
}
