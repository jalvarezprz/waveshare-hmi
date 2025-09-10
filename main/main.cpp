/**
 * @file main.cpp
 * @brief Arranque provisional para probar el mockup de menú (usar y tirar).
 *
 * Toggle:
 *   - USE_MOCKUP = 1  -> Carga la pantalla mockup (grid + 3 botones)
 *   - USE_MOCKUP = 0  -> Flujo normal (Scaffold + Router)
 */

#ifndef USE_MOCKUP
#define USE_MOCKUP 1
#endif

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
    // Log global
    esp_log_level_set("*", ESP_LOG_INFO);

    // Inicializa el panel RGB Waveshare (ST7701) y el touch (GT911) si procede
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    // Bloquea el wrapper de LVGL mientras construimos la UI
    lvgl_port_lock(UINT32_MAX);

    // Inicializa tema (tokens + styles cargados en Ui::getThemeStyles())
    Ui::themeInitOnce();

    // Aplica estilo base al screen activo para fondo/typography coherentes
    lv_obj_add_style(lv_scr_act(), &Ui::getThemeStyles().base, LV_PART_MAIN);

#if !defined(USE_MOCKUP) || (USE_MOCKUP == 0)
    // ================================================================
    // FLUJO NORMAL (Scaffold + Router)
    // ================================================================
    Ui::UiLayoutScaffold scaffold;
    scaffold.setBarsHeight(56, 48);
    scaffold.build(lv_scr_act());

    // Indicar al router que monte sus vistas en el Content del scaffold
    ui_router_mount_set(scaffold.content());

    // Lanzar la vista inicial del router
    ui_router_go(UiScreen::MAIN_MENU);
#else
    // ================================================================
    // MODO MOCKUP (usar y tirar)
    // Pantalla simple con grid y 3 botones: "Inicio", "Parámetros", "Salir".
    // Sirve para validar layout/espaciados/eventos sin pasar por Router.
    // ================================================================
    // ui_mockup_menu_load();
    ui_router_go_screen("main");
    // ui_menu_loader_run_tests();
    // ui_menu_loader_run_smoke_test();
#endif

    // Desbloquea LVGL
    lvgl_port_unlock();

    // Bucle mínimo (tu sistema ya debe tener la tarea/timer de LVGL en marcha)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
