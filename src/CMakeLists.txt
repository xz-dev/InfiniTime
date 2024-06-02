cmake_minimum_required(VERSION 3.10)

project(pinetime-app C CXX ASM)


# define some variables just for this example to determine file locations
set(NRF_PROJECT_NAME pinetime-app)
set(NRF_BOARD pca10040)

# check if all the necessary tools paths have been provided.
if (NOT NRF5_SDK_PATH)
  message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif ()
if (DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
  set(ARM_NONE_EABI_TOOLCHAIN_BIN_PATH ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin)
endif ()

if (NOT NRF_TARGET MATCHES "nrf52")
  message(FATAL_ERROR "Only rRF52 boards are supported right now")
endif ()

# Setup toolchain
include(${CMAKE_SOURCE_DIR}/cmake-nRF5x/arm-gcc-toolchain.cmake)

if (NOT DEFINED ARM_GCC_TOOLCHAIN)
  message(FATAL_ERROR "The toolchain must be set up before calling this macro")
endif ()
set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")


set(SDK_SOURCE_FILES
        # Startup
        "${NRF5_SDK_PATH}/modules/nrfx/mdk/system_nrf52.c"
        "${NRF5_SDK_PATH}/modules/nrfx/mdk/gcc_startup_nrf52.S"

        # Base SDK
        "${NRF5_SDK_PATH}/components/boards/boards.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.h"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_clock.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_gpiote.c"
        "${NRF5_SDK_PATH}/modules/nrfx/soc/nrfx_atomic.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_saadc.c"

        # FreeRTOS
        ${NRF5_SDK_PATH}/external/freertos/source/croutine.c
        ${NRF5_SDK_PATH}/external/freertos/source/event_groups.c
        ${NRF5_SDK_PATH}/external/freertos/source/list.c
        ${NRF5_SDK_PATH}/external/freertos/source/queue.c
        ${NRF5_SDK_PATH}/external/freertos/source/stream_buffer.c
        ${NRF5_SDK_PATH}/external/freertos/source/tasks.c
        ${NRF5_SDK_PATH}/external/freertos/source/timers.c

        # Libs
        "${NRF5_SDK_PATH}/components/libraries/atomic/nrf_atomic.c"
        "${NRF5_SDK_PATH}/components/libraries/balloc/nrf_balloc.c"
        "${NRF5_SDK_PATH}/components/libraries/util/nrf_assert.c"
        "${NRF5_SDK_PATH}/components/libraries/util/app_error.c"
        "${NRF5_SDK_PATH}/components/libraries/util/app_error_weak.c"
        "${NRF5_SDK_PATH}/components/libraries/util/app_error_handler_gcc.c"
        "${NRF5_SDK_PATH}/components/libraries/util/app_util_platform.c"
        "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_rtt.c"
        "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_serial.c"
        "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_default_backends.c"
        "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_frontend.c"
        "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_str_formatter.c"
        "${NRF5_SDK_PATH}/components/libraries/memobj/nrf_memobj.c"
        "${NRF5_SDK_PATH}/components/libraries/ringbuf/nrf_ringbuf.c"
        "${NRF5_SDK_PATH}/components/libraries/strerror/nrf_strerror.c"

        # Segger RTT
        "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c"
        "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT.c"
        "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_printf.c"

        # Other
        "${NRF5_SDK_PATH}/external/utf_converter/utf.c"
        "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf.c"
        "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf_format.c"

        # TWI
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_twim.c"

        # GPIOTE
        "${NRF5_SDK_PATH}/components/libraries/gpiote/app_gpiote.c"
        )

set(TINYCRYPT_SRC
        libs/mynewt-nimble/ext/tinycrypt/src/aes_encrypt.c
        libs/mynewt-nimble/ext/tinycrypt/src/utils.c
        libs/mynewt-nimble/ext/tinycrypt/src/cmac_mode.c
        libs/mynewt-nimble/ext/tinycrypt/src/ecc.c
        libs/mynewt-nimble/ext/tinycrypt/src/ecc_dh.c
        )

set(NIMBLE_SRC
        libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c
        libs/mynewt-nimble/porting/npl/freertos/src/npl_os_freertos.c
        libs/mynewt-nimble/nimble/host/src/ble_hs.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_hci_evt.c
        libs/mynewt-nimble/nimble/host/src/ble_l2cap_sig_cmd.c
        libs/mynewt-nimble/nimble/host/src/ble_l2cap_sig.c
        libs/mynewt-nimble/nimble/host/src/ble_l2cap.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_mbuf.c
        libs/mynewt-nimble/nimble/host/src/ble_sm.c
        libs/mynewt-nimble/nimble/host/src/ble_sm_cmd.c
        libs/mynewt-nimble/nimble/host/src/ble_sm_lgcy.c
        libs/mynewt-nimble/nimble/host/src/ble_sm_alg.c
        libs/mynewt-nimble/nimble/host/src/ble_sm_sc.c
        libs/mynewt-nimble/nimble/host/src/ble_gap.c
        libs/mynewt-nimble/nimble/host/src/ble_gatts.c
        libs/mynewt-nimble/nimble/host/src/ble_gattc.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_conn.c
        libs/mynewt-nimble/nimble/host/src/ble_att_svr.c
        libs/mynewt-nimble/nimble/host/src/ble_store.c
        libs/mynewt-nimble/nimble/host/src/ble_store_util.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_pvcy.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_hci.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_log.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_hci_util.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_hci_cmd.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_cfg.c
        libs/mynewt-nimble/nimble/host/src/ble_uuid.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_id.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_misc.c
        libs/mynewt-nimble/nimble/host/src/ble_att.c
        libs/mynewt-nimble/nimble/host/src/ble_att_clt.c
        libs/mynewt-nimble/nimble/host/src/ble_att_svr.c
        libs/mynewt-nimble/nimble/host/src/ble_att_cmd.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_atomic.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_adv.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_flow.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_mqueue.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_stop.c
        libs/mynewt-nimble/nimble/host/src/ble_hs_startup.c
        libs/mynewt-nimble/nimble/host/store/ram/src/ble_store_ram.c
        libs/mynewt-nimble/nimble/host/src/ble_monitor.c
        libs/mynewt-nimble/nimble/transport/ram/src/ble_hci_ram.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_rand.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_conn.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_ctrl.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_hci.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_conn_hci.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_utils.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_scan.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_whitelist.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_adv.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_sched.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_supp_cmd.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_hci_ev.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_rfmgmt.c
        libs/mynewt-nimble/nimble/controller/src/ble_ll_resolv.c
        libs/mynewt-nimble/porting/nimble/src/os_cputime.c
        libs/mynewt-nimble/porting/nimble/src/os_cputime_pwr2.c
        libs/mynewt-nimble/porting/nimble/src/os_mbuf.c
        libs/mynewt-nimble/porting/nimble/src/os_mempool.c
        libs/mynewt-nimble/porting/nimble/src/hal_timer.c
        libs/mynewt-nimble/porting/nimble/src/mem.c
        libs/mynewt-nimble/porting/nimble/src/endian.c
        libs/mynewt-nimble/porting/nimble/src/os_msys_init.c
        libs/mynewt-nimble/nimble/drivers/nrf52/src/ble_hw.c
        libs/mynewt-nimble/nimble/drivers/nrf52/src/ble_phy.c
        libs/mynewt-nimble/nimble/host/services/gap/src/ble_svc_gap.c
        libs/mynewt-nimble/nimble/host/services/gatt/src/ble_svc_gatt.c
        libs/mynewt-nimble/nimble/host/util/src/addr.c
        )

set(LITTLEFS_SRC
        libs/littlefs/lfs_util.h
        libs/littlefs/lfs.h
        libs/littlefs/lfs_util.c
        libs/littlefs/lfs.c
        )

set(LVGL_SRC
        libs/lv_conf.h
        libs/lvgl/lvgl.h
        libs/lvgl/src/core/lv_global.h
        libs/lvgl/src/core/lv_group.c
        libs/lvgl/src/core/lv_group.h
        libs/lvgl/src/core/lv_obj.c
        libs/lvgl/src/core/lv_obj.h
        libs/lvgl/src/core/lv_obj_class.c
        libs/lvgl/src/core/lv_obj_class.h
        libs/lvgl/src/core/lv_obj_draw.c
        libs/lvgl/src/core/lv_obj_draw.h
        libs/lvgl/src/core/lv_obj_event.c
        libs/lvgl/src/core/lv_obj_event.h
        libs/lvgl/src/core/lv_obj_id_builtin.c
        libs/lvgl/src/core/lv_obj_pos.c
        libs/lvgl/src/core/lv_obj_pos.h
        libs/lvgl/src/core/lv_obj_property.c
        libs/lvgl/src/core/lv_obj_property.h
        libs/lvgl/src/core/lv_obj_scroll.c
        libs/lvgl/src/core/lv_obj_scroll.h
        libs/lvgl/src/core/lv_obj_style.c
        libs/lvgl/src/core/lv_obj_style.h
        libs/lvgl/src/core/lv_obj_style_gen.c
        libs/lvgl/src/core/lv_obj_style_gen.h
        libs/lvgl/src/core/lv_obj_tree.c
        libs/lvgl/src/core/lv_obj_tree.h
        libs/lvgl/src/core/lv_refr.c
        libs/lvgl/src/core/lv_refr.h
        libs/lvgl/src/display/lv_display.c
        libs/lvgl/src/display/lv_display.h
        libs/lvgl/src/display/lv_display_private.h
        libs/lvgl/src/draw/lv_draw.c
        libs/lvgl/src/draw/lv_draw.h
        libs/lvgl/src/draw/lv_draw_arc.c
        libs/lvgl/src/draw/lv_draw_arc.h
        libs/lvgl/src/draw/lv_draw_buf.c
        libs/lvgl/src/draw/lv_draw_buf.h
        libs/lvgl/src/draw/lv_draw_image.c
        libs/lvgl/src/draw/lv_draw_image.h
        libs/lvgl/src/draw/lv_draw_label.c
        libs/lvgl/src/draw/lv_draw_label.h
        libs/lvgl/src/draw/lv_draw_line.c
        libs/lvgl/src/draw/lv_draw_line.h
        libs/lvgl/src/draw/lv_draw_mask.c
        libs/lvgl/src/draw/lv_draw_mask.h
        libs/lvgl/src/draw/lv_draw_rect.c
        libs/lvgl/src/draw/lv_draw_rect.h
        libs/lvgl/src/draw/lv_draw_triangle.c
        libs/lvgl/src/draw/lv_draw_triangle.h
        libs/lvgl/src/draw/lv_draw_vector.c
        libs/lvgl/src/draw/lv_draw_vector.h
        libs/lvgl/src/draw/lv_image_decoder.c
        libs/lvgl/src/draw/lv_image_decoder.h
        libs/lvgl/src/draw/lv_image_dsc.h
        libs/lvgl/src/draw/nxp/pxp/lv_draw_buf_pxp.c
        libs/lvgl/src/draw/nxp/pxp/lv_draw_pxp.c
        libs/lvgl/src/draw/nxp/pxp/lv_draw_pxp.h
        libs/lvgl/src/draw/nxp/pxp/lv_draw_pxp_fill.c
        libs/lvgl/src/draw/nxp/pxp/lv_draw_pxp_img.c
        libs/lvgl/src/draw/nxp/pxp/lv_draw_pxp_layer.c
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_cfg.c
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_cfg.h
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_osa.c
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_osa.h
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_utils.c
        libs/lvgl/src/draw/nxp/pxp/lv_pxp_utils.h
        libs/lvgl/src/draw/nxp/vglite/lv_draw_buf_vglite.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite.h
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_border.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_fill.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_img.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_label.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_layer.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.c
        libs/lvgl/src/draw/nxp/vglite/lv_draw_vglite_triangle.c
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_buf.c
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_buf.h
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_matrix.c
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_matrix.h
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_path.c
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_path.h
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_utils.c
        libs/lvgl/src/draw/nxp/vglite/lv_vglite_utils.h
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d.h
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_arc.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_border.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_fill.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_image.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_label.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_line.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_mask_rectangle.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_triangle.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_utils.c
        libs/lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_utils.h
        libs/lvgl/src/draw/sdl/lv_draw_sdl.c
        libs/lvgl/src/draw/sdl/lv_draw_sdl.h
        libs/lvgl/src/draw/sw/arm2d/lv_draw_sw_arm2d.h
        libs/lvgl/src/draw/sw/arm2d/lv_draw_sw_helium.h
        libs/lvgl/src/draw/sw/blend/arm2d/lv_blend_arm2d.h
        libs/lvgl/src/draw/sw/blend/helium/lv_blend_helium.h
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend.c
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend.h
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_argb8888.c
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_argb8888.h
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb565.c
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb565.h
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb888.c
        libs/lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb888.h
        libs/lvgl/src/draw/sw/blend/neon/lv_blend_neon.h
        libs/lvgl/src/draw/sw/lv_draw_sw.c
        libs/lvgl/src/draw/sw/lv_draw_sw.h
        libs/lvgl/src/draw/sw/lv_draw_sw_arc.c
        libs/lvgl/src/draw/sw/lv_draw_sw_border.c
        libs/lvgl/src/draw/sw/lv_draw_sw_box_shadow.c
        libs/lvgl/src/draw/sw/lv_draw_sw_fill.c
        libs/lvgl/src/draw/sw/lv_draw_sw_gradient.c
        libs/lvgl/src/draw/sw/lv_draw_sw_gradient.h
        libs/lvgl/src/draw/sw/lv_draw_sw_img.c
        libs/lvgl/src/draw/sw/lv_draw_sw_letter.c
        libs/lvgl/src/draw/sw/lv_draw_sw_line.c
        libs/lvgl/src/draw/sw/lv_draw_sw_mask.c
        libs/lvgl/src/draw/sw/lv_draw_sw_mask.h
        libs/lvgl/src/draw/sw/lv_draw_sw_mask_rect.c
        libs/lvgl/src/draw/sw/lv_draw_sw_transform.c
        libs/lvgl/src/draw/sw/lv_draw_sw_triangle.c
        libs/lvgl/src/draw/sw/lv_draw_sw_vector.c
        libs/lvgl/src/draw/vg_lite/lv_draw_buf_vg_lite.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite.h
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_arc.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_border.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_box_shadow.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_fill.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_img.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_label.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_layer.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_line.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_mask_rect.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_triangle.c
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_type.h
        libs/lvgl/src/draw/vg_lite/lv_draw_vg_lite_vector.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_decoder.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_decoder.h
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_grad.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_grad.h
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_math.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_math.h
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_path.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_path.h
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_pending.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_pending.h
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_utils.c
        libs/lvgl/src/draw/vg_lite/lv_vg_lite_utils.h
        libs/lvgl/src/drivers/display/drm/lv_linux_drm.c
        libs/lvgl/src/drivers/display/drm/lv_linux_drm.h
        libs/lvgl/src/drivers/display/fb/lv_linux_fbdev.c
        libs/lvgl/src/drivers/display/fb/lv_linux_fbdev.h
        libs/lvgl/src/drivers/display/ili9341/lv_ili9341.c
        libs/lvgl/src/drivers/display/ili9341/lv_ili9341.h
        libs/lvgl/src/drivers/display/lcd/lv_lcd_generic_mipi.c
        libs/lvgl/src/drivers/display/lcd/lv_lcd_generic_mipi.h
        libs/lvgl/src/drivers/display/st7735/lv_st7735.c
        libs/lvgl/src/drivers/display/st7735/lv_st7735.h
        libs/lvgl/src/drivers/display/st7789/lv_st7789.c
        libs/lvgl/src/drivers/display/st7789/lv_st7789.h
        libs/lvgl/src/drivers/display/st7796/lv_st7796.c
        libs/lvgl/src/drivers/display/st7796/lv_st7796.h
        libs/lvgl/src/drivers/display/tft_espi/lv_tft_espi.h
        libs/lvgl/src/drivers/evdev/lv_evdev.c
        libs/lvgl/src/drivers/evdev/lv_evdev.h
        libs/lvgl/src/drivers/libinput/lv_libinput.c
        libs/lvgl/src/drivers/libinput/lv_libinput.h
        libs/lvgl/src/drivers/libinput/lv_xkb.c
        libs/lvgl/src/drivers/libinput/lv_xkb.h
        libs/lvgl/src/drivers/lv_drivers.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_cache.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_cache.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_entry.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_entry.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_fbdev.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_fbdev.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_image_cache.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_image_cache.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_lcd.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_lcd.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_libuv.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_libuv.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_profiler.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_profiler.h
        libs/lvgl/src/drivers/nuttx/lv_nuttx_touchscreen.c
        libs/lvgl/src/drivers/nuttx/lv_nuttx_touchscreen.h
        libs/lvgl/src/drivers/sdl/lv_sdl_keyboard.c
        libs/lvgl/src/drivers/sdl/lv_sdl_keyboard.h
        libs/lvgl/src/drivers/sdl/lv_sdl_mouse.c
        libs/lvgl/src/drivers/sdl/lv_sdl_mouse.h
        libs/lvgl/src/drivers/sdl/lv_sdl_mousewheel.c
        libs/lvgl/src/drivers/sdl/lv_sdl_mousewheel.h
        libs/lvgl/src/drivers/sdl/lv_sdl_window.c
        libs/lvgl/src/drivers/sdl/lv_sdl_window.h
        libs/lvgl/src/drivers/windows/lv_windows_context.c
        libs/lvgl/src/drivers/windows/lv_windows_context.h
        libs/lvgl/src/drivers/windows/lv_windows_display.c
        libs/lvgl/src/drivers/windows/lv_windows_display.h
        libs/lvgl/src/drivers/windows/lv_windows_input.c
        libs/lvgl/src/drivers/windows/lv_windows_input.h
        libs/lvgl/src/drivers/x11/lv_x11.h
        libs/lvgl/src/drivers/x11/lv_x11_display.c
        libs/lvgl/src/drivers/x11/lv_x11_input.c
        libs/lvgl/src/font/lv_binfont_loader.c
        libs/lvgl/src/font/lv_binfont_loader.h
        libs/lvgl/src/font/lv_font.c
        libs/lvgl/src/font/lv_font.h
        libs/lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c
        libs/lvgl/src/font/lv_font_fmt_txt.c
        libs/lvgl/src/font/lv_font_fmt_txt.h
        libs/lvgl/src/font/lv_font_montserrat_10.c
        libs/lvgl/src/font/lv_font_montserrat_12.c
        libs/lvgl/src/font/lv_font_montserrat_14.c
        libs/lvgl/src/font/lv_font_montserrat_16.c
        libs/lvgl/src/font/lv_font_montserrat_18.c
        libs/lvgl/src/font/lv_font_montserrat_20.c
        libs/lvgl/src/font/lv_font_montserrat_22.c
        libs/lvgl/src/font/lv_font_montserrat_24.c
        libs/lvgl/src/font/lv_font_montserrat_26.c
        libs/lvgl/src/font/lv_font_montserrat_28.c
        libs/lvgl/src/font/lv_font_montserrat_28_compressed.c
        libs/lvgl/src/font/lv_font_montserrat_30.c
        libs/lvgl/src/font/lv_font_montserrat_32.c
        libs/lvgl/src/font/lv_font_montserrat_34.c
        libs/lvgl/src/font/lv_font_montserrat_36.c
        libs/lvgl/src/font/lv_font_montserrat_38.c
        libs/lvgl/src/font/lv_font_montserrat_40.c
        libs/lvgl/src/font/lv_font_montserrat_42.c
        libs/lvgl/src/font/lv_font_montserrat_44.c
        libs/lvgl/src/font/lv_font_montserrat_46.c
        libs/lvgl/src/font/lv_font_montserrat_48.c
        libs/lvgl/src/font/lv_font_montserrat_8.c
        libs/lvgl/src/font/lv_font_simsun_16_cjk.c
        libs/lvgl/src/font/lv_font_unscii_16.c
        libs/lvgl/src/font/lv_font_unscii_8.c
        libs/lvgl/src/font/lv_symbol_def.h
        libs/lvgl/src/indev/lv_indev.c
        libs/lvgl/src/indev/lv_indev.h
        libs/lvgl/src/indev/lv_indev_private.h
        libs/lvgl/src/indev/lv_indev_scroll.c
        libs/lvgl/src/indev/lv_indev_scroll.h
        libs/lvgl/src/layouts/flex/lv_flex.c
        libs/lvgl/src/layouts/flex/lv_flex.h
        libs/lvgl/src/layouts/grid/lv_grid.c
        libs/lvgl/src/layouts/grid/lv_grid.h
        libs/lvgl/src/layouts/lv_layout.c
        libs/lvgl/src/layouts/lv_layout.h
        libs/lvgl/src/libs/barcode/code128.c
        libs/lvgl/src/libs/barcode/code128.h
        libs/lvgl/src/libs/barcode/lv_barcode.c
        libs/lvgl/src/libs/barcode/lv_barcode.h
        libs/lvgl/src/libs/bin_decoder/lv_bin_decoder.c
        libs/lvgl/src/libs/bin_decoder/lv_bin_decoder.h
        libs/lvgl/src/libs/bmp/lv_bmp.c
        libs/lvgl/src/libs/bmp/lv_bmp.h
        libs/lvgl/src/libs/ffmpeg/lv_ffmpeg.c
        libs/lvgl/src/libs/ffmpeg/lv_ffmpeg.h
        libs/lvgl/src/libs/freetype/ftmodule.h
        libs/lvgl/src/libs/freetype/ftoption.h
        libs/lvgl/src/libs/freetype/lv_freetype.c
        libs/lvgl/src/libs/freetype/lv_freetype.h
        libs/lvgl/src/libs/freetype/lv_freetype_glyph.c
        libs/lvgl/src/libs/freetype/lv_freetype_image.c
        libs/lvgl/src/libs/freetype/lv_freetype_outline.c
        libs/lvgl/src/libs/freetype/lv_freetype_private.h
        libs/lvgl/src/libs/freetype/lv_ftsystem.c
        libs/lvgl/src/libs/fsdrv/lv_fs_cbfs.c
        libs/lvgl/src/libs/fsdrv/lv_fs_fatfs.c
        libs/lvgl/src/libs/fsdrv/lv_fs_littlefs.c
        libs/lvgl/src/libs/fsdrv/lv_fs_memfs.c
        libs/lvgl/src/libs/fsdrv/lv_fs_posix.c
        libs/lvgl/src/libs/fsdrv/lv_fs_stdio.c
        libs/lvgl/src/libs/fsdrv/lv_fs_win32.c
        libs/lvgl/src/libs/fsdrv/lv_fsdrv.h
        libs/lvgl/src/libs/gif/gifdec.c
        libs/lvgl/src/libs/gif/gifdec.h
        libs/lvgl/src/libs/gif/gifdec_mve.h
        libs/lvgl/src/libs/gif/lv_gif.c
        libs/lvgl/src/libs/gif/lv_gif.h
        libs/lvgl/src/libs/libjpeg_turbo/lv_libjpeg_turbo.c
        libs/lvgl/src/libs/libjpeg_turbo/lv_libjpeg_turbo.h
        libs/lvgl/src/libs/libpng/lv_libpng.c
        libs/lvgl/src/libs/libpng/lv_libpng.h
        libs/lvgl/src/libs/lodepng/lodepng.c
        libs/lvgl/src/libs/lodepng/lodepng.h
        libs/lvgl/src/libs/lodepng/lv_lodepng.c
        libs/lvgl/src/libs/lodepng/lv_lodepng.h
        libs/lvgl/src/libs/lz4/lz4.c
        libs/lvgl/src/libs/lz4/lz4.h
        libs/lvgl/src/libs/qrcode/lv_qrcode.c
        libs/lvgl/src/libs/qrcode/lv_qrcode.h
        libs/lvgl/src/libs/qrcode/qrcodegen.c
        libs/lvgl/src/libs/qrcode/qrcodegen.h
        libs/lvgl/src/libs/rle/lv_rle.c
        libs/lvgl/src/libs/rle/lv_rle.h
        libs/lvgl/src/libs/rlottie/lv_rlottie.c
        libs/lvgl/src/libs/rlottie/lv_rlottie.h
        libs/lvgl/src/libs/thorvg/config.h
        libs/lvgl/src/libs/thorvg/thorvg.h
        libs/lvgl/src/libs/thorvg/thorvg_capi.h
        libs/lvgl/src/libs/thorvg/tvgArray.h
        libs/lvgl/src/libs/thorvg/tvgBezier.h
        libs/lvgl/src/libs/thorvg/tvgCanvas.h
        libs/lvgl/src/libs/thorvg/tvgCommon.h
        libs/lvgl/src/libs/thorvg/tvgCompressor.h
        libs/lvgl/src/libs/thorvg/tvgFill.h
        libs/lvgl/src/libs/thorvg/tvgFrameModule.h
        libs/lvgl/src/libs/thorvg/tvgIteratorAccessor.h
        libs/lvgl/src/libs/thorvg/tvgLoadModule.h
        libs/lvgl/src/libs/thorvg/tvgLoader.h
        libs/lvgl/src/libs/thorvg/tvgMath.h
        libs/lvgl/src/libs/thorvg/tvgPaint.h
        libs/lvgl/src/libs/thorvg/tvgPicture.h
        libs/lvgl/src/libs/thorvg/tvgRawLoader.h
        libs/lvgl/src/libs/thorvg/tvgRender.h
        libs/lvgl/src/libs/thorvg/tvgSaveModule.h
        libs/lvgl/src/libs/thorvg/tvgScene.h
        libs/lvgl/src/libs/thorvg/tvgShape.h
        libs/lvgl/src/libs/thorvg/tvgStr.h
        libs/lvgl/src/libs/thorvg/tvgSvgCssStyle.h
        libs/lvgl/src/libs/thorvg/tvgSvgLoader.h
        libs/lvgl/src/libs/thorvg/tvgSvgLoaderCommon.h
        libs/lvgl/src/libs/thorvg/tvgSvgPath.h
        libs/lvgl/src/libs/thorvg/tvgSvgSceneBuilder.h
        libs/lvgl/src/libs/thorvg/tvgSvgUtil.h
        libs/lvgl/src/libs/thorvg/tvgSwCommon.h
        libs/lvgl/src/libs/thorvg/tvgSwRasterAvx.h
        libs/lvgl/src/libs/thorvg/tvgSwRasterC.h
        libs/lvgl/src/libs/thorvg/tvgSwRasterNeon.h
        libs/lvgl/src/libs/thorvg/tvgSwRasterTexmap.h
        libs/lvgl/src/libs/thorvg/tvgSwRenderer.h
        libs/lvgl/src/libs/thorvg/tvgTaskScheduler.h
        libs/lvgl/src/libs/thorvg/tvgXmlParser.h
        libs/lvgl/src/libs/tiny_ttf/lv_tiny_ttf.c
        libs/lvgl/src/libs/tiny_ttf/lv_tiny_ttf.h
        libs/lvgl/src/libs/tiny_ttf/stb_rect_pack.h
        libs/lvgl/src/libs/tiny_ttf/stb_truetype_htcw.h
        libs/lvgl/src/libs/tjpgd/lv_tjpgd.c
        libs/lvgl/src/libs/tjpgd/lv_tjpgd.h
        libs/lvgl/src/libs/tjpgd/tjpgd.c
        libs/lvgl/src/libs/tjpgd/tjpgd.h
        libs/lvgl/src/libs/tjpgd/tjpgdcnf.h
        libs/lvgl/src/lv_api_map_v8.h
        libs/lvgl/src/lv_api_map_v9_0.h
        libs/lvgl/src/lv_conf_internal.h
        libs/lvgl/src/lv_conf_kconfig.h
        libs/lvgl/src/lv_init.c
        libs/lvgl/src/lv_init.h
        libs/lvgl/src/lvgl.h
        libs/lvgl/src/lvgl_private.h
        libs/lvgl/src/misc/cache/_lv_cache_lru_rb.c
        libs/lvgl/src/misc/cache/_lv_cache_lru_rb.h
        libs/lvgl/src/misc/cache/lv_cache.c
        libs/lvgl/src/misc/cache/lv_cache.h
        libs/lvgl/src/misc/cache/lv_cache_entry.c
        libs/lvgl/src/misc/cache/lv_cache_entry.h
        libs/lvgl/src/misc/cache/lv_cache_entry_private.h
        libs/lvgl/src/misc/cache/lv_cache_private.h
        libs/lvgl/src/misc/cache/lv_image_cache.c
        libs/lvgl/src/misc/cache/lv_image_cache.h
        libs/lvgl/src/misc/lv_anim.c
        libs/lvgl/src/misc/lv_anim.h
        libs/lvgl/src/misc/lv_anim_timeline.c
        libs/lvgl/src/misc/lv_anim_timeline.h
        libs/lvgl/src/misc/lv_area.c
        libs/lvgl/src/misc/lv_area.h
        libs/lvgl/src/misc/lv_array.c
        libs/lvgl/src/misc/lv_array.h
        libs/lvgl/src/misc/lv_assert.h
        libs/lvgl/src/misc/lv_async.c
        libs/lvgl/src/misc/lv_async.h
        libs/lvgl/src/misc/lv_bidi.c
        libs/lvgl/src/misc/lv_bidi.h
        libs/lvgl/src/misc/lv_color.c
        libs/lvgl/src/misc/lv_color.h
        libs/lvgl/src/misc/lv_color_op.c
        libs/lvgl/src/misc/lv_color_op.h
        libs/lvgl/src/misc/lv_event.c
        libs/lvgl/src/misc/lv_event.h
        libs/lvgl/src/misc/lv_fs.c
        libs/lvgl/src/misc/lv_fs.h
        libs/lvgl/src/misc/lv_ll.c
        libs/lvgl/src/misc/lv_ll.h
        libs/lvgl/src/misc/lv_log.c
        libs/lvgl/src/misc/lv_log.h
        libs/lvgl/src/misc/lv_lru.c
        libs/lvgl/src/misc/lv_lru.h
        libs/lvgl/src/misc/lv_math.c
        libs/lvgl/src/misc/lv_math.h
        libs/lvgl/src/misc/lv_palette.c
        libs/lvgl/src/misc/lv_palette.h
        libs/lvgl/src/misc/lv_profiler.h
        libs/lvgl/src/misc/lv_profiler_builtin.c
        libs/lvgl/src/misc/lv_profiler_builtin.h
        libs/lvgl/src/misc/lv_rb.c
        libs/lvgl/src/misc/lv_rb.h
        libs/lvgl/src/misc/lv_style.c
        libs/lvgl/src/misc/lv_style.h
        libs/lvgl/src/misc/lv_style_gen.c
        libs/lvgl/src/misc/lv_style_gen.h
        libs/lvgl/src/misc/lv_templ.c
        libs/lvgl/src/misc/lv_templ.h
        libs/lvgl/src/misc/lv_text.c
        libs/lvgl/src/misc/lv_text.h
        libs/lvgl/src/misc/lv_text_ap.c
        libs/lvgl/src/misc/lv_text_ap.h
        libs/lvgl/src/misc/lv_timer.c
        libs/lvgl/src/misc/lv_timer.h
        libs/lvgl/src/misc/lv_types.h
        libs/lvgl/src/misc/lv_utils.c
        libs/lvgl/src/misc/lv_utils.h
        libs/lvgl/src/osal/lv_cmsis_rtos2.c
        libs/lvgl/src/osal/lv_cmsis_rtos2.h
        libs/lvgl/src/osal/lv_freertos.c
        libs/lvgl/src/osal/lv_freertos.h
        libs/lvgl/src/osal/lv_os.h
        libs/lvgl/src/osal/lv_os_none.c
        libs/lvgl/src/osal/lv_os_none.h
        libs/lvgl/src/osal/lv_pthread.c
        libs/lvgl/src/osal/lv_pthread.h
        libs/lvgl/src/osal/lv_rtthread.c
        libs/lvgl/src/osal/lv_rtthread.h
        libs/lvgl/src/osal/lv_windows.c
        libs/lvgl/src/osal/lv_windows.h
        libs/lvgl/src/others/file_explorer/lv_file_explorer.c
        libs/lvgl/src/others/file_explorer/lv_file_explorer.h
        libs/lvgl/src/others/fragment/lv_fragment.c
        libs/lvgl/src/others/fragment/lv_fragment.h
        libs/lvgl/src/others/fragment/lv_fragment_manager.c
        libs/lvgl/src/others/gridnav/lv_gridnav.c
        libs/lvgl/src/others/gridnav/lv_gridnav.h
        libs/lvgl/src/others/ime/lv_ime_pinyin.c
        libs/lvgl/src/others/ime/lv_ime_pinyin.h
        libs/lvgl/src/others/imgfont/lv_imgfont.c
        libs/lvgl/src/others/imgfont/lv_imgfont.h
        libs/lvgl/src/others/monkey/lv_monkey.c
        libs/lvgl/src/others/monkey/lv_monkey.h
        libs/lvgl/src/others/observer/lv_observer.c
        libs/lvgl/src/others/observer/lv_observer.h
        libs/lvgl/src/others/snapshot/lv_snapshot.c
        libs/lvgl/src/others/snapshot/lv_snapshot.h
        libs/lvgl/src/others/sysmon/lv_sysmon.c
        libs/lvgl/src/others/sysmon/lv_sysmon.h
        libs/lvgl/src/others/vg_lite_tvg/vg_lite.h
        libs/lvgl/src/others/vg_lite_tvg/vg_lite_matrix.c
        libs/lvgl/src/stdlib/builtin/lv_mem_core_builtin.c
        libs/lvgl/src/stdlib/builtin/lv_sprintf_builtin.c
        libs/lvgl/src/stdlib/builtin/lv_string_builtin.c
        libs/lvgl/src/stdlib/builtin/lv_tlsf.c
        libs/lvgl/src/stdlib/builtin/lv_tlsf.h
        libs/lvgl/src/stdlib/clib/lv_mem_core_clib.c
        libs/lvgl/src/stdlib/clib/lv_sprintf_clib.c
        libs/lvgl/src/stdlib/clib/lv_string_clib.c
        libs/lvgl/src/stdlib/lv_mem.c
        libs/lvgl/src/stdlib/lv_mem.h
        libs/lvgl/src/stdlib/lv_sprintf.h
        libs/lvgl/src/stdlib/lv_string.h
        libs/lvgl/src/stdlib/micropython/lv_mem_core_micropython.c
        libs/lvgl/src/stdlib/rtthread/lv_mem_core_rtthread.c
        libs/lvgl/src/stdlib/rtthread/lv_sprintf_rtthread.c
        libs/lvgl/src/stdlib/rtthread/lv_string_rtthread.c
        libs/lvgl/src/themes/default/lv_theme_default.c
        libs/lvgl/src/themes/default/lv_theme_default.h
        libs/lvgl/src/themes/lv_theme.c
        libs/lvgl/src/themes/lv_theme.h
        libs/lvgl/src/themes/mono/lv_theme_mono.c
        libs/lvgl/src/themes/mono/lv_theme_mono.h
        libs/lvgl/src/themes/simple/lv_theme_simple.c
        libs/lvgl/src/themes/simple/lv_theme_simple.h
        libs/lvgl/src/tick/lv_tick.c
        libs/lvgl/src/tick/lv_tick.h
        libs/lvgl/src/widgets/animimage/lv_animimage.c
        libs/lvgl/src/widgets/animimage/lv_animimage.h
        libs/lvgl/src/widgets/arc/lv_arc.c
        libs/lvgl/src/widgets/arc/lv_arc.h
        libs/lvgl/src/widgets/bar/lv_bar.c
        libs/lvgl/src/widgets/bar/lv_bar.h
        libs/lvgl/src/widgets/button/lv_button.c
        libs/lvgl/src/widgets/button/lv_button.h
        libs/lvgl/src/widgets/buttonmatrix/lv_buttonmatrix.c
        libs/lvgl/src/widgets/buttonmatrix/lv_buttonmatrix.h
        libs/lvgl/src/widgets/calendar/lv_calendar.c
        libs/lvgl/src/widgets/calendar/lv_calendar.h
        libs/lvgl/src/widgets/calendar/lv_calendar_header_arrow.c
        libs/lvgl/src/widgets/calendar/lv_calendar_header_arrow.h
        libs/lvgl/src/widgets/calendar/lv_calendar_header_dropdown.c
        libs/lvgl/src/widgets/calendar/lv_calendar_header_dropdown.h
        libs/lvgl/src/widgets/canvas/lv_canvas.c
        libs/lvgl/src/widgets/canvas/lv_canvas.h
        libs/lvgl/src/widgets/chart/lv_chart.c
        libs/lvgl/src/widgets/chart/lv_chart.h
        libs/lvgl/src/widgets/checkbox/lv_checkbox.c
        libs/lvgl/src/widgets/checkbox/lv_checkbox.h
        libs/lvgl/src/widgets/dropdown/lv_dropdown.c
        libs/lvgl/src/widgets/dropdown/lv_dropdown.h
        libs/lvgl/src/widgets/image/lv_image.c
        libs/lvgl/src/widgets/image/lv_image.h
        libs/lvgl/src/widgets/imagebutton/lv_imagebutton.c
        libs/lvgl/src/widgets/imagebutton/lv_imagebutton.h
        libs/lvgl/src/widgets/keyboard/lv_keyboard.c
        libs/lvgl/src/widgets/keyboard/lv_keyboard.h
        libs/lvgl/src/widgets/label/lv_label.c
        libs/lvgl/src/widgets/label/lv_label.h
        libs/lvgl/src/widgets/led/lv_led.c
        libs/lvgl/src/widgets/led/lv_led.h
        libs/lvgl/src/widgets/line/lv_line.c
        libs/lvgl/src/widgets/line/lv_line.h
        libs/lvgl/src/widgets/list/lv_list.c
        libs/lvgl/src/widgets/list/lv_list.h
        libs/lvgl/src/widgets/menu/lv_menu.c
        libs/lvgl/src/widgets/menu/lv_menu.h
        libs/lvgl/src/widgets/msgbox/lv_msgbox.c
        libs/lvgl/src/widgets/msgbox/lv_msgbox.h
        libs/lvgl/src/widgets/objx_templ/lv_objx_templ.c
        libs/lvgl/src/widgets/objx_templ/lv_objx_templ.h
        libs/lvgl/src/widgets/roller/lv_roller.c
        libs/lvgl/src/widgets/roller/lv_roller.h
        libs/lvgl/src/widgets/scale/lv_scale.c
        libs/lvgl/src/widgets/scale/lv_scale.h
        libs/lvgl/src/widgets/slider/lv_slider.c
        libs/lvgl/src/widgets/slider/lv_slider.h
        libs/lvgl/src/widgets/span/lv_span.c
        libs/lvgl/src/widgets/span/lv_span.h
        libs/lvgl/src/widgets/spinbox/lv_spinbox.c
        libs/lvgl/src/widgets/spinbox/lv_spinbox.h
        libs/lvgl/src/widgets/spinner/lv_spinner.c
        libs/lvgl/src/widgets/spinner/lv_spinner.h
        libs/lvgl/src/widgets/switch/lv_switch.c
        libs/lvgl/src/widgets/switch/lv_switch.h
        libs/lvgl/src/widgets/table/lv_table.c
        libs/lvgl/src/widgets/table/lv_table.h
        libs/lvgl/src/widgets/tabview/lv_tabview.c
        libs/lvgl/src/widgets/tabview/lv_tabview.h
        libs/lvgl/src/widgets/textarea/lv_textarea.c
        libs/lvgl/src/widgets/textarea/lv_textarea.h
        libs/lvgl/src/widgets/tileview/lv_tileview.c
        libs/lvgl/src/widgets/tileview/lv_tileview.h
        libs/lvgl/src/widgets/win/lv_win.c
        libs/lvgl/src/widgets/win/lv_win.h
        )

list(APPEND IMAGE_FILES
        displayapp/icons/battery/batteryicon.c
        )
list(APPEND SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c
        BootloaderVersion.cpp
        logging/NrfLogger.cpp
        displayapp/DisplayApp.cpp
        displayapp/screens/Screen.cpp
        displayapp/screens/Tile.cpp
        displayapp/screens/InfiniPaint.cpp
        displayapp/screens/Paddle.cpp
        displayapp/screens/StopWatch.cpp
        displayapp/screens/BatteryIcon.cpp
        displayapp/screens/BleIcon.cpp
        displayapp/screens/NotificationIcon.cpp
        displayapp/screens/SystemInfo.cpp
        displayapp/screens/Label.cpp
        displayapp/screens/FirmwareUpdate.cpp
        displayapp/screens/Music.cpp
        displayapp/screens/Navigation.cpp
        displayapp/screens/Metronome.cpp
        displayapp/screens/Motion.cpp
        displayapp/screens/Weather.cpp
        displayapp/screens/FirmwareValidation.cpp
        displayapp/screens/ApplicationList.cpp
        displayapp/screens/Notifications.cpp
        displayapp/screens/Twos.cpp
        displayapp/screens/HeartRate.cpp
        displayapp/screens/FlashLight.cpp
        displayapp/screens/List.cpp
        displayapp/screens/CheckboxList.cpp
        displayapp/screens/BatteryInfo.cpp
        displayapp/screens/Steps.cpp
        displayapp/screens/Timer.cpp
        displayapp/screens/Dice.cpp
        displayapp/screens/PassKey.cpp
        displayapp/screens/Error.cpp
        displayapp/screens/Alarm.cpp
        displayapp/screens/Styles.cpp
        displayapp/screens/WeatherSymbols.cpp
        displayapp/Colors.cpp
        displayapp/widgets/Counter.cpp
        displayapp/widgets/PageIndicator.cpp
        displayapp/widgets/DotIndicator.cpp
        displayapp/widgets/StatusIcons.cpp

        ## Settings
        displayapp/screens/settings/QuickSettings.cpp
        displayapp/screens/settings/Settings.cpp
        displayapp/screens/settings/SettingWatchFace.cpp
        displayapp/screens/settings/SettingTimeFormat.cpp
        displayapp/screens/settings/SettingWeatherFormat.cpp
        displayapp/screens/settings/SettingWakeUp.cpp
        displayapp/screens/settings/SettingDisplay.cpp
        displayapp/screens/settings/SettingSteps.cpp
        displayapp/screens/settings/SettingSetDateTime.cpp
        displayapp/screens/settings/SettingSetDate.cpp
        displayapp/screens/settings/SettingSetTime.cpp
        displayapp/screens/settings/SettingChimes.cpp
        displayapp/screens/settings/SettingShakeThreshold.cpp
        displayapp/screens/settings/SettingBluetooth.cpp

        ## Watch faces
        displayapp/screens/WatchFaceAnalog.cpp
        displayapp/screens/WatchFaceDigital.cpp
        displayapp/screens/WatchFaceInfineat.cpp
        displayapp/screens/WatchFaceTerminal.cpp
        displayapp/screens/WatchFacePineTimeStyle.cpp
        displayapp/screens/WatchFaceCasioStyleG7710.cpp

        ##

        main.cpp
        drivers/St7789.cpp
        drivers/SpiNorFlash.cpp
        drivers/SpiMaster.cpp
        drivers/Spi.cpp
        drivers/Watchdog.cpp
        drivers/InternalFlash.cpp
        drivers/Hrs3300.cpp
        drivers/Bma421.cpp
        drivers/Bma421_C/bma4.c
        drivers/Bma421_C/bma423.c
        components/battery/BatteryController.cpp
        components/ble/BleController.cpp
        components/ble/NotificationManager.cpp
        components/datetime/DateTimeController.cpp
        components/brightness/BrightnessController.cpp
        components/motion/MotionController.cpp
        components/ble/NimbleController.cpp
        components/ble/DeviceInformationService.cpp
        components/ble/CurrentTimeClient.cpp
        components/ble/AlertNotificationClient.cpp
        components/ble/DfuService.cpp
        components/ble/CurrentTimeService.cpp
        components/ble/AlertNotificationService.cpp
        components/ble/MusicService.cpp
        components/ble/SimpleWeatherService.cpp
        components/ble/NavigationService.cpp
        components/ble/BatteryInformationService.cpp
        components/ble/FSService.cpp
        components/ble/ImmediateAlertService.cpp
        components/ble/ServiceDiscovery.cpp
        components/ble/HeartRateService.cpp
        components/ble/MotionService.cpp
        components/firmwarevalidator/FirmwareValidator.cpp
        components/motor/MotorController.cpp
        components/settings/Settings.cpp
        components/timer/Timer.cpp
        components/alarm/AlarmController.cpp
        components/fs/FS.cpp
        drivers/Cst816s.cpp
        FreeRTOS/port.c
        FreeRTOS/port_cmsis_systick.c
        FreeRTOS/port_cmsis.c

        displayapp/LittleVgl.cpp
        displayapp/InfiniTimeTheme.cpp

        systemtask/SystemTask.cpp
        systemtask/SystemMonitor.cpp
        drivers/TwiMaster.cpp

        heartratetask/HeartRateTask.cpp
        components/heartrate/HeartRateController.cpp
        components/heartrate/Ppg.cpp

        buttonhandler/ButtonHandler.cpp
        touchhandler/TouchHandler.cpp

        utility/Math.cpp
        )

list(APPEND RECOVERY_SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c

        BootloaderVersion.cpp
        logging/NrfLogger.cpp
        displayapp/DisplayAppRecovery.cpp

        main.cpp
        drivers/St7789.cpp
        drivers/SpiNorFlash.cpp
        drivers/SpiMaster.cpp
        drivers/Spi.cpp
        drivers/Watchdog.cpp
        drivers/InternalFlash.cpp
        drivers/Hrs3300.cpp
        drivers/Bma421.cpp
        drivers/Bma421_C/bma4.c
        drivers/Bma421_C/bma423.c
        components/battery/BatteryController.cpp
        components/ble/BleController.cpp
        components/ble/NotificationManager.cpp
        components/datetime/DateTimeController.cpp
        components/brightness/BrightnessController.cpp
        components/motion/MotionController.cpp
        components/ble/NimbleController.cpp
        components/ble/DeviceInformationService.cpp
        components/ble/CurrentTimeClient.cpp
        components/ble/AlertNotificationClient.cpp
        components/ble/DfuService.cpp
        components/ble/CurrentTimeService.cpp
        components/ble/AlertNotificationService.cpp
        components/ble/MusicService.cpp
        components/ble/SimpleWeatherService.cpp
        components/ble/BatteryInformationService.cpp
        components/ble/FSService.cpp
        components/ble/ImmediateAlertService.cpp
        components/ble/ServiceDiscovery.cpp
        components/ble/NavigationService.cpp
        components/ble/HeartRateService.cpp
        components/ble/MotionService.cpp
        components/firmwarevalidator/FirmwareValidator.cpp
        components/settings/Settings.cpp
        components/timer/Timer.cpp
        components/alarm/AlarmController.cpp
        drivers/Cst816s.cpp
        FreeRTOS/port.c
        FreeRTOS/port_cmsis_systick.c
        FreeRTOS/port_cmsis.c

        systemtask/SystemTask.cpp
        systemtask/SystemMonitor.cpp
        drivers/TwiMaster.cpp
        components/rle/RleDecoder.cpp
        components/heartrate/HeartRateController.cpp
        heartratetask/HeartRateTask.cpp
        components/heartrate/Ppg.cpp

        components/motor/MotorController.cpp
        components/fs/FS.cpp
        buttonhandler/ButtonHandler.cpp
        touchhandler/TouchHandler.cpp

        utility/Math.cpp
        )

list(APPEND RECOVERYLOADER_SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c

        # FreeRTOS
        FreeRTOS/port.c
        FreeRTOS/port_cmsis_systick.c
        FreeRTOS/port_cmsis.c

        drivers/SpiNorFlash.cpp
        drivers/SpiMaster.cpp
        drivers/Spi.cpp
        logging/NrfLogger.cpp

        components/rle/RleDecoder.cpp

        drivers/St7789.cpp
        components/brightness/BrightnessController.cpp

        recoveryLoader.cpp
        )


set(INCLUDE_FILES

        ${CMAKE_CURRENT_BINARY_DIR}/src/Version.h
        BootloaderVersion.h
        logging/Logger.h
        logging/NrfLogger.h
        displayapp/DisplayApp.h
        displayapp/Messages.h
        displayapp/TouchEvents.h
        displayapp/screens/Screen.h
        displayapp/screens/Tile.h
        displayapp/screens/InfiniPaint.h
        displayapp/screens/StopWatch.h
        displayapp/screens/Paddle.h
        displayapp/screens/BatteryIcon.h
        displayapp/screens/BleIcon.h
        displayapp/screens/NotificationIcon.h
        displayapp/screens/SystemInfo.h
        displayapp/screens/ScreenList.h
        displayapp/screens/Label.h
        displayapp/screens/FirmwareUpdate.h
        displayapp/screens/FirmwareValidation.h
        displayapp/screens/ApplicationList.h
        displayapp/screens/CheckboxList.h
        displayapp/Apps.h
        displayapp/screens/Notifications.h
        displayapp/screens/HeartRate.h
        displayapp/screens/Metronome.h
        displayapp/screens/Motion.h
        displayapp/screens/Timer.h
        displayapp/screens/Dice.h
        displayapp/screens/Alarm.h
        displayapp/Colors.h
        displayapp/widgets/Counter.h
        displayapp/widgets/PageIndicator.h
        displayapp/widgets/DotIndicator.h
        displayapp/widgets/StatusIcons.h
        drivers/St7789.h
        drivers/SpiNorFlash.h
        drivers/SpiMaster.h
        drivers/Spi.h
        drivers/Watchdog.h
        drivers/InternalFlash.h
        drivers/Hrs3300.h
        drivers/PinMap.h
        drivers/Bma421.h
        drivers/Bma421_C/bma4.c
        drivers/Bma421_C/bma423.c
        components/battery/BatteryController.h
        components/ble/BleController.h
        components/ble/NotificationManager.h
        components/datetime/DateTimeController.h
        components/brightness/BrightnessController.h
        components/motion/MotionController.h
        components/firmwarevalidator/FirmwareValidator.h
        components/ble/BleController.h
        components/ble/NotificationManager.h
        components/ble/NimbleController.h
        components/ble/DeviceInformationService.h
        components/ble/CurrentTimeClient.h
        components/ble/AlertNotificationClient.h
        components/ble/DfuService.h
        components/firmwarevalidator/FirmwareValidator.h
        components/ble/BatteryInformationService.h
        components/ble/FSService.h
        components/ble/ImmediateAlertService.h
        components/ble/ServiceDiscovery.h
        components/ble/BleClient.h
        components/ble/HeartRateService.h
        components/ble/MotionService.h
        components/ble/SimpleWeatherService.h
        components/settings/Settings.h
        components/timer/Timer.h
        components/alarm/AlarmController.h
        drivers/Cst816s.h
        FreeRTOS/portmacro.h
        FreeRTOS/portmacro_cmsis.h
        displayapp/LittleVgl.h
        displayapp/InfiniTimeTheme.h
        systemtask/SystemTask.h
        systemtask/SystemMonitor.h
        displayapp/screens/Symbols.h
        drivers/TwiMaster.h
        heartratetask/HeartRateTask.h
        components/heartrate/Ppg.h
        components/heartrate/HeartRateController.h
        libs/arduinoFFT/src/arduinoFFT.h
        libs/arduinoFFT/src/defs.h
        libs/arduinoFFT/src/types.h
        components/motor/MotorController.h
        buttonhandler/ButtonHandler.h
        touchhandler/TouchHandler.h
        utility/Math.h
        )

include_directories(
        ${CMAKE_BINARY_DIR}/src # include generated files like Version.h
        .
        ../
)
include_directories(SYSTEM
        libs/
        FreeRTOS/
        libs/mynewt-nimble/porting/npl/freertos/include
        libs/mynewt-nimble/nimble/include
        libs/mynewt-nimble/porting/nimble/include
        libs/mynewt-nimble/nimble/host/include
        libs/mynewt-nimble/nimble/controller/include
        libs/mynewt-nimble/nimble/transport/ram/include
        libs/mynewt-nimble/nimble/drivers/nrf52/include
        libs/mynewt-nimble/ext/tinycrypt/include
        libs/mynewt-nimble/nimble/host/services/gap/include
        libs/mynewt-nimble/nimble/host/services/gatt/include
        libs/mynewt-nimble/nimble/host/util/include
        libs/mynewt-nimble/nimble/host/store/ram/include

        "${NRF5_SDK_PATH}/components/drivers_nrf/nrf_soc_nosd"
        "${NRF5_SDK_PATH}/components"
        "${NRF5_SDK_PATH}/components/boards"
        "${NRF5_SDK_PATH}/components/softdevice/common"
        "${NRF5_SDK_PATH}/integration/nrfx"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy"
        "${NRF5_SDK_PATH}/modules/nrfx"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/include"
        "${NRF5_SDK_PATH}/modules/nrfx/hal"
        "${NRF5_SDK_PATH}/modules/nrfx/mdk"
        "${NRF5_SDK_PATH}/external/freertos/source/include"
        "${NRF5_SDK_PATH}/components/toolchain/cmsis/include"
        "${NRF5_SDK_PATH}/components/libraries/atomic"
        "${NRF5_SDK_PATH}/components/libraries/atomic_fifo"
        "${NRF5_SDK_PATH}/components/libraries/atomic_flags"
        "${NRF5_SDK_PATH}/components/libraries/balloc"
        "${NRF5_SDK_PATH}/components/libraries/bootloader/ble_dfu"
        "${NRF5_SDK_PATH}/components/libraries/cli"
        "${NRF5_SDK_PATH}/components/libraries/crc16"
        "${NRF5_SDK_PATH}/components/libraries/crc32"
        "${NRF5_SDK_PATH}/components/libraries/crypto"
        "${NRF5_SDK_PATH}/components/libraries/csense"
        "${NRF5_SDK_PATH}/components/libraries/csense_drv"
        "${NRF5_SDK_PATH}/components/libraries/delay"
        "${NRF5_SDK_PATH}/components/libraries/ecc"
        "${NRF5_SDK_PATH}/components/libraries/experimental_section_vars"
        "${NRF5_SDK_PATH}/components/libraries/experimental_task_manager"
        "${NRF5_SDK_PATH}/components/libraries/fds"
        "${NRF5_SDK_PATH}/components/libraries/fstorage"
        "${NRF5_SDK_PATH}/components/libraries/gfx"
        "${NRF5_SDK_PATH}/components/libraries/gpiote"
        "${NRF5_SDK_PATH}/components/libraries/hardfault"
        "${NRF5_SDK_PATH}/components/libraries/hci"
        "${NRF5_SDK_PATH}/components/libraries/led_softblink"
        "${NRF5_SDK_PATH}/components/libraries/log"
        "${NRF5_SDK_PATH}/components/libraries/log/src"
        "${NRF5_SDK_PATH}/components/libraries/low_power_pwm"
        "${NRF5_SDK_PATH}/components/libraries/mem_manager"
        "${NRF5_SDK_PATH}/components/libraries/memobj"
        "${NRF5_SDK_PATH}/components/libraries/mpu"
        "${NRF5_SDK_PATH}/components/libraries/mutex"
        "${NRF5_SDK_PATH}/components/libraries/pwm"
        "${NRF5_SDK_PATH}/components/libraries/pwr_mgmt"
        "${NRF5_SDK_PATH}/components/libraries/queue"
        "${NRF5_SDK_PATH}/components/libraries/ringbuf"
        "${NRF5_SDK_PATH}/components/libraries/scheduler"
        "${NRF5_SDK_PATH}/components/libraries/sdcard"
        "${NRF5_SDK_PATH}/components/libraries/slip"
        "${NRF5_SDK_PATH}/components/libraries/sortlist"
        "${NRF5_SDK_PATH}/components/libraries/spi_mngr"
        "${NRF5_SDK_PATH}/components/libraries/stack_guard"
        "${NRF5_SDK_PATH}/components/libraries/strerror"
        "${NRF5_SDK_PATH}/components/libraries/svc"
        "${NRF5_SDK_PATH}/components/libraries/timer"
        "${NRF5_SDK_PATH}/components/libraries/usbd"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/audio"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc/acm"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/generic"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/kbd"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/mouse"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/msc"
        "${NRF5_SDK_PATH}/components/libraries/util"
        "${NRF5_SDK_PATH}/external/segger_rtt/"
        "${NRF5_SDK_PATH}/external/fprintf/"
        "${NRF5_SDK_PATH}/external/thedotfactory_fonts"
        "${NRF5_SDK_PATH}/components/libraries/gpiote"

)

link_directories(
        ../
)


set(COMMON_FLAGS -MP -MD -mthumb -mabi=aapcs -ftree-vrp -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin -fshort-enums -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fstack-usage -fno-exceptions -fno-non-call-exceptions)
set(WARNING_FLAGS -Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-nonliteral -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-expansion-to-defined -Wreturn-type -Werror=return-type -Werror)
set(DEBUG_FLAGS -Og -g3)
set(RELEASE_FLAGS -Os)
set(CXX_FLAGS -fno-rtti)
set(ASM_FLAGS -x assembler-with-cpp)
add_definitions(-DCONFIG_GPIO_AS_PINRESET)
add_definitions(-DNIMBLE_CFG_CONTROLLER)
add_definitions(-DOS_CPUTIME_FREQ)
add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
add_definitions(-DFREERTOS)
add_definitions(-D__STACK_SIZE=1024)
add_definitions(-D__HEAP_SIZE=0)
add_definitions(-DMYNEWT_VAL_BLE_LL_RFMGMT_ENABLE_TIME=1500)

# Note: Only use this for debugging
# Derive the low frequency clock from the main clock (SYNT)
# add_definitions(-DCLOCK_CONFIG_LF_SRC=2)

# Target hardware configuration options
add_definitions(-DTARGET_DEVICE_${TARGET_DEVICE})
add_definitions(-DTARGET_DEVICE_NAME="${TARGET_DEVICE}")
if(TARGET_DEVICE STREQUAL "PINETIME")
  add_definitions(-DDRIVER_PINMAP_PINETIME)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY_TFK5") # P8a
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY_TIN5") # P8a variant 2
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY_TON5") # P8b
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0) # RC
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
elseif(TARGET_DEVICE STREQUAL "MOY_UNK") # P8b mirrored
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0) # RC
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
  add_definitions(-DDRIVER_DISPLAY_MIRROR)
else()
  message(FATAL_ERROR "Invalid TARGET_DEVICE")
endif()

# Debug configuration
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DDEBUG)
  add_definitions(-DDEBUG_NRF_USER)

  # NRF SDK Logging
  add_definitions(-DNRF_LOG_ENABLED=1)
  # add_definitions(-DNRF_LOG_BACKEND_RTT_ENABLED=1)
  # add_definitions(-DNRF_LOG_BACKEND_SERIAL_USES_RTT=1)

  # NRF SDK individual modules logging
  # add_definitions(-DCLOCK_CONFIG_LOG_ENABLED=1)
  # add_definitions(-DCLOCK_CONFIG_LOG_LEVEL=4)
  # add_definitions(-DRTC_CONFIG_LOG_ENABLED=1)
  # add_definitions(-DRTC_CONFIG_LOG_LEVEL=4)
  
  # Nimble Logging
  add_definitions(-DMYNEWT_VAL_NEWT_FEATURE_LOGCFG=1)
  # add_definitions(-DMYNEWT_VAL_LOG_LEVEL=0)
  # add_definitions(-DMYNEWT_VAL_BLE_HS_LOG_LVL=0)
endif()

add_subdirectory(displayapp/fonts)
target_compile_options(infinitime_fonts PUBLIC
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

add_subdirectory(displayapp/apps)

# NRF SDK
add_library(nrf-sdk STATIC ${SDK_SOURCE_FILES})
target_include_directories(nrf-sdk SYSTEM PUBLIC . ../)
target_include_directories(nrf-sdk SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(nrf-sdk PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        -O3
        )

# NimBLE
add_library(nimble STATIC ${NIMBLE_SRC} ${TINYCRYPT_SRC})
target_include_directories(nimble SYSTEM PUBLIC . ../)
target_include_directories(nimble SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(nimble PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# lvgl
add_library(lvgl STATIC ${LVGL_SRC})
target_include_directories(lvgl SYSTEM PUBLIC . ../)
target_include_directories(lvgl SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(lvgl PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# LITTLEFS_SRC
add_library(littlefs STATIC ${LITTLEFS_SRC})
target_include_directories(littlefs SYSTEM PUBLIC . ../)
target_include_directories(littlefs SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(littlefs PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# Build autonomous binary (without support for bootloader)
set(EXECUTABLE_NAME "pinetime-app")
set(EXECUTABLE_FILE_NAME ${EXECUTABLE_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf52.ld")
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_FILE_NAME})
target_link_libraries(${EXECUTABLE_NAME} nimble nrf-sdk lvgl littlefs infinitime_fonts infinitime_apps)
target_compile_options(${EXECUTABLE_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_FILE_NAME}")

if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_NAME} GenerateResources)
endif()

# Build binary intended to be used by bootloader
set(EXECUTABLE_MCUBOOT_NAME "pinetime-mcuboot-app")
set(EXECUTABLE_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_FILE_NAME_HEX ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.hex)
set(IMAGE_MCUBOOT_FILE_NAME_BIN ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.bin)
set(DFU_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
set(NRF5_LINKER_SCRIPT_MCUBOOT "${CMAKE_SOURCE_DIR}/gcc_nrf52-mcuboot.ld")
add_executable(${EXECUTABLE_MCUBOOT_NAME} ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE_MCUBOOT_NAME} nimble nrf-sdk lvgl littlefs infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_MCUBOOT_FILE_NAME})
target_compile_options(${EXECUTABLE_MCUBOOT_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.hex ${IMAGE_MCUBOOT_FILE_NAME_HEX}
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.bin ${IMAGE_MCUBOOT_FILE_NAME_BIN}
        COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
        )

if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_MCUBOOT_NAME} GenerateResources)
endif()

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_MCUBOOT_FILE_NAME_HEX} ${DFU_MCUBOOT_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
          )
endif()

# InfiniTime recovery firmware (autonomous)
set(EXECUTABLE_RECOVERY_NAME "pinetime-recovery")
set(EXECUTABLE_RECOVERY_FILE_NAME ${EXECUTABLE_RECOVERY_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
add_executable(${EXECUTABLE_RECOVERY_NAME} ${RECOVERY_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERY_NAME} nimble nrf-sdk littlefs infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_RECOVERY_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERY_FILE_NAME})
target_compile_definitions(${EXECUTABLE_RECOVERY_NAME} PUBLIC "PINETIME_IS_RECOVERY")
target_compile_options(${EXECUTABLE_RECOVERY_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_RECOVERY_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_RECOVERY_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERY_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERY_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERY_FILE_NAME}.out "${EXECUTABLE_RECOVERY_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERY_FILE_NAME}.out "${EXECUTABLE_RECOVERY_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_RECOVERY_FILE_NAME}"
        )

# InfiniTime recovery firmware (mcuboot)
set(EXECUTABLE_RECOVERY_MCUBOOT_NAME "pinetime-mcuboot-recovery")
set(EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.hex)
set(DFU_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
add_executable(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} ${RECOVERY_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} nimble nrf-sdk littlefs infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME})
target_compile_definitions(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PUBLIC "PINETIME_IS_RECOVERY")
target_compile_options(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_GRAPHICS_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.hex ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX}
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX} "${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.bin"
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/bin2c.py ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.bin recoveryImage > recoveryImage.h
        COMMENT "post build steps for ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}"
        )

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX} ${DFU_RECOVERY_MCUBOOT_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}"
          )
endif()

# Build binary that writes the recovery image into the SPI flash memory
set(EXECUTABLE_RECOVERYLOADER_NAME "pinetime-recovery-loader")
set(EXECUTABLE_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_RECOVERYLOADER_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
add_executable(${EXECUTABLE_RECOVERYLOADER_NAME} ${RECOVERYLOADER_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERYLOADER_NAME} nrf-sdk infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_RECOVERYLOADER_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERYLOADER_FILE_NAME})
target_compile_options(${EXECUTABLE_RECOVERYLOADER_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )
target_include_directories(${EXECUTABLE_RECOVERYLOADER_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
        )
add_dependencies(${EXECUTABLE_RECOVERYLOADER_NAME} ${EXECUTABLE_RECOVERY_MCUBOOT_NAME})

set_target_properties(${EXECUTABLE_RECOVERYLOADER_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERYLOADER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}"
        )

# Build binary that writes the recovery image (MCUBoot version)
set(EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME "pinetime-mcuboot-recovery-loader")
set(EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex)
set(DFU_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
add_executable(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} ${RECOVERYLOADER_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} nrf-sdk infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME})
target_compile_options(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )
target_include_directories(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
        )
add_dependencies(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} ${EXECUTABLE_RECOVERY_MCUBOOT_NAME})

set_target_properties(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX}
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX} "${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/bin2c.py ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin recoveryLoaderImage > recoveryLoaderImage.h
        COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}"
        )

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX} ${DFU_MCUBOOT_RECOVERYLOADER_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}"
          )
endif()

if(BUILD_RESOURCES)
  add_subdirectory(resources)
endif()

