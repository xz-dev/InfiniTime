#include "displayapp/screens/Dice.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  lv_obj_t* MakeLabel(const lv_font_t* font,
                      lv_color_t color,
                      lv_label_long_mode_t longMode,
                      uint8_t width,
                      lv_text_align_t labelAlignment,
                      const char* text,
                      lv_obj_t* reference,
                      lv_align_t alignment,
                      int8_t x,
                      int8_t y) {
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_label_set_long_mode(label, longMode);
    if (width != 0) {
      lv_obj_set_width(label, width);
    }
    lv_obj_set_style_text_align(label, labelAlignment, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_obj_align_to(label, reference, alignment, x, y);
    return label;
  }

  void btnRollEventHandler(lv_event_t* event) {
    auto* screen = static_cast<Dice*>(lv_event_get_user_data(event));
    screen->Roll();
  }
}

Dice::Dice(Controllers::MotionController& motionController,
           Controllers::MotorController& motorController,
           Controllers::Settings& settingsController)
  : motorController {motorController}, motionController {motionController}, settingsController {settingsController} {
  std::seed_seq sseq {static_cast<uint32_t>(xTaskGetTickCount()),
                      static_cast<uint32_t>(motionController.X()),
                      static_cast<uint32_t>(motionController.Y()),
                      static_cast<uint32_t>(motionController.Z())};
  gen.seed(sseq);

  lv_obj_t* nCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      LV_COLOR_WHITE,
                                      LV_LABEL_LONG_WRAP,
                                      0,
                                      LV_TEXT_ALIGN_CENTER,
                                      "count",
                                      lv_screen_active(),
                                      LV_ALIGN_TOP_LEFT,
                                      0,
                                      0);

  lv_obj_t* dCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      LV_COLOR_WHITE,
                                      LV_LABEL_LONG_WRAP,
                                      0,
                                      LV_TEXT_ALIGN_CENTER,
                                      "sides",
                                      nCounterLabel,
                                      LV_ALIGN_OUT_RIGHT_MID,
                                      20,
                                      0);

  nCounter.Create();
  lv_obj_align_to(nCounter.GetObject(), nCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  nCounter.SetValue(1);

  dCounter.Create();
  lv_obj_align_to(dCounter.GetObject(), dCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  dCounter.SetValue(6);

  std::uniform_int_distribution<> distrib(0, std::extent<decltype(resultColors)>::value - 1);
  currentColorIndex = distrib(gen);

  resultTotalLabel = MakeLabel(&jetbrains_mono_42,
                               resultColors[currentColorIndex],
                               LV_LABEL_LONG_WRAP,
                               120,
                               LV_TEXT_ALIGN_CENTER,
                               "",
                               lv_screen_active(),
                               LV_ALIGN_TOP_RIGHT,
                               11,
                               38);
  resultIndividualLabel = MakeLabel(&jetbrains_mono_bold_20,
                                    resultColors[currentColorIndex],
                                    LV_LABEL_LONG_WRAP,
                                    90,
                                    LV_TEXT_ALIGN_CENTER,
                                    "",
                                    resultTotalLabel,
                                    LV_ALIGN_OUT_BOTTOM_MID,
                                    0,
                                    10);

  Roll();
  openingRoll = false;

  btnRoll = lv_btn_create(lv_screen_active());
  btnRoll->user_data = this;
  lv_obj_add_event_cb(btnRoll, btnRollEventHandler, LV_EVENT_CLICKED, this);
  lv_obj_set_size(btnRoll, 240, 50);
  lv_obj_align_to(btnRoll, lv_screen_active(), LV_ALIGN_BOTTOM_MID, 0, 0);

  btnRollLabel = MakeLabel(&jetbrains_mono_bold_20,
                           LV_COLOR_WHITE,
                           LV_LABEL_LONG_WRAP,
                           0,
                           LV_TEXT_ALIGN_CENTER,
                           Symbols::dice,
                           btnRoll,
                           LV_ALIGN_CENTER,
                           0,
                           0);

  // Spagetti code in motion controller: it only updates the shake speed when shake to wake is on...
  enableShakeForDice = !settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake);
  if (enableShakeForDice) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, true);
  }
  refreshTask = lv_timer_create(RefreshTaskCallback, LV_DEF_REFR_PERIOD, this);
}

Dice::~Dice() {
  // reset the shake to wake mode.
  if (enableShakeForDice) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, false);
    enableShakeForDice = false;
  }
  lv_timer_set_repeat_count(refreshTask, 0);
  lv_obj_clean(lv_screen_active());
}

void Dice::Refresh() {
  // we only reset the hysteresis when at rest
  if (motionController.CurrentShakeSpeed() >= settingsController.GetShakeThreshold()) {
    if (currentRollHysteresis <= 0) {
      // this timestamp is used for the screen timeout
      //lv_display_get_next(NULL)->last_activity_time = lv_tick_get();

      Roll();
    }
  } else if (currentRollHysteresis > 0)
    --currentRollHysteresis;
}

void Dice::Roll() {
  uint8_t resultIndividual;
  uint16_t resultTotal = 0;
  std::uniform_int_distribution<> distrib(1, dCounter.GetValue());

  lv_label_set_text(resultIndividualLabel, "");

  if (nCounter.GetValue() == 1) {
    resultTotal = distrib(gen);
    if (dCounter.GetValue() == 2) {
      switch (resultTotal) {
        case 1:
          lv_label_set_text(resultIndividualLabel, "HEADS");
          break;
        case 2:
          lv_label_set_text(resultIndividualLabel, "TAILS");
          break;
      }
    }
  } else {
    for (uint8_t i = 0; i < nCounter.GetValue(); i++) {
      resultIndividual = distrib(gen);
      resultTotal += resultIndividual;
      lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, std::to_string(resultIndividual).c_str());
      if (i < (nCounter.GetValue() - 1)) {
        lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, "+");
      }
    }
  }

  lv_label_set_text_fmt(resultTotalLabel, "%d", resultTotal);
  if (openingRoll == false) {
    motorController.RunForDuration(30);
    NextColor();
    currentRollHysteresis = rollHysteresis;
  }
}

void Dice::NextColor() {
  currentColorIndex = (currentColorIndex + 1) % std::extent<decltype(resultColors)>::value;
  lv_obj_set_style_text_color(resultTotalLabel, resultColors[currentColorIndex], LV_PART_MAIN);
  lv_obj_set_style_text_color(resultIndividualLabel, resultColors[currentColorIndex], LV_PART_MAIN);
}
