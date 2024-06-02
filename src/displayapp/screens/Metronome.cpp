#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventBpmArcHandler(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(lv_event_get_user_data(event));
    screen->OnBpmArcEvent(event);
  }

  void eventBpmDropdownHandler(lv_event_t* event) {
    auto* obj = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    auto* screen = static_cast<Metronome*>(lv_obj_get_user_data(obj));
    screen->OnBpmDropdownEvent(obj ,event);
  }

  void eventBpmTapHandler(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(lv_event_get_user_data(event));
    screen->OnBpmTapEvent(event);
  }

  void eventPlayPauseHandler(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(lv_event_get_user_data(event));
    screen->OnPlayPauseEvent(event);
  }

  lv_obj_t* createLabel(const char* name, lv_obj_t* reference, lv_align_t align, const lv_font_t* font, uint8_t x, uint8_t y) {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, Colors::lightGray, LV_PART_MAIN);
    lv_label_set_text(label, name);
    lv_obj_align_to(label, reference, align, x, y);

    return label;
  }
}

Metronome::Metronome(Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : motorController {motorController}, systemTask {systemTask} {

  bpmArc = lv_arc_create(lv_screen_active());
  lv_obj_add_event_cb(bpmArc, eventBpmArcHandler, LV_EVENT_VALUE_CHANGED, this);
  lv_arc_set_bg_angles(bpmArc, 0, 270);
  lv_arc_set_rotation(bpmArc, 135);
  lv_arc_set_range(bpmArc, 40, 220);
  lv_arc_set_value(bpmArc, bpm);
  lv_obj_set_size(bpmArc, 210, 210);
  lv_obj_align_to(bpmArc, lv_screen_active(), LV_ALIGN_TOP_MID, 0, 0);

  bpmValue = createLabel("120", bpmArc, LV_ALIGN_TOP_MID, &jetbrains_mono_76, 0, 55);
  createLabel("bpm", bpmValue, LV_ALIGN_OUT_BOTTOM_MID, &jetbrains_mono_bold_20, 0, 0);

  bpmTap = lv_btn_create(lv_screen_active());
  bpmTap->user_data = this;
  lv_obj_add_event_cb(bpmTap, eventBpmTapHandler, LV_EVENT_ALL, this);
  lv_obj_set_style_bg_opa(bpmTap, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_height(bpmTap, 80);
  lv_obj_align_to(bpmTap, bpmValue, LV_ALIGN_TOP_MID, 0, 0);

  bpbDropdown = lv_dropdown_create(lv_screen_active());
  bpbDropdown->user_data = this;
  lv_obj_add_event_cb(bpbDropdown, eventBpmDropdownHandler, LV_EVENT_VALUE_CHANGED, bpbDropdown);
  lv_obj_set_size(bpbDropdown, 115, 50);
  lv_obj_align_to(bpbDropdown, lv_screen_active(), LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_dropdown_set_options(bpbDropdown, "1\n2\n3\n4\n5\n6\n7\n8\n9");
  lv_dropdown_set_selected(bpbDropdown, bpb - 1);
  lv_dropdown_set_text(bpbDropdown, nullptr);

  currentBpbText = lv_label_create(bpbDropdown);
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
  lv_obj_align_to(currentBpbText, bpbDropdown, LV_ALIGN_CENTER, 0, 0);

  playPause = lv_btn_create(lv_screen_active());
  playPause->user_data = this;
  lv_obj_add_event_cb(playPause, eventPlayPauseHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(playPause, 115, 50);
  lv_obj_align_to(playPause, lv_screen_active(), LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lblPlayPause = lv_label_create(playPause);
  lv_label_set_text_static(lblPlayPause, Symbols::play);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Metronome::~Metronome() {
  lv_timer_set_repeat_count(taskRefresh, 0);
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_clean(lv_screen_active());
}

void Metronome::Refresh() {
  if (metronomeStarted) {
    if (xTaskGetTickCount() - startTime > 60u * configTICK_RATE_HZ / static_cast<uint16_t>(bpm)) {
      startTime += 60 * configTICK_RATE_HZ / bpm;
      counter--;
      if (counter == 0) {
        counter = bpb;
        motorController.RunForDuration(90);
      } else {
        motorController.RunForDuration(30);
      }
    }
  }
}

void Metronome::OnBpmArcEvent([[maybe_unused]] lv_event_t* event) {
  bpm = lv_arc_get_value(bpmArc);
  lv_label_set_text_fmt(bpmValue, "%03d", bpm);
}

void Metronome::OnBpmDropdownEvent(lv_obj_t * obj, [[maybe_unused]] lv_event_t* event) {
  bpb = lv_dropdown_get_selected(obj) + 1;
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
  lv_obj_align_to(currentBpbText, bpbDropdown, LV_ALIGN_CENTER, 0, 0);
}

void Metronome::OnBpmTapEvent(lv_event_t* event) {
  lv_event_code_t code = lv_event_get_code(event);
  switch (code) {
    case LV_EVENT_PRESSED: {
      TickType_t delta = xTaskGetTickCount() - tappedTime;
      if (tappedTime != 0 && delta < configTICK_RATE_HZ * 3) {
        bpm = configTICK_RATE_HZ * 60 / delta;
        lv_arc_set_value(bpmArc, bpm);
        lv_label_set_text_fmt(bpmValue, "%03d", bpm);
      }
      tappedTime = xTaskGetTickCount();
      allowExit = true;
      break;
    }
    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
      allowExit = false;
      break;
    default:
      break;
  }
}

void Metronome::OnPlayPauseEvent([[maybe_unused]] lv_event_t* event) {
  metronomeStarted = !metronomeStarted;
  if (metronomeStarted) {
    lv_label_set_text_static(lblPlayPause, Symbols::pause);
    systemTask.PushMessage(System::Messages::DisableSleeping);
    startTime = xTaskGetTickCount();
    counter = 1;
  } else {
    lv_label_set_text_static(lblPlayPause, Symbols::play);
    systemTask.PushMessage(System::Messages::EnableSleeping);
  }
}

bool Metronome::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeDown && allowExit) {
    running = false;
  }
  return true;
}
