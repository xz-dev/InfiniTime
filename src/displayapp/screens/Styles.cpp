#include "Styles.h"
#include "displayapp/InfiniTimeTheme.h"

void Pinetime::Applications::Screens::SetRadioButtonStyle(lv_obj_t* checkbox) {
  lv_obj_set_style_radius(checkbox, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
  lv_obj_set_style_border_width(checkbox, 9, LV_PART_INDICATOR | LV_PART_SELECTED);
  lv_obj_set_style_border_color(checkbox, Colors::highlight, LV_PART_INDICATOR | LV_PART_SELECTED);
  lv_obj_set_style_bg_color(checkbox, LV_COLOR_WHITE, LV_PART_INDICATOR | LV_PART_SELECTED);
}
