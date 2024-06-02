#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(uint32_t key) {
  passkeyLabel = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(passkeyLabel, LV_COLOR_YELLOW, LV_PART_MAIN);
  lv_obj_set_style_text_font(passkeyLabel, &jetbrains_mono_42, LV_PART_MAIN);
  lv_label_set_text_fmt(passkeyLabel, "%06lu", key);
  lv_obj_align_to(passkeyLabel, nullptr, LV_ALIGN_CENTER, 0, -20);
}

PassKey::~PassKey() {
  lv_obj_clean(lv_screen_active());
}
