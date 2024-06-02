#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/src/core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class FirmwareValidator;
  }

  namespace Applications {
    namespace Screens {

      class FirmwareValidation : public Screen {
      public:
        FirmwareValidation(Pinetime::Controllers::FirmwareValidator& validator);
        ~FirmwareValidation() override;

        void OnValidateButtonEvent(lv_event_t * event);
        void OnResetButtonEvent(lv_event_t * event);

      private:
        Pinetime::Controllers::FirmwareValidator& validator;

        lv_obj_t* labelVersion;
        lv_obj_t* labelIsValidated;
        lv_obj_t* buttonValidate;
        lv_obj_t* labelButtonValidate;
        lv_obj_t* buttonReset;
        lv_obj_t* labelButtonReset;
      };
    }
  }
}
