#include "displayapp/screens/Motion.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Motion::Motion(Controllers::MotionController& motionController) : motionController {motionController} {
  chart = lv_chart_create(lv_screen_active());
  lv_obj_set_size(chart, 240, 240);
  lv_obj_align_to(chart, nullptr, LV_ALIGN_TOP_MID, 0, 0);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
  // lv_chart_set_series_opa(chart, LV_OPA_70);                            /*Opacity of the data series*/
  // lv_chart_set_series_width(chart, 4);                                  /*Line width and point radious*/

  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -1100, 1100);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_point_count(chart, 10);

  /*Add 3 data series*/
  ser1 = lv_chart_add_series(chart, LV_COLOR_RED, LV_CHART_AXIS_PRIMARY_Y);
  ser2 = lv_chart_add_series(chart, Colors::green, LV_CHART_AXIS_PRIMARY_Y);
  ser3 = lv_chart_add_series(chart, LV_COLOR_YELLOW, LV_CHART_AXIS_PRIMARY_Y);

  lv_chart_set_all_value(chart, ser1, 0);
  lv_chart_set_all_value(chart, ser2, 0);
  lv_chart_set_all_value(chart, ser3, 0);
  lv_chart_refresh(chart); /*Required after direct set*/

  label = lv_spangroup_create(lv_screen_active());
  lv_span_t * span = lv_spangroup_new_span(label);
  lv_span_set_text_static(span, "X 0");
  lv_style_set_text_color(&span->style, lv_color_hex(0xFF0000));
  span = lv_spangroup_new_span(label);
  lv_span_set_text_static(span, "Y 0");
  lv_style_set_text_color(&span->style, lv_color_hex(0x00B000));
  span = lv_spangroup_new_span(label);
  lv_span_set_text_static(span, "Z 0");
  lv_style_set_text_color(&span->style, lv_color_hex(0xFFFF00));
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align_to(label, nullptr, LV_ALIGN_TOP_MID, 0, 10);

  labelStep = lv_label_create(lv_screen_active());
  lv_obj_align_to(labelStep, chart, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_label_set_text_static(labelStep, "Steps ---");

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Motion::~Motion() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void Motion::Refresh() {
  lv_chart_set_next_value(chart, ser1, motionController.X());
  lv_chart_set_next_value(chart, ser2, motionController.Y());
  lv_chart_set_next_value(chart, ser3, motionController.Z());

  lv_label_set_text_fmt(labelStep, "Steps %lu", motionController.NbSteps());

  lv_label_set_text_fmt(label,
                        "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d# mg",
                        motionController.X(),
                        motionController.Y(),
                        motionController.Z());
  lv_span_t * span = lv_spangroup_new_span(label);
  std::string result = "X " + std::to_string(motionController.X());
  lv_span_set_text(span, result.c_str());
  lv_style_set_text_color(&span->style, lv_color_hex(0xFF0000));
  span = lv_spangroup_new_span(label);
  result = "Y " + std::to_string(motionController.Y());
  lv_span_set_text(span, result.c_str());
  lv_style_set_text_color(&span->style, lv_color_hex(0x00B000));
  span = lv_spangroup_new_span(label);
  result = "Z " + std::to_string(motionController.Z());
  lv_span_set_text(span, result.c_str());
  lv_style_set_text_color(&span->style, lv_color_hex(0xFFFF00));
  lv_obj_align_to(label, nullptr, LV_ALIGN_TOP_MID, 0, 10);
}
