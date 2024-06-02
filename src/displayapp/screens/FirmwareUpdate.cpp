#include "displayapp/screens/FirmwareUpdate.h"
#include <lvgl/lvgl.h>
#include "components/ble/BleController.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

static void ClearSpangroup(lv_obj_t * obj) {
    uint32_t spanCount = lv_spangroup_get_span_count(obj);
    for (int32_t i = static_cast<int32_t>(spanCount) - 1; i >= 0; --i) {
        lv_span_t* span = lv_spangroup_get_child(obj, i);
        if (span) {
            lv_spangroup_delete_span(obj, span);
        }
    }
}

FirmwareUpdate::FirmwareUpdate(const Pinetime::Controllers::Ble& bleController) : bleController {bleController} {

  titleLabel = lv_label_create(lv_screen_active());
  lv_label_set_text_static(titleLabel, "Firmware update");
  lv_obj_align_to(titleLabel,  nullptr, LV_ALIGN_TOP_MID, 0, 50);

  bar1 = lv_bar_create(lv_screen_active());
  lv_obj_set_size(bar1, 200, 30);
  lv_obj_align(bar1, LV_ALIGN_CENTER, 0, 0);
  lv_bar_set_range(bar1, 0, 1000);
  lv_bar_set_value(bar1, 0, LV_ANIM_OFF);

  percentLabel = lv_spangroup_create(lv_screen_active());
  lv_span_t * span = lv_spangroup_new_span(percentLabel);
  lv_span_set_text_static(span, "Waiting...");
  lv_spangroup_refr_mode(percentLabel);
  //lv_obj_set_auto_realign(percentLabel, true);
  lv_obj_align_to(percentLabel, bar1, LV_ALIGN_OUT_TOP_MID, 0, 60);
  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
  startTime = xTaskGetTickCount();
}

FirmwareUpdate::~FirmwareUpdate() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  lv_obj_clean(lv_screen_active());
}

void FirmwareUpdate::Refresh() {
  switch (bleController.State()) {
    default:
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Idle:
      // This condition makes sure that the app is exited if somehow it got
      // launched without a firmware update. This should never happen.
      if (state != States::Error) {
        if (xTaskGetTickCount() - startTime > (60 * 1024)) {
          UpdateError();
          state = States::Error;
        }
      } else if (xTaskGetTickCount() - startTime > (5 * 1024)) {
        running = false;
      }
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Running:
      if (state != States::Running) {
        state = States::Running;
      }
      DisplayProgression();
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated:
      if (state != States::Validated) {
        UpdateValidated();
        state = States::Validated;
      }
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Error:
      if (state != States::Error) {
        UpdateError();
        state = States::Error;
      }
      if (xTaskGetTickCount() - startTime > (5 * 1024)) {
        running = false;
      }
      break;
  }
}

void FirmwareUpdate::DisplayProgression() const {
  const uint32_t current = bleController.FirmwareUpdateCurrentBytes();
  const uint32_t total = bleController.FirmwareUpdateTotalBytes();
  const int16_t permille = current / (total / 1000);

  ClearSpangroup(percentLabel);
  char formattedText[6];
  snprintf(formattedText, sizeof(formattedText), "%d%%", permille / 10);
  lv_span_t * span = lv_spangroup_new_span(percentLabel);
  lv_span_set_text(span, formattedText);
  lv_spangroup_refr_mode(percentLabel);

  lv_bar_set_value(bar1, permille, LV_ANIM_OFF);
}

void FirmwareUpdate::UpdateValidated() {
  ClearSpangroup(percentLabel);
  lv_span_t * span = lv_spangroup_new_span(percentLabel);
  lv_span_set_text(span, "Image Ok!");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0, 255, 0));
  lv_spangroup_refr_mode(percentLabel);
}

void FirmwareUpdate::UpdateError() {
  ClearSpangroup(percentLabel);
  lv_span_t * span = lv_spangroup_new_span(percentLabel);
  lv_span_set_text(span, "Error!");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(255, 0, 0));
  lv_spangroup_refr_mode(percentLabel);
  startTime = xTaskGetTickCount();
}

bool FirmwareUpdate::OnButtonPushed() {
  return true;
}
