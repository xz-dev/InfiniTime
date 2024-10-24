#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

#include <string>

#include "components/fs/FS.h"

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class RemoteFont {
    public:
      RemoteFont(NimbleController& nimble, FS& fs);
      int OnDownloadFont(struct ble_gatt_access_ctxt* ctxt);

      void Init();
      int GetFont(uint16_t codePoint, lfs_file_t* file);
      int RequestFont(uint16_t codePoint);
      // Font directory path
      static constexpr char const* const FONT_DIR = "/remote_fonts";

    private:
      NimbleController& nimble;
      FS& fs;

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t eventHandle;

      int CacheNodeAppend(uint16_t codePoint);
      int CacheNodeMoveToTop(uint16_t codePoint);
      int CacheNodeDelete(uint16_t node);
      int CacheNodeRemoveOldest(uint16_t& oldestNode);
      int ClearAllFontCache();

      // Attributes
      static constexpr size_t maxCacheSize = 250;
      // current cache size
      size_t cacheSize = 0;
      /**
       * File containing the cache of code points
       * Start -> Cache 1 <--> Cache 2 <--> Cache 3 <--> Cache 4 <--> Cache 5 <- End
       */
      static constexpr uint16_t const cacheStart = 0x0000;
      static constexpr uint16_t const cacheEnd = 0xFFFF;
    };
  }
}
