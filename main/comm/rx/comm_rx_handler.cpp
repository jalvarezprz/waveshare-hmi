#include "comm_rx_handler.h"
#include "comm_rx_queue.h"
#include "comm_rx_state.h"
#include "comm_commitment.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "comm_rx_handler";
static TaskHandle_t s_task{nullptr};

static void task_rx_(void*) {
    AppEnvelope env{};
    for (;;) {
        if (!comm_rx_queue_recv(env, portMAX_DELAY)) continue;

        // Validación mínima
        if (env.header.ver != APP_PROTO_VER) { ESP_LOGW(TAG, "proto mismatch"); continue; }

        auto type = static_cast<AppMsgType>(env.header.type);
        switch (type) {
            case AppMsgType::TelemetryDs18x: {
                if (env.header.len >= sizeof(AppTelemetryPayload)) {
                    auto* p = reinterpret_cast<const AppTelemetryPayload*>(env.payload.data());
                    CommRxState::setTelemetryRaw(p->temps, 8, env.header.seq);
                } else {
                    ESP_LOGW(TAG, "bad len for Telemetry");
                }
            } break;
            case AppMsgType::Ack:
            case AppMsgType::Heartbeat:
            case AppMsgType::Error:
            case AppMsgType::CmdLed:
            case AppMsgType::ReqSnapshot:
            default:
                // Por ahora, solo log
                ESP_LOGI(TAG, "RX type=0x%02X seq=%u len=%u",
                         env.header.type, env.header.seq, env.header.len);
                break;
        }
    }
}

bool comm_rx_handler_start() {
    if (s_task) return true;
    return xTaskCreate(task_rx_, "comm_rx", 4096, nullptr, 7, &s_task) == pdPASS;
}
