#include "hmi_command_sender.h"

#include <cstring>
#include <string>
#include "esp_log.h"

#include "comm/tx/comm_tx_json_bridge.h"  // comm_tx_json_send(...)
#include "comm/command_contract.h"        // Comm::CommandContract::make_command_do(...)

namespace Hmi::CommandSender {

static const char* TAG = "HMI_CMD_SENDER";

// Callback opcional (si alguien quiere sobreescribir el envío)
static TxFn s_tx = nullptr;

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

    // Si hay callback, úsalo; si no, por el bridge estándar (fragmentado)
    if (s_tx) return s_tx(json, len);

    const bool ok = comm_tx_json_send(json, len);
    ESP_LOGI(TAG, "TX JSON_BRIDGE len=%u -> %s", (unsigned)len, ok ? "OK" : "FAIL");
    return ok;
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
