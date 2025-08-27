/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "waveshare_rgb_lcd_port.h"
#include "esp_log.h"
#include "ui.h"

static const char *TAG = "main";             // en main.c

void ui_create(void);

void app_main()
{
    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG, "Display LVGL demos");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {

        ui_create();

        // lv_demo_stress();
        // lv_demo_benchmark();
        // lv_demo_music();
        // lv_demo_widgets();
        // example_lvgl_demo_ui();
        // Release the mutex
        lvgl_port_unlock();
    }
}
