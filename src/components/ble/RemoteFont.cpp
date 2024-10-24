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
      ClearAllFontCache();
    }

    int RemoteFont::GetFont(uint16_t codePoint, lfs_file_t* file) {
      char file_name[6];
      snprintf(file_name, sizeof(file_name), "%u", codePoint);
      char fullPath[sizeof(FONT_DIR) + sizeof(file_name) + 1];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", FONT_DIR, file_name);
      int result = fs.FileOpen(file, fullPath, LFS_O_RDONLY);
      if (result < 0) {
        return result;
      }
      CacheNodeMoveToTop(codePoint);
      return result;
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

    int RemoteFont::CacheNodeAppend(uint16_t codePoint) {
      // open cache file and append codePoint in new line
      int result;
      uint16_t node = cacheStart;
      uint16_t nextNode;
      lfs_file_t cacheFile;
      char cachePath[sizeof(FONT_DIR) + 6];
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, node);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      // read the first node
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&node), sizeof(node));
      if (result < 0) {
        // if file is empty, the next node is the end
        nextNode = cacheEnd;
      } else {
        // read the next node
        result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&nextNode), sizeof(nextNode));
        if (result < 0) {
          return result;
        }
      }
      // update the start node
      result = fs.FileSeek(&cacheFile, 0);
      if (result < 0) {
        return result;
      }
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&codePoint), sizeof(codePoint));
      if (result < 0) {
        return result;
      }
      // create new node file, name is the codePoint, and write the start node and the next node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, codePoint);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&node), sizeof(node));
      if (result < 0) {
        return result;
      }
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&nextNode), sizeof(nextNode));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      if (result < 0) {
        return result;
      }
      // update the next node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, nextNode);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&codePoint), sizeof(codePoint));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      return result;
    }

    int RemoteFont::CacheNodeMoveToTop(uint16_t codePoint) {
      uint16_t node;
      lfs_file_t cacheFile;
      char cachePath[sizeof(FONT_DIR) + 6];
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, codePoint);
      int result = fs.FileOpen(&cacheFile, cachePath, LFS_O_RDONLY);
      if (result < 0) {
        return result;
      }
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&node), sizeof(node));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      if (result < 0) {
        return result;
      }
      if (node == cacheStart) {
        return 0;
      }
      result = CacheNodeDelete(codePoint);
      if (result < 0) {
        return result;
      }
      return CacheNodeAppend(codePoint);
    }

    int RemoteFont::CacheNodeDelete(uint16_t node) {
      lfs_file_t cacheFile;
      char cachePath[sizeof(FONT_DIR) + 6];
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, node);
      uint16_t lastNode;
      uint16_t nextNode;
      int result;
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_RDONLY);
      if (result < 0) {
        return result;
      }
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&lastNode), sizeof(lastNode));
      if (result < 0) {
        return result;
      }
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&nextNode), sizeof(nextNode));
      if (result < 0) {
        return result;
      }
      // delete the node file
      result = fs.FileClose(&cacheFile);
      if (result < 0) {
        return result;
      }
      result = fs.FileDelete(cachePath);
      if (result < 0) {
        return result;
      }
      // update the last node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, lastNode);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      // seek 2 bytes from the end
      result = fs.FileSeek(&cacheFile, sizeof(lastNode));
      if (result < 0) {
        return result;
      }
      // write the next node to last node
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&nextNode), sizeof(nextNode));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      // update the next node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, nextNode);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      // write the last node to next node
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&lastNode), sizeof(lastNode));
      if (result < 0) {
        return result;
      }
      return fs.FileClose(&cacheFile);
    }

    int RemoteFont::CacheNodeRemoveOldest(uint16_t& oldestNode) {
      uint16_t node = cacheEnd;
      uint16_t lastNode;
      uint16_t last2Node;
      int result;
      lfs_file_t cacheFile;
      char cachePath[sizeof(FONT_DIR) + 6];
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, node);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&lastNode), sizeof(lastNode));
      if (result < 0) {
        return result;
      }
      oldestNode = lastNode;
      result = fs.FileClose(&cacheFile);
      if (result < 0) {
        return result;
      }
      // open the last node file and read the next last node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, lastNode);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      result = fs.FileRead(&cacheFile, reinterpret_cast<uint8_t*>(&last2Node), sizeof(last2Node));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      // delete the last node file
      result = fs.FileDelete(cachePath);
      if (result < 0) {
        return result;
      }
      // update the last2 node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, last2Node);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      // seek 2 bytes from the end
      result = fs.FileSeek(&cacheFile, sizeof(last2Node));
      if (result < 0) {
        return result;
      }
      // write the end node to last2 node
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&cacheEnd), sizeof(cacheEnd));
      if (result < 0) {
        return result;
      }
      result = fs.FileClose(&cacheFile);
      if (result < 0) {
        return result;
      }
      // update the end node
      snprintf(cachePath, sizeof(cachePath), "%s/%u", FONT_DIR, cacheEnd);
      result = fs.FileOpen(&cacheFile, cachePath, LFS_O_CREAT | LFS_O_RDWR);
      if (result < 0) {
        return result;
      }
      // write the last2 node to end node
      result = fs.FileWrite(&cacheFile, reinterpret_cast<const uint8_t*>(&last2Node), sizeof(last2Node));
      if (result < 0) {
        return result;
      }
      return fs.FileClose(&cacheFile);
    }

    int RemoteFont::ClearAllFontCache() {
      return fs.DirList(FONT_DIR, DeleteFonts);
    }

    int RemoteFont::OnDownloadFont(struct ble_gatt_access_ctxt* ctxt) {
      uint16_t codePoint = static_cast<uint16_t>(ctxt->om->om_data[0] << 8 | ctxt->om->om_data[1]);
      char file_name[6];
      snprintf(file_name, sizeof(file_name), "%u", codePoint);
      char fullPath[sizeof(RemoteFont::FONT_DIR) + sizeof(file_name) + 1];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", FONT_DIR, file_name);
      lfs_file_t file;
      int result = fs.FileOpen(&file, fullPath, LFS_O_CREAT | LFS_O_RDWR | LFS_O_TRUNC);
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

      result = fs.FileClose(&file);
      if (result < 0) {
        return result;
      }
      result = CacheNodeAppend(codePoint);
      if (result < 0) {
        return result;
      }
      cacheSize++;
      if (cacheSize > maxCacheSize) {
        uint16_t oldestNode;
        result = CacheNodeRemoveOldest(oldestNode);
        if (result < 0) {
          return result;
        }
        snprintf(fullPath, sizeof(fullPath), "%s/%u", FONT_DIR, oldestNode);
        result = fs.FileDelete(fullPath);
        if (result < 0) {
          return result;
        }
        cacheSize--;
      }
      return cacheSize;
    }
  }
}