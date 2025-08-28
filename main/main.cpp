#include "lvgl_port.h"          // <— en vez de example_*
#include "waveshare_rgb_lcd_port.h"
#include "ui/ui_screens.h"
#include "lvgl_lock_shim.h"

#include "ui/ui_menu_mcr50.h"

extern "C" void app_main(void)
{
    /*
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    lvgl_port_lock(UINT32_MAX);   // bloquea (equivalente a -1)
    ui_init_example();
    lvgl_port_unlock();
    */
    
    ESP_LOGI("MAIN", "Arrancando HMI con menús MCR50...");
    // Depuración: imprime los menús embebidos
    printMenuMcr50();

    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
}
