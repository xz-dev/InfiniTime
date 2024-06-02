#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

DotIndicator::DotIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void DotIndicator::Create() {
  lv_obj_t* dotIndicator[nScreens];
  static constexpr uint8_t dotSize = 12;

  lv_obj_t* container = lv_obj_create(lv_screen_active());
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_base_dir(container, LV_BASE_DIR_RTL, 0);
  lv_obj_set_style_pad_gap(container, dotSize, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);

  for (int i = 0; i < nScreens; i++) {
    dotIndicator[i] = lv_obj_create(container);
    lv_obj_set_size(dotIndicator[i], dotSize, dotSize);
    lv_obj_set_style_bg_color(dotIndicator[i], LV_COLOR_GRAY, LV_PART_MAIN);
  }

  lv_obj_set_style_bg_color(dotIndicator[nCurrentScreen], LV_COLOR_WHITE, LV_PART_MAIN);

  lv_obj_align_to(container, nullptr, LV_ALIGN_RIGHT_MID, 0, 0);
}
