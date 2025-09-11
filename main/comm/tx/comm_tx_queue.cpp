#include "comm_tx_queue.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "comm_tx_queue";
static QueueHandle_t s_q = nullptr;

bool comm_tx_queue_init(size_t depth) {
    if (depth == 0) depth = CONFIG_COMM_TX_QUEUE_LEN;   // ← usa Kconfig
    if (!s_q) s_q = xQueueCreate(depth, sizeof(AppEnvelope));
    if (!s_q) s_q = xQueueCreate(depth, sizeof(AppEnvelope));
    bool ok = (s_q != nullptr);
    if (!ok) ESP_LOGE(TAG, "create failed");
    return ok;
}

QueueHandle_t comm_tx_queue_handle() { return s_q; }

bool comm_tx_queue_send(const AppEnvelope& env, TickType_t to) {
    return s_q ? (xQueueSend(s_q, &env, to) == pdTRUE) : false;
}

bool comm_tx_queue_recv(AppEnvelope& env, TickType_t to) {
    return s_q ? (xQueueReceive(s_q, &env, to) == pdTRUE) : false;
}
