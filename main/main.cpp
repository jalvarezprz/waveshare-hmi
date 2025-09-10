/**
 * @file main.cpp
 * @brief Arranque HMI: inicializa panel Waveshare, LVGL + tema, comunicaciones (comm) y lanza Router.
 */

#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "lvgl_lock_shim.h"

#include "ui/theme/ui_theme_styles.h"
#include "ui/router/ui_router.h"
#include "ui/menu/ui_menu_loader.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ==== COMM: headers TX/RX ====
#include "comm/comm_commitment.h"
#include "comm/tx/comm_tx_queue.h"
#include "comm/tx/comm_tx_gateway.h"
#include "comm/tx/comm_tx_api.h"
#include "comm/rx/comm_rx_queue.h"
#include "comm/rx/comm_rx_handler.h"
#include "comm/rx/comm_rx_state.h"

#include "comm/comm_demo_loopback.h"

extern "C" void app_main(void)
{
    // Log general
    esp_log_level_set("*", ESP_LOG_INFO);

    // Panel RGB + touch (Waveshare ESP32-S3 Touch LCD 7")
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    // ===== COMM: inicialización en orden =====
    // Colas TX/RX
    comm_tx_queue_init();               // profundidad por defecto (16)
    comm_rx_queue_init();
    // Estado RX y tareas
    CommRxState::init();
    comm_rx_handler_start();            // consume RX y actualiza estado
    comm_tx_gateway_start();            // lee TX y (loopback o, después, ESP-NOW)
    // API TX (si requiere estado propio)
    CommTxApi::init();

    // Demo
    comm_demo_loopback_start();

    // Construcción de UI protegida por lock de LVGL
    lvgl_port_lock(UINT32_MAX);

    // Tema (tokens + styles)
    Ui::themeInitOnce();
    lv_obj_add_style(lv_scr_act(), &Ui::getThemeStyles().base, LV_PART_MAIN);

    // Pantalla inicial del Router
    ui_router_go_screen("main");

    lvgl_port_unlock();

    // Bucle mínimo (el timer/tarea de LVGL ya está activo)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
