/**
 * @file main.cpp
 * @brief Arranque HMI: inicializa panel Waveshare, LVGL + tema y lanza Router.
 */

#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "lvgl_lock_shim.h"

#include "ui/theme/ui_theme_styles.h"
#include "ui/layout/ui_layout_scaffold.h"
#include "ui/router/ui_router.h"
#include "ui/menu/ui_menu_loader.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void)
{
    // Log general
    esp_log_level_set("*", ESP_LOG_INFO);

    // Panel RGB + touch (Waveshare ESP32-S3 Touch LCD 7")
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    // Construcción de UI protegida por lock de LVGL
    lvgl_port_lock(UINT32_MAX);

    // Tema (tokens + styles)
    Ui::themeInitOnce();
    lv_obj_add_style(lv_scr_act(), &Ui::getThemeStyles().base, LV_PART_MAIN);

    ui_router_go_screen("main");

    lvgl_port_unlock();

    // Bucle mínimo (el timer/tarea de LVGL ya está activo)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
