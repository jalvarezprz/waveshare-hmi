#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "lvgl_lock_shim.h"

#include "ui/theme/ui_theme_c_api.h"
#include "ui/layout/ui_layout_scaffold.h"
#include "ui/ui_router.h"
#include "ui/ui_router_mount.h"         // ← NUEVO

extern "C" void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    lvgl_port_lock(UINT32_MAX);

    ui_theme_init_once_c();

    Ui::UiLayoutScaffold scaffold;
    scaffold.setBarsHeight(56, 48);
    scaffold.build(lv_scr_act());

    // ← indicar al router que monte sus vistas en el Content del scaffold
    ui_router_mount_set(scaffold.content());

    // lanzar la vista inicial del router
    ui_router_go(UiScreen::MAIN_MENU);

    lvgl_port_unlock();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
