#include "displayapp/LittleVgl.h"
#include "displayapp/InfiniTimeTheme.h"

#include <FreeRTOS.h>
#include <task.h>
#include "drivers/St7789.h"
#include "littlefs/lfs.h"
#include "components/fs/FS.h"

using namespace Pinetime::Components;

namespace {
  void* lvglOpen(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t /*mode*/) {
    lfs_file_t* file = new lfs_file_t();
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    int res = filesys->FileOpen(file, path, LFS_O_RDONLY);
    if (res == 0) {
      if (file->type == 0) {
        return nullptr;
      } else {
        return file;
      }
    }
    return nullptr;
  }

  lv_fs_res_t lvglClose(lv_fs_drv_t* drv, void* file_p) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileClose(file);

    return LV_FS_RES_OK;
  }

  lv_fs_res_t lvglRead(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileRead(file, static_cast<uint8_t*>(buf), btr);
    *br = btr;
    return LV_FS_RES_OK;
  }

  lv_fs_res_t lvglSeek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t /*whence*/) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileSeek(file, pos);
    return LV_FS_RES_OK;
  }
}

static void disp_flush(lv_display_t* disp_drv, const lv_area_t* area, uint8_t* px_map) {
  auto* lvgl = static_cast<LittleVgl*>(lv_display_get_user_data(disp_drv));
  lvgl->FlushDisplay(area, px_map);
}

void touchpad_read(lv_indev_t* indev_drv, lv_indev_data_t* data) {
  auto* lvgl = static_cast<LittleVgl*>(lv_indev_get_user_data(indev_drv));
  lvgl->GetTouchPadInfo(data);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Controllers::FS& filesystem) : lcd {lcd}, filesystem {filesystem} {
}

void LittleVgl::Init() {
  lv_init();
  InitDisplay();
  InitTheme();
  InitTouchpad();
  InitFileSystem();
}

void LittleVgl::InitDisplay() {
  /*Set the resolution of the display*/
  disp_buf_2 = lv_display_create(LV_HOR_RES_MAX, LV_VER_RES_MAX);

  /*Set up the functions to access to your display*/

  /*Used to copy the buffer's content to the display*/
  lv_display_set_flush_cb(disp_buf_2, disp_flush);
  /*Set a display buffer*/
  lv_display_set_buffers(disp_buf_2, buf2_1, buf2_2, sizeof(buf2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_user_data(disp_buf_2, this);
}

void LittleVgl::InitTheme() {
  lv_theme_t* theme = lv_pinetime_theme_init();
  lv_display_set_theme(disp_buf_2, theme);
}

void LittleVgl::InitTouchpad() {
  lv_indev_t* indev_drv = lv_indev_create();

  lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev_drv, touchpad_read);
  lv_indev_set_user_data(indev_drv, this);
  lv_indev_read(indev_drv);
}

void LittleVgl::InitFileSystem() {
  lv_fs_drv_t fs_drv;
  lv_fs_drv_init(&fs_drv);

  fs_drv.letter = 'F';
  fs_drv.open_cb = lvglOpen;
  fs_drv.close_cb = lvglClose;
  fs_drv.read_cb = lvglRead;
  fs_drv.seek_cb = lvglSeek;

  fs_drv.user_data = &filesystem;

  lv_fs_drv_register(&fs_drv);
}

void LittleVgl::SetFullRefresh(FullRefreshDirections direction) {
  if (scrollDirection == FullRefreshDirections::None) {
    scrollDirection = direction;
    if (scrollDirection == FullRefreshDirections::Down) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_180);
    } else if (scrollDirection == FullRefreshDirections::Right) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_90);
    } else if (scrollDirection == FullRefreshDirections::Left) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_270);
    } else if (scrollDirection == FullRefreshDirections::RightAnim) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_90);
    } else if (scrollDirection == FullRefreshDirections::LeftAnim) {
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_270);
    }
  }
  fullRefresh = true;
}

void LittleVgl::FlushDisplay(const lv_area_t* area, uint8_t* px_map) {
  uint16_t y1, y2, width, height = 0;

  if ((scrollDirection == LittleVgl::FullRefreshDirections::Down) && (area->y2 == visibleNbLines - 1)) {
    writeOffset = ((writeOffset + totalNbLines) - visibleNbLines) % totalNbLines;
  } else if ((scrollDirection == FullRefreshDirections::Up) && (area->y1 == 0)) {
    writeOffset = (writeOffset + visibleNbLines) % totalNbLines;
  }

  y1 = (area->y1 + writeOffset) % totalNbLines;
  y2 = (area->y2 + writeOffset) % totalNbLines;

  width = (area->x2 - area->x1) + 1;
  height = (area->y2 - area->y1) + 1;

  if (scrollDirection == LittleVgl::FullRefreshDirections::Down) {

    if (area->y2 < visibleNbLines - 1) {
      uint16_t toScroll = 0;
      if (area->y1 == 0) {
        toScroll = height * 2;
        scrollDirection = FullRefreshDirections::None;
        lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_0);
      } else {
        toScroll = height;
      }

      if (scrollOffset >= toScroll)
        scrollOffset -= toScroll;
      else {
        toScroll -= scrollOffset;
        scrollOffset = (totalNbLines) -toScroll;
      }
      lcd.VerticalScrollStartAddress(scrollOffset);
    }

  } else if (scrollDirection == FullRefreshDirections::Up) {

    if (area->y1 > 0) {
      if (area->y2 == visibleNbLines - 1) {
        scrollOffset += (height * 2);
        scrollDirection = FullRefreshDirections::None;
        lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_0);
      } else {
        scrollOffset += height;
      }
      scrollOffset = scrollOffset % totalNbLines;
      lcd.VerticalScrollStartAddress(scrollOffset);
    }
  } else if (scrollDirection == FullRefreshDirections::Left or scrollDirection == FullRefreshDirections::LeftAnim) {
    if (area->x2 == visibleNbLines - 1) {
      scrollDirection = FullRefreshDirections::None;
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_0);
    }
  } else if (scrollDirection == FullRefreshDirections::Right or scrollDirection == FullRefreshDirections::RightAnim) {
    if (area->x1 == 0) {
      scrollDirection = FullRefreshDirections::None;
      lv_display_set_rotation(lv_disp_get_default(), LV_DISP_ROTATION_0);
    }
  }

  if (y2 < y1) {
    height = totalNbLines - y1;

    if (height > 0) {
      lcd.DrawBuffer(area->x1, y1, width, height, px_map, width * height * 2);
    }

    uint16_t pixOffset = width * height;
    height = y2 + 1;
    lcd.DrawBuffer(area->x1, 0, width, height, px_map + pixOffset, width * height * 2);

  } else {
    lcd.DrawBuffer(area->x1, y1, width, height, px_map, width * height * 2);
  }

  // IMPORTANT!!!
  // Inform the graphics library that you are ready with the flushing
  lv_display_flush_ready(disp_buf_2);
}

void LittleVgl::SetNewTouchPoint(int16_t x, int16_t y, bool contact) {
  if (contact) {
    if (!isCancelled) {
      touchPoint = {x, y};
      tapped = true;
    }
  } else {
    if (isCancelled) {
      touchPoint = {-1, -1};
      tapped = false;
      isCancelled = false;
    } else {
      touchPoint = {x, y};
      tapped = false;
    }
  }
}

void LittleVgl::CancelTap() {
  if (tapped) {
    isCancelled = true;
    touchPoint = {-1, -1};
  }
}

bool LittleVgl::GetTouchPadInfo(lv_indev_data_t* ptr) {
  ptr->point.x = touchPoint.x;
  ptr->point.y = touchPoint.y;
  if (tapped) {
    ptr->state = LV_INDEV_STATE_PR;
  } else {
    ptr->state = LV_INDEV_STATE_REL;
  }
  return false;
}
