#include "comm_tx_queue.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "comm_tx_queue";

// Cola de mensajes de aplicación hacia el gateway TX
static QueueHandle_t s_q = nullptr;

bool comm_tx_queue_init(size_t depth) {
    if (s_q) {
        ESP_LOGW(TAG, "init: ya estaba inicializada (handle=%p)", (void*)s_q);
        return true;
    }

    // Profundidad por defecto desde Kconfig si el caller no especifica
    if (depth == 0) {
    #ifdef CONFIG_COMM_TX_QUEUE_LEN
        depth = CONFIG_COMM_TX_QUEUE_LEN;
    #else
        depth = 16; // fallback seguro si no hay Kconfig
    #endif
    }

    s_q = xQueueCreate(depth, sizeof(AppEnvelope));
    if (!s_q) {
        ESP_LOGE(TAG, "init: xQueueCreate() FAILED (depth=%u, item=%u bytes)",
                 (unsigned)depth, (unsigned)sizeof(AppEnvelope));
        return false;
    }

    ESP_LOGI(TAG, "init: creada OK (handle=%p, depth=%u, item=%u bytes)",
             (void*)s_q, (unsigned)depth, (unsigned)sizeof(AppEnvelope));
    return true;
}

QueueHandle_t comm_tx_queue_handle() {
    return s_q;
}

bool comm_tx_queue_send(const AppEnvelope& env, TickType_t to) {
    if (!s_q) {
        ESP_LOGE(TAG, "send: cola no inicializada");
        return false;
    }
    BaseType_t rc = xQueueSend(s_q, &env, to);
    if (rc != pdTRUE) {
        ESP_LOGW(TAG, "send: cola llena o timeout (to=%u ticks)", (unsigned)to);
        return false;
    }
    return true;
}

bool comm_tx_queue_recv(AppEnvelope& env, TickType_t to) {
    if (!s_q) {
        ESP_LOGE(TAG, "recv: cola no inicializada");
        return false;
    }
    BaseType_t rc = xQueueReceive(s_q, &env, to);
    if (rc != pdTRUE) {
        // No hacemos ESP_LOGW aquí para no saturar logs si no hay tráfico
        return false;
    }
    return true;
}
