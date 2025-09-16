#include "comm_demo_loopback.h"
#include "comm/tx/comm_tx_api.h"
#include "comm/rx/comm_rx_queue.h"
#include "comm/rx/comm_rx_state.h"
#include "comm/comm_commitment.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "comm_demo";
static TaskHandle_t s_task{nullptr};

static void task_(void*) {
    bool on = false;
    uint8_t seq = 0;

    for (;;) {
        // 1) Enviar comando LED0 ON/OFF por la API (TX completo hasta gateway)
        on = !on;
        (void)CommTxApi::sendLed(0, on);
        ESP_LOGI(TAG, "Sent LED0 %s", on ? "ON" : "OFF");

        // 2) Simular telemetría: construir AppEnvelope y meterlo en cola RX
        AppEnvelope env{};
        env.header.ver   = APP_PROTO_VER;
        env.header.type  = static_cast<uint8_t>(AppMsgType::TelemetryDs18x);
        env.header.seq   = ++seq;
        env.header.flags = 0;
        env.header.ts10ms= 0;
        env.header.len   = sizeof(AppTelemetryPayload);

        auto* p = reinterpret_cast<AppTelemetryPayload*>(env.payload.data());
        for (int i = 0; i < 8; ++i) p->temps[i] = 2150 + i * 37;  // 21.50°C, 21.87°C, ...

        (void)comm_rx_queue_send(env); // → lo consumirá comm_rx_handler y actualizará CommRxState

        // 3) Leer desde el estado y loggear
        auto temps = CommRxState::getTempsCelsius();
        ESP_LOGI(TAG, "T[0]=%.2f T[1]=%.2f T[2]=%.2f ...", temps[0], temps[1], temps[2]);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

bool comm_demo_loopback_start() {
    if (s_task) return true;
    return xTaskCreate(task_, "comm_demo", 4096, nullptr, 6, &s_task) == pdPASS;
}
