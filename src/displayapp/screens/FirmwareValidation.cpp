#include "displayapp/screens/FirmwareValidation.h"
#include <lvgl/lvgl.h>
#include "Version.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonClickHandlerValidate(lv_event_t * event) {
    FirmwareValidation * obj = static_cast<FirmwareValidation*>(lv_event_get_user_data(event));
    obj->OnValidateButtonEvent(event);
  }

  void ButtonClickHandlerReset(lv_event_t * event) {
    FirmwareValidation * obj = static_cast<FirmwareValidation*>(lv_event_get_user_data(event));
    obj->OnResetButtonEvent(event);
  }
}

FirmwareValidation::FirmwareValidation(Pinetime::Controllers::FirmwareValidator& validator) : validator {validator} {
  labelVersion = lv_label_create(lv_screen_active());
  lv_label_set_text_fmt(labelVersion,
                        "Version : %lu.%lu.%lu\n"
                        "ShortRef : %s",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash());
  lv_obj_align_to(labelVersion, nullptr, LV_ALIGN_TOP_LEFT, 0, 0);

  labelIsValidated = lv_spangroup_create(lv_screen_active());
  lv_obj_align_to(labelIsValidated, labelVersion, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_obj_set_width(labelIsValidated, 240);

  if (validator.IsValidated()) {
    lv_span_t* span = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span, "You have already\n");
    lv_span_t* span2 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span2, "validated");
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x00, 0xff, 0x00));
    lv_span_t* span3 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span3, " this firmware");
    lv_spangroup_set_mode(labelIsValidated, LV_SPAN_MODE_BREAK);
    lv_spangroup_refr_mode(labelIsValidated);
  } else {
    lv_span_t* span = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span, "Please ");
    lv_span_t* span2 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span2, "Validate");
    lv_style_set_text_color(&span2->style, LV_COLOR_MAKE(0x00, 0xff, 0x00));
    lv_span_t* span3 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span3, " this version or\n");
    lv_span_t* span4 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span4, "Reset");
    lv_style_set_text_color(&span4->style, LV_COLOR_MAKE(0xff, 0x00, 0x00));
    lv_span_t* span5 = lv_spangroup_new_span(labelIsValidated);
    lv_span_set_text_static(span5, " to rollback to the previous version.");
    lv_spangroup_set_mode(labelIsValidated, LV_SPAN_MODE_BREAK);
    lv_spangroup_refr_mode(labelIsValidated);

    buttonValidate = lv_btn_create(lv_screen_active());
    buttonValidate->user_data = this;
    lv_obj_set_size(buttonValidate, 115, 50);
    lv_obj_align_to(buttonValidate, nullptr, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(buttonValidate, Colors::highlight, 0);
    lv_obj_add_event_cb(buttonValidate, ButtonClickHandlerValidate, LV_EVENT_CLICKED, this);

    labelButtonValidate = lv_label_create(buttonValidate);
    lv_label_set_text_static(labelButtonValidate, "Validate");

    buttonReset = lv_btn_create(lv_screen_active());
    buttonReset->user_data = this;
    lv_obj_set_size(buttonReset, 115, 50);
    lv_obj_align_to(buttonReset, nullptr, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(buttonReset, LV_COLOR_RED, 0);
    lv_obj_add_event_cb(buttonReset, ButtonClickHandlerReset, LV_EVENT_CLICKED, this);

    labelButtonReset = lv_label_create(buttonReset);
    lv_label_set_text_static(labelButtonReset, "Reset");
  }
}

FirmwareValidation::~FirmwareValidation() {
  lv_obj_clean(lv_screen_active());
}

void FirmwareValidation::OnValidateButtonEvent([[maybe_unused]] lv_event_t* event) {
  validator.Validate();
  running = false;
}

void FirmwareValidation::OnResetButtonEvent([[maybe_unused]] lv_event_t* event) {
  validator.Reset();
}
