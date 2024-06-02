#include "displayapp/InfiniTimeTheme.h"
#include <algorithm>

// Replace LV_DPX with a constexpr version using a constant LV_DPI
#undef LV_DPX

namespace {
  constexpr int LV_DPX(int n) {
    if (n == 0) {
      return 0;
    }
    return std::max(((LV_DPI_DEF * n + 80) / 160), 1); /*+80 for rounding*/
  }
}

static lv_theme_t theme;

static lv_style_t style_bg;
static lv_style_t style_box;
static lv_style_t style_btn;
static lv_style_t style_btn_checked;
static lv_style_t style_btn_disabled;
static lv_style_t style_label_white;
static lv_style_t style_label_white_disabled;
static lv_style_t style_icon;
static lv_style_t style_bar_indic;
static lv_style_t style_slider_knob;
static lv_style_t style_slider_knob_passed;
static lv_style_t style_scrollbar;
static lv_style_t style_list_btn;
static lv_style_t style_list_btn_checked;
static lv_style_t style_ddlist_list;
static lv_style_t style_ddlist_selected;
static lv_style_t style_sw_bg;
static lv_style_t style_sw_indic;
static lv_style_t style_sw_knob;
static lv_style_t style_sw_knob_checked;
static lv_style_t style_arc_bg;
static lv_style_t style_arc_knob;
static lv_style_t style_arc_indic;
static lv_style_t style_table_cell;
static lv_style_t style_pad_small;
static lv_style_t style_lmeter;
static lv_style_t style_chart_serie;
static lv_style_t style_cb_bg;
static lv_style_t style_cb_bullet;
static lv_style_t style_cb_bullet_checked;

static bool inited;

static void style_init_reset(lv_style_t* style) {
  if (inited) {
    lv_style_reset(style);
  } else {
    lv_style_init(style);
  }
}

static void basic_init() {
  style_init_reset(&style_bg);
  lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
  lv_style_set_bg_color(&style_bg, LV_COLOR_BLACK);
  lv_style_set_text_font(&style_bg, theme.font_normal);

  style_init_reset(&style_box);
  lv_style_set_bg_opa(&style_box, LV_OPA_COVER);
  lv_style_set_radius(&style_box, 10);
  lv_style_set_text_font(&style_box, theme.font_normal);

  style_init_reset(&style_label_white);
  lv_style_set_text_color(&style_label_white, LV_COLOR_WHITE);
  style_init_reset(&style_label_white_disabled);
  lv_style_set_text_color(&style_label_white_disabled, LV_COLOR_GRAY);

  style_init_reset(&style_btn);
  lv_style_set_radius(&style_btn, 10);
  lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, Colors::bg);
  style_init_reset(&style_btn_checked);
  lv_style_set_bg_color(&style_btn_checked, Colors::highlight);
  style_init_reset(&style_btn_disabled);
  lv_style_set_bg_color(&style_btn_disabled, Colors::bgDark);

  lv_style_set_text_color(&style_btn, LV_COLOR_WHITE);
  lv_style_set_text_color(&style_btn_disabled, LV_COLOR_GRAY);

  lv_style_set_pad_all(&style_btn, LV_DPX(20));
  lv_style_set_pad_gap(&style_btn, LV_DPX(15));

  style_init_reset(&style_icon);
  lv_style_set_text_color(&style_icon, LV_COLOR_WHITE);

  style_init_reset(&style_bar_indic);
  lv_style_set_bg_opa(&style_bar_indic, LV_OPA_COVER);
  lv_style_set_radius(&style_bar_indic, 10);

  style_init_reset(&style_scrollbar);
  lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
  lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
  lv_style_set_bg_color(&style_scrollbar, LV_COLOR_WHITE);
  lv_style_set_height(&style_scrollbar, LV_HOR_RES / 80);
  lv_style_set_pad_right(&style_scrollbar, LV_HOR_RES / 60);

  style_init_reset(&style_list_btn);
  lv_style_set_bg_opa(&style_list_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_list_btn, LV_COLOR_WHITE);
  lv_style_set_text_color(&style_list_btn, Colors::bg);
  style_init_reset(&style_list_btn_checked);
  lv_style_set_text_color(&style_list_btn_checked, LV_COLOR_WHITE);
  lv_style_set_image_recolor(&style_list_btn, Colors::bg);
  lv_style_set_image_recolor(&style_list_btn_checked, LV_COLOR_WHITE);
  lv_style_set_pad_left(&style_list_btn, LV_HOR_RES / 25);
  lv_style_set_pad_right(&style_list_btn, LV_HOR_RES / 25);
  lv_style_set_pad_top(&style_list_btn, LV_HOR_RES / 100);
  lv_style_set_pad_bottom(&style_list_btn, LV_HOR_RES / 100);
  lv_style_set_pad_gap(&style_list_btn, LV_HOR_RES / 50);

  style_init_reset(&style_ddlist_list);
  // Causes lag unfortunately, so we'll have to live with the selected item overflowing the corner
  // lv_style_set_clip_corner(&style_ddlist_list, true);
  lv_style_set_text_line_space(&style_ddlist_list, LV_VER_RES / 25);
  lv_style_set_bg_color(&style_ddlist_list, Colors::lightGray);
  lv_style_set_pad_all(&style_ddlist_list, 20);

  style_init_reset(&style_ddlist_selected);
  lv_style_set_bg_opa(&style_ddlist_selected, LV_OPA_COVER);
  lv_style_set_bg_color(&style_ddlist_selected, Colors::bg);

  style_init_reset(&style_sw_bg);
  lv_style_set_bg_opa(&style_sw_bg, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_bg, Colors::bg);
  lv_style_set_radius(&style_sw_bg, LV_RADIUS_CIRCLE);

  style_init_reset(&style_sw_indic);
  lv_style_set_bg_opa(&style_sw_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_indic, Colors::highlight);

  style_init_reset(&style_sw_knob);
  lv_style_set_bg_opa(&style_sw_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_knob, LV_COLOR_SILVER);
  style_init_reset(&style_sw_knob_checked);
  lv_style_set_bg_color(&style_sw_knob_checked, LV_COLOR_WHITE);
  lv_style_set_radius(&style_sw_knob, LV_RADIUS_CIRCLE);
  lv_style_set_pad_all(&style_sw_knob, -4);

  style_init_reset(&style_slider_knob);
  lv_style_set_bg_opa(&style_slider_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_slider_knob, LV_COLOR_RED);
  lv_style_set_border_color(&style_slider_knob, LV_COLOR_WHITE);
  lv_style_set_border_width(&style_slider_knob, 6);
  lv_style_set_radius(&style_slider_knob, LV_RADIUS_CIRCLE);
  lv_style_set_pad_all(&style_slider_knob, 10);
  style_init_reset(&style_slider_knob_passed);
  lv_style_set_pad_all(&style_slider_knob_passed, 14);

  style_init_reset(&style_arc_indic);
  lv_style_set_line_color(&style_arc_indic, Colors::lightGray);
  lv_style_set_line_width(&style_arc_indic, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_indic, true);

  style_init_reset(&style_arc_bg);
  lv_style_set_line_color(&style_arc_bg, Colors::bg);
  lv_style_set_line_width(&style_arc_bg, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_bg, true);
  lv_style_set_pad_all(&style_arc_bg, LV_DPX(5));

  lv_style_reset(&style_arc_knob);
  lv_style_set_radius(&style_arc_knob, LV_RADIUS_CIRCLE);
  lv_style_set_bg_opa(&style_arc_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_arc_knob, LV_COLOR_WHITE);
  lv_style_set_pad_all(&style_arc_knob, LV_DPX(5));

  style_init_reset(&style_table_cell);
  lv_style_set_border_color(&style_table_cell, LV_COLOR_GRAY);
  lv_style_set_border_width(&style_table_cell, 1);
  lv_style_set_border_side(&style_table_cell, LV_BORDER_SIDE_FULL);
  lv_style_set_pad_left(&style_table_cell, 5);
  lv_style_set_pad_right(&style_table_cell, 5);
  lv_style_set_pad_top(&style_table_cell, 2);
  lv_style_set_pad_bottom(&style_table_cell, 2);

  style_init_reset(&style_pad_small);
  int32_t pad_small_value = 10;
  lv_style_set_pad_all(&style_pad_small, pad_small_value);
  lv_style_set_pad_gap(&style_pad_small, pad_small_value);

  style_init_reset(&style_lmeter);
  lv_style_set_radius(&style_lmeter, LV_RADIUS_CIRCLE);
  lv_style_set_pad_left(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_right(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_top(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_gap(&style_lmeter, LV_DPX(30));
  lv_style_set_line_width(&style_lmeter, LV_DPX(25));

  lv_style_set_line_color(&style_lmeter, LV_COLOR_WHITE);
  lv_style_set_line_width(&style_lmeter, LV_DPX(10));

  style_init_reset(&style_chart_serie);
  lv_style_set_line_color(&style_chart_serie, LV_COLOR_WHITE);
  lv_style_set_line_width(&style_chart_serie, 4);
  lv_style_set_bg_opa(&style_chart_serie, 0);

  lv_style_reset(&style_cb_bg);
  lv_style_set_radius(&style_cb_bg, LV_DPX(4));
  lv_style_set_pad_gap(&style_cb_bg, 18);

  lv_style_reset(&style_cb_bullet);
  lv_style_set_radius(&style_cb_bullet, LV_DPX(4));
  lv_style_set_arc_image_src(&style_cb_bullet, LV_SYMBOL_OK);
  style_init_reset(&style_cb_bullet_checked);
  lv_style_set_img_recolor(&style_cb_bullet_checked, LV_COLOR_WHITE);
  lv_style_set_pad_all(&style_cb_bullet, LV_DPX(8));
}

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init() {
  theme.color_primary = LV_COLOR_WHITE;
  theme.color_secondary = LV_COLOR_GRAY;
  theme.font_small = &jetbrains_mono_bold_20;
  theme.font_normal = &jetbrains_mono_bold_20;
  theme.flags = 0;

  basic_init();

  inited = true;

  return &theme;
}

