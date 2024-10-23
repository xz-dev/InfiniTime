#include "components/ble/RemoteFont.h"
#include "components/ble/NimbleController.h"
#include "RemoteFont.h"
#include "components/fs/FS.h"
#include <hal/nrf_rtc.h>

using namespace Pinetime::Controllers;

namespace {
  // 0006yyxx-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x06, 0x00}};
  }

  // 00060000-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t fontServiceUuid {BaseUuid()};
  constexpr ble_uuid128_t requestFontUuid {CharUuid(0x01, 0x00)};
  constexpr ble_uuid128_t downloadFontUuid {CharUuid(0x02, 0x00)};

  int RemoteFontCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto anService = static_cast<RemoteFont*>(arg);
    return anService->OnDownloadFont(ctxt);
  }
}

namespace Pinetime {
  namespace Controllers {
    RemoteFont::RemoteFont(NimbleController& nimble, FS& fs) : nimble {nimble}, fs {fs} {
      characteristicDefinition[0] = {.uuid = &requestFontUuid.u,
                                     .access_cb = RemoteFontCallback,
                                     .arg = this,
                                     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                                     .val_handle = &eventHandle};
      characteristicDefinition[1] = {.uuid = &downloadFontUuid.u,
                                     .access_cb = RemoteFontCallback,
                                     .arg = this,
                                     .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                                     .val_handle = nullptr};
      characteristicDefinition[2] = {0};

      serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &fontServiceUuid.u, .characteristics = characteristicDefinition};
      serviceDefinition[1] = {0};
    }

    void RemoteFont::Init() {
      uint8_t res = 0;
      res = ble_gatts_count_cfg(serviceDefinition);
      ASSERT(res == 0);
      res = ble_gatts_add_svcs(serviceDefinition);
      ASSERT(res == 0);
      fs.DirCreate(FONT_DIR);
      ClearAllFonts();
    }

    int RemoteFont::RequestFont(uint16_t codePoint) {
      auto* om = ble_hs_mbuf_from_flat(&codePoint, 2);

      uint16_t connectionHandle = nimble.connHandle();

      if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
        return BLE_HS_ENOTCONN;
      }

      return ble_gattc_notify_custom(connectionHandle, eventHandle, om);
    }

    int DeleteFonts(FS& fs, lfs_info& info) {
      if (info.type == LFS_TYPE_REG) { // Ensure we only delete files
        char fullPath[sizeof(RemoteFont::FONT_DIR) + sizeof(info.name) + 1];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", RemoteFont::FONT_DIR, info.name);
        return fs.FileDelete(fullPath);
      }
      return 0;
    }

    int RemoteFont::ClearAllFonts() {
      return fs.DirList(FONT_DIR, &DeleteFonts);
    }

    int RemoteFont::OnDownloadFont(struct ble_gatt_access_ctxt* ctxt) {
      uint16_t codePoint = static_cast<uint16_t>(ctxt->om->om_data[0] << 8 | ctxt->om->om_data[1]);
      char file_name[6];
      snprintf(file_name, sizeof(file_name), "%u", codePoint);
      std::string fontFileName = std::string(FONT_DIR) + "/" + file_name;
      lfs_file_t file;
      int result = fs.FileOpen(&file, fontFileName.c_str(), LFS_O_CREAT | LFS_O_RDWR | LFS_O_TRUNC);
      if (result < 0) {
        return result;
      }

      struct os_mbuf* om = ctxt->om;
      uint8_t* data_ptr = om->om_data + 2;
      uint16_t data_len = om->om_len - 2;

      while (om != nullptr) {
        int written = fs.FileWrite(&file, data_ptr, data_len);
        if (written < 0) {
          fs.FileClose(&file);
          return written;
        }
        om = SLIST_NEXT(om, om_next);
        if (om != nullptr) {
          data_ptr = om->om_data;
          data_len = om->om_len;
        }
      }

      return fs.FileClose(&file);
    }
  }
}