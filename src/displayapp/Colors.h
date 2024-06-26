#pragma once

#include <lvgl/src/misc/lv_color.h>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    lv_color_t Convert(Controllers::Settings::Colors color);
  }
}
