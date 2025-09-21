#include "hmi_command_sender.h"

#include <cstring>
#include <string>
#include "esp_log.h"

#include "comm/tx/comm_tx_queue.h"   // comm_tx_queue_send(...)
#include "comm/comm_proto_v1.h"      // AppEnvelope, AppV1::JsonBridge
#include "comm/command_contract.h"   // Comm::CommandContract::make_command_do(...)

namespace Hmi::CommandSender {

static const char* TAG = "HMI_CMD_SENDER";

// Callback opcional (si alguien quiere sobreescribir el envío)
static TxFn s_tx = nullptr;

// Encola un JSON en la cola TX usando el framing v1 + JSON_BRIDGE (0x30)
static bool enqueue_via_tx_queue(const char* json, size_t len)
{
    if (!json || len == 0) return false;

    AppEnvelope env{};
    env.header.ver    = 1;
    env.header.type   = static_cast<uint8_t>(AppV1::JsonBridge); // == 0x30
    env.header.seq    = 0;
    env.header.flags  = 0;
    env.header.ts10ms = 0;

    if (len > env.payload.size()) {
        ESP_LOGE(TAG, "JSON demasiado largo (%u > %u)",
                 (unsigned)len, (unsigned)env.payload.size());
        return false;
    }

    env.header.len = static_cast<uint16_t>(len);
    std::memcpy(env.payload.data(), json, len);

    const bool ok = comm_tx_queue_send(env, pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "TX enqueue JSON (%u bytes) -> %s",
             (unsigned)len, ok ? "OK" : "FAIL");
    return ok;
}

void set_tx_fn(TxFn fn)
{
    s_tx = fn;
}

// Envío “raw” de JSON (c-string)
bool send_raw_json(const char* json)
{
    if (!json) return false;
    const size_t len = std::strlen(json);

    ESP_LOGI(TAG, "TX CMD: %.*s", (int)len, json);

    // Si hay callback, se usa; si no, por la cola estándar
    if (s_tx) return s_tx(json, len);
    return enqueue_via_tx_queue(json, len);
}

// Envío “raw” de JSON (std::string)
bool send_raw_json(const std::string& j)
{
    return send_raw_json(j.c_str());
}

// Helper de alto nivel: construye y envía un “do” (command_contract)
bool send_do(const char* path, const char* op, bool expects_ack)
{
    const std::string j = Comm::CommandContract::make_command_do(path, op, expects_ack);
    ESP_LOGI(TAG, "TX DO %s %s (ack=%s)", path ? path : "-", op ? op : "-", expects_ack ? "true" : "false");
    return send_raw_json(j);
}

} // namespace Hmi::CommandSender
