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
      bool IsBuildInFont(uint16_t codePoint);
      int RequestFont(uint16_t codePoint);
      // Font directory path
      static constexpr char const* const FONT_DIR = "/remote_fonts";

    private:
      NimbleController& nimble;
      FS& fs;

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t eventHandle;

      void MoveToTop(uint16_t codePoint);
      void RemoveOldestFont();
      int ClearAllFonts();

      // Attributes
      static constexpr size_t maxCacheSize = 250;
    };
  }
}
