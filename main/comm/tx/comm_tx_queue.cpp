#include "comm/tx/comm_tx_queue.h"
#include "esp_log.h"

static const char* TAG = "comm_tx_queue";

static QueueHandle_t s_queue = nullptr;
static constexpr size_t QUEUE_LEN = 16;   // capacidad de la cola

void comm_tx_queue_init()
{
    if (!s_queue) {
        s_queue = xQueueCreate(QUEUE_LEN, sizeof(AppV1::Envelope));
        if (!s_queue) {
            ESP_LOGE(TAG, "No se pudo crear la cola TX");
        }
    }
}

bool comm_tx_queue_send(const AppV1::Envelope& env, TickType_t to)
{
    if (!s_queue) comm_tx_queue_init();
    if (!s_queue) return false;
    return (xQueueSend(s_queue, &env, to) == pdTRUE);
}

bool comm_tx_queue_recv(AppV1::Envelope& out, TickType_t to)
{
    if (!s_queue) comm_tx_queue_init();
    if (!s_queue) return false;
    return (xQueueReceive(s_queue, &out, to) == pdTRUE);
}

size_t comm_tx_queue_level()
{
    return s_queue ? uxQueueMessagesWaiting(s_queue) : 0;
}
