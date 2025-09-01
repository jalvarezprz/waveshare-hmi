#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "lvgl_lock_shim.h"

#include "ui/theme/ui_theme_c_api.h"     // Init de tokens+styles (C-API)
#include "ui/layout/ui_layout_scaffold.h"
#include "ui/ui_router.h"

extern "C" void app_main(void)
{
    // Inicializa el panel RGB y LVGL (BSP Waveshare)
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    // Bloquea LVGL para operaciones de UI
    lvgl_port_lock(UINT32_MAX);

    // Inicializa el Theme (idempotente; seguro llamarlo siempre)
    ui_theme_init_once_c();

    // Construye el layout base Header/Content/Footer sobre la pantalla activa
    Ui::UiLayoutScaffold scaffold;
    scaffold.setBarsHeight(56, 48);          // (opcional) ajusta alturas Header/Footer
    scaffold.build(lv_scr_act());

    // Lanza la vista inicial mediante el router (montará la UI dentro de 'content()')
    ui_router_go(UiScreen::MAIN_MENU);

    // Desbloquea LVGL
    lvgl_port_unlock();

    // Bucle principal (mantener tarea viva)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
