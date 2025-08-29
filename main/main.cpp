#include "lvgl_port.h"          // <— en vez de example_*
#include "waveshare_rgb_lcd_port.h"
#include "ui/ui_screens.h"
#include "lvgl_lock_shim.h"

#include "ui/ui_menu_mcr50.h"

extern "C" void app_main(void)
{
    
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    lvgl_port_lock(UINT32_MAX);   // bloquea (equivalente a -1)

    // ui_init_example();
    
    printMenuMcr50();              // logs
    ui_mcr50_build_main_menu();    // pinta la lista en pantalla

    lvgl_port_unlock();

    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
}
