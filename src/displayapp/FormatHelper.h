#pragma once

#ifndef FORMAT_HELPER_H
#define FORMAT_HELPER_H

#include <lvgl/src/misc/lv_text.h>

static inline char* get_text_fmt(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* text = _lv_text_set_text_vfmt(fmt, args);
  va_end(args);
  return text;
}

#endif /* FORMAT_HELPER_H */
