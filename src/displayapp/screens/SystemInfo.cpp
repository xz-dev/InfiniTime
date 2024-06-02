#include <FreeRTOS.h>
#include <algorithm>
#include <task.h>
#include "displayapp/screens/SystemInfo.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Label.h"
#include "Version.h"
#include "BootloaderVersion.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "components/motion/MotionController.h"
#include "drivers/Watchdog.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(const Pinetime::Controllers::MotionController::DeviceTypes deviceType) {
    switch (deviceType) {
      case Pinetime::Controllers::MotionController::DeviceTypes::BMA421:
        return "BMA421";
      case Pinetime::Controllers::MotionController::DeviceTypes::BMA425:
        return "BMA425";
      case Pinetime::Controllers::MotionController::DeviceTypes::Unknown:
        return "???";
    }
    return "???";
  }
}

SystemInfo::SystemInfo(Pinetime::Applications::DisplayApp* app,
                       Pinetime::Controllers::DateTime& dateTimeController,
                       const Pinetime::Controllers::Battery& batteryController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       const Pinetime::Controllers::Ble& bleController,
                       const Pinetime::Drivers::Watchdog& watchdog,
                       Pinetime::Controllers::MotionController& motionController,
                       const Pinetime::Drivers::Cst816S& touchPanel)
  : app {app},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    brightnessController {brightnessController},
    bleController {bleController},
    watchdog {watchdog},
    motionController {motionController},
    touchPanel {touchPanel},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen3();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen4();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen5();
              }},
             Screens::ScreenListModes::UpDown} {
}

SystemInfo::~SystemInfo() {
  lv_obj_clean(lv_screen_active());
}

bool SystemInfo::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen1() {
  lv_obj_t* spans = lv_spangroup_create(lv_screen_active());
  lv_span_t* span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, "InfiniTime\n\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, get_text_fmt("Version %u.%u.%u\n", Version::Major(), Version::Minor(), Version::Patch()));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, get_text_fmt("Short Ref %s\n", Version::GitCommitHash()));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, "Build date\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, __DATE__ "\n");
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, __TIME__ "\n\n");
  span = lv_spangroup_new_span(spans);
  lv_span_set_text(span, get_text_fmt("Bootloader %s\n", BootloaderVersion::VersionString()));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  lv_spangroup_set_align(spans, LV_TEXT_ALIGN_CENTER);
  lv_obj_align_to(spans, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(0, 5, spans);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen2() {
  auto batteryPercent = batteryController.PercentRemaining();
  const auto* resetReason = [this]() {
    switch (watchdog.GetResetReason()) {
      case Drivers::Watchdog::ResetReason::Watchdog:
        return "wtdg";
      case Drivers::Watchdog::ResetReason::HardReset:
        return "hardr";
      case Drivers::Watchdog::ResetReason::NFC:
        return "nfc";
      case Drivers::Watchdog::ResetReason::SoftReset:
        return "softr";
      case Drivers::Watchdog::ResetReason::CpuLockup:
        return "cpulock";
      case Drivers::Watchdog::ResetReason::SystemOff:
        return "off";
      case Drivers::Watchdog::ResetReason::LpComp:
        return "lpcomp";
      case Drivers::Watchdog::ResetReason::DebugInterface:
        return "dbg";
      case Drivers::Watchdog::ResetReason::ResetPin:
        return "rst";
      default:
        return "?";
    }
  }();

  // uptime
  static constexpr uint32_t secondsInADay = 60 * 60 * 24;
  static constexpr uint32_t secondsInAnHour = 60 * 60;
  static constexpr uint32_t secondsInAMinute = 60;
  uint32_t uptimeSeconds = dateTimeController.Uptime().count();
  uint32_t uptimeDays = (uptimeSeconds / secondsInADay);
  uptimeSeconds = uptimeSeconds % secondsInADay;
  uint32_t uptimeHours = uptimeSeconds / secondsInAnHour;
  uptimeSeconds = uptimeSeconds % secondsInAnHour;
  uint32_t uptimeMinutes = uptimeSeconds / secondsInAMinute;
  uptimeSeconds = uptimeSeconds % secondsInAMinute;
  // TODO handle more than 100 days of uptime

#ifndef TARGET_DEVICE_NAME
  #define TARGET_DEVICE_NAME "UNKNOWN"
#endif

  lv_obj_t* label = lv_spangroup_create(lv_screen_active());
  lv_span_t* span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Date");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span,
                   get_text_fmt(" %04d-%02d-%02d\n",
                                dateTimeController.Year(),
                                static_cast<uint8_t>(dateTimeController.Month()),
                                dateTimeController.Day()));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Time");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(
    span,
    get_text_fmt(" %02d:%02d:%02d\n", dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds()));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Uptime");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %02lu:%02lu:%02lu\n", uptimeDays, uptimeHours, uptimeMinutes));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Battery");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %d%%/%03imV\n", batteryPercent, batteryController.Voltage()));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Backlight");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %s\n", brightnessController.ToString()));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Last reset");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %s\n", resetReason));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Accel.");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %s\n", ToString(motionController.DeviceType())));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Touch.");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" %x.%x.%x\n", touchPanel.GetChipId(), touchPanel.GetVendorId(), touchPanel.GetFwVersion()));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Model");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, " " TARGET_DEVICE_NAME);
  lv_spangroup_set_align(label, LV_TEXT_ALIGN_CENTER);
  lv_spangroup_refr_mode(label);
  lv_obj_align_to(label, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(1, 5, label);
}

extern int mallocFailedCount;
extern int stackOverflowCount;

std::unique_ptr<Screen> SystemInfo::CreateScreen3() {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);

  lv_obj_t* label = lv_spangroup_create(lv_screen_active());
  const auto& bleAddr = bleController.Address();
  lv_span_t* span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "BLE MAC\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(
    span,
    get_text_fmt(" %02x:%02x:%02x:%02x:%02x:%02x\n\n", bleAddr[5], bleAddr[4], bleAddr[3], bleAddr[2], bleAddr[1], bleAddr[0]));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Memory heap\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" Free %d\n", xPortGetFreeHeapSize()));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" Min free %d\n", xPortGetMinimumEverFreeHeapSize()));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" Alloc err %d\n", mallocFailedCount));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, get_text_fmt(" Ovrfl err %d\n", stackOverflowCount));
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  lv_spangroup_set_align(label, LV_TEXT_ALIGN_CENTER);
  lv_spangroup_refr_mode(label);
  lv_obj_align_to(label, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(2, 5, label);
}

bool SystemInfo::sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs) {
  return lhs.xTaskNumber < rhs.xTaskNumber;
}

std::unique_ptr<Screen> SystemInfo::CreateScreen4() {
  static constexpr uint8_t maxTaskCount = 9;
  TaskStatus_t tasksStatus[maxTaskCount];

  lv_obj_t* infoTask = lv_table_create(lv_screen_active());
  lv_table_set_col_cnt(infoTask, 4);
  lv_table_set_row_cnt(infoTask, maxTaskCount + 1);
  lv_obj_set_style_pad_all(infoTask, 0, LV_PART_ITEMS);
  lv_obj_set_style_border_color(infoTask, Colors::lightGray, LV_PART_MAIN);

  lv_table_set_cell_value(infoTask, 0, 0, "#");
  lv_table_set_col_width(infoTask, 0, 30);
  lv_table_set_cell_value(infoTask, 0, 1, "S"); // State
  lv_table_set_col_width(infoTask, 1, 30);
  lv_table_set_cell_value(infoTask, 0, 2, "Task");
  lv_table_set_col_width(infoTask, 2, 80);
  lv_table_set_cell_value(infoTask, 0, 3, "Free");
  lv_table_set_col_width(infoTask, 3, 90);

  auto nb = uxTaskGetSystemState(tasksStatus, maxTaskCount, nullptr);
  std::sort(tasksStatus, tasksStatus + nb, sortById);
  for (uint8_t i = 0; i < nb && i < maxTaskCount; i++) {
    char buffer[11] = {0};

    snprintf(buffer, sizeof(buffer), "%lu", tasksStatus[i].xTaskNumber);
    lv_table_set_cell_value(infoTask, i + 1, 0, buffer);
    switch (tasksStatus[i].eCurrentState) {
      case eReady:
      case eRunning:
        buffer[0] = 'R';
        break;
      case eBlocked:
        buffer[0] = 'B';
        break;
      case eSuspended:
        buffer[0] = 'S';
        break;
      case eDeleted:
        buffer[0] = 'D';
        break;
      default:
        buffer[0] = 'I'; // Invalid
        break;
    }
    buffer[1] = '\0';
    lv_table_set_cell_value(infoTask, i + 1, 1, buffer);
    lv_table_set_cell_value(infoTask, i + 1, 2, tasksStatus[i].pcTaskName);
    if (tasksStatus[i].usStackHighWaterMark < 20) {
      snprintf(buffer, sizeof(buffer), "%" PRIu16 " low", tasksStatus[i].usStackHighWaterMark);
    } else {
      snprintf(buffer, sizeof(buffer), "%" PRIu16, tasksStatus[i].usStackHighWaterMark);
    }
    lv_table_set_cell_value(infoTask, i + 1, 3, buffer);
  }
  return std::make_unique<Screens::Label>(3, 5, infoTask);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen5() {
  lv_obj_t* label = lv_spangroup_create(lv_screen_active());
  lv_span_t* span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Software Licensed\n");
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "under the terms of\n");
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "the GNU General\n");
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Public License v3\n");
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "Source code\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0x80, 0x80, 0x80));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "https://github.com/\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "InfiniTimeOrg/\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  span = lv_spangroup_new_span(label);
  lv_span_set_text(span, "InfiniTime\n");
  lv_style_set_text_color(&span->style, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  lv_spangroup_set_align(label, LV_TEXT_ALIGN_CENTER);
  lv_spangroup_refr_mode(label);
  lv_obj_align_to(label, lv_screen_active(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(4, 5, label);
}
