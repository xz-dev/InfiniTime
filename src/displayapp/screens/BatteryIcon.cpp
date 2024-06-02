#include "displayapp/screens/BatteryIcon.h"
#include <cstdint>
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/battery/batteryicon.c"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

BatteryIcon::BatteryIcon(bool colorOnLowBattery) : colorOnLowBattery {colorOnLowBattery} {};

void BatteryIcon::Create(lv_obj_t* parent) {
  batteryImg = lv_image_create(parent);
  lv_img_set_src(batteryImg, &batteryicon);
  static lv_style_t style;
  lv_style_set_image_recolor(&style, LV_COLOR_BLACK);

  batteryJuice = lv_obj_create(batteryImg);
  lv_obj_set_width(batteryJuice, 8);
  lv_obj_align_to(batteryJuice, nullptr, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
  lv_style_set_radius(&style, 0);
  lv_obj_add_style(batteryJuice, &style, LV_STATE_DEFAULT);
}

lv_obj_t* BatteryIcon::GetObject() {
  return batteryImg;
}

void BatteryIcon::SetBatteryPercentage(uint8_t percentage) {
  lv_obj_set_height(batteryJuice, percentage * 14 / 100);
  lv_obj_align_to(batteryJuice, nullptr, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
  if (colorOnLowBattery) {
    static constexpr int lowBatteryThreshold = 15;
    static constexpr int criticalBatteryThreshold = 5;
    if (percentage > lowBatteryThreshold) {
      SetColor(LV_COLOR_WHITE);
    } else if (percentage > criticalBatteryThreshold) {
      SetColor(lv_palette_main(LV_PALETTE_ORANGE));
    } else {
      SetColor(lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    }
  }
}

void BatteryIcon::SetColor(lv_color_t color) {
  static lv_style_t style;
  lv_style_set_image_recolor(&style, color);
  lv_style_set_image_recolor_opa(&style, LV_OPA_COVER);
  lv_style_set_bg_color(&style, color);
  lv_obj_add_style(batteryJuice, &style, LV_STATE_DEFAULT);
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging)
    return Symbols::plug;
  else
    return "";
}
