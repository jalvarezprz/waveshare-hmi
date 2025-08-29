#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "ui/ui_screens.h"
#include "lvgl_lock_shim.h"

#include "ui/ui_router.h"        // <-- nuevo
// #include "ui/ui_menu_mcr50.h" // ya no hace falta aquí

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    lvgl_port_lock(UINT32_MAX);
    // ui_init_example();                 // (opcional) lo quitamos para ver el router
    ui_router_go(UiScreen::MAIN_MENU);     // <-- navegar a la pantalla inicial
    lvgl_port_unlock();

    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
}
