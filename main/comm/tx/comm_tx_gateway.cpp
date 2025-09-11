#include "comm_tx_gateway.h"
#include "comm_tx_queue.h"
#include "comm_commitment.h"
#include "comm/rx/comm_rx_queue.h"

#include "sdkconfig.h"
#include "comm_diag.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#ifndef COMM_LOOPBACK
#define COMM_LOOPBACK CONFIG_COMM_LOOPBACK   // ← del Kconfig
#endif

static const char* TAG = "comm_tx_gateway";
static TaskHandle_t s_task{nullptr};

static void task_tx_(void*) {
    AppEnvelope env{};
    for (;;) {
        if (!comm_tx_queue_recv(env, portMAX_DELAY)) continue;
        CommDiag::incTxQueued();

#if COMM_LOOPBACK
        (void)comm_rx_queue_send(env, 0);
        CommDiag::incTxLoopback();
        if (env.header.flags & static_cast<uint8_t>(AppMsgFlags::ReqAck)) {
            AppEnvelope ack{};
            ack.header.ver = APP_PROTO_VER;
            ack.header.type = static_cast<uint8_t>(AppMsgType::Ack);
            ack.header.seq = env.header.seq;
            ack.header.flags = 0; ack.header.ts10ms = 0; ack.header.len = 0;
            (void)comm_rx_queue_send(ack, 0);
        }
        ESP_LOGI(TAG, "LOOPBACK TX→RX type=0x%02X seq=%u", env.header.type, env.header.seq);
#else
        // TODO: esp_now_send(...), y en su callback sumar ok/fail
        // if (ok) CommDiag::incTxOk(); else CommDiag::incTxFail();
        ESP_LOGI(TAG, "TX queued type=0x%02X seq=%u (driver TBD)", env.header.type, env.header.seq);
#endif
    }
}

bool comm_tx_gateway_start() {
    if (s_task) return true;
    return xTaskCreate(task_tx_, "comm_tx", 4096, nullptr, 8, &s_task) == pdPASS;
}
