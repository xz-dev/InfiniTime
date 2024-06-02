#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

PageIndicator::PageIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void PageIndicator::Create() {
  pageIndicatorBasePoints[0] = {LV_HOR_RES - 1, 0};
  pageIndicatorBasePoints[1] = {LV_HOR_RES - 1, LV_VER_RES};

  pageIndicatorBase = lv_line_create(lv_screen_active());
  lv_obj_set_style_line_width(pageIndicatorBase, 3, LV_PART_MAIN);
  lv_obj_set_style_line_color(pageIndicatorBase, Colors::bgDark, LV_PART_MAIN);
  lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

  const int16_t indicatorSize = LV_VER_RES / nScreens;
  const int16_t indicatorPos = indicatorSize * nCurrentScreen;

  pageIndicatorPoints[0] = {LV_HOR_RES - 1, indicatorPos};
  pageIndicatorPoints[1] = {LV_HOR_RES - 1, indicatorPos + indicatorSize};

  pageIndicator = lv_line_create(lv_screen_active());
  lv_obj_set_style_line_width(pageIndicator, 3, LV_PART_MAIN);
  lv_obj_set_style_line_color(pageIndicator, Colors::lightGray, LV_PART_MAIN);
  lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
}
