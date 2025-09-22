#include "comm/tx/comm_tx_api.h"
#include "comm/tx/comm_tx_queue.h"
#include "comm/tx/comm_tx_gateway.h"
#include "comm/comm_proto_v1.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace {
    const char* TAG = "comm_tx_api";
}

namespace CommTxApi {

bool init()
{
    // Arranca la pasarela de transmisión (Wi-Fi STA + ESP-NOW + tarea TX).
    // La configuración de canal y peer se hace dentro del gateway
    // (leerá CONFIG_COMM_PEER_MAC_STR si está definido).
    const bool ok = comm_tx_gateway_start();
    ESP_LOGI(TAG, "comm_tx_gateway_start() -> %s", ok ? "OK" : "FAIL");
    return ok;
}

bool sendLed(uint8_t led_idx, bool on)
{
    AppV1::Envelope env{};
    env.hdr.ver    = 1;
    env.hdr.type   = AppV1::CmdLed;
    env.hdr.seq    = 0;
    env.hdr.flags  = 0;
    env.hdr.ts10ms = 0;

    // payload mínimo: [led_idx, on_off]
    env.payload[0] = led_idx;
    env.payload[1] = on ? 1 : 0;
    env.hdr.len    = 2;

    const bool ok = comm_tx_queue_send(env, pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "sendLed(idx=%u,on=%u) -> %s", (unsigned)led_idx, (unsigned)(on?1:0), ok ? "OK" : "FAIL");
    return ok;
}

bool requestSnapshot()
{
    AppV1::Envelope env{};
    env.hdr.ver    = 1;
    env.hdr.type   = AppV1::ReqSnapshot;
    env.hdr.seq    = 0;
    env.hdr.flags  = 0;
    env.hdr.ts10ms = 0;
    env.hdr.len    = 0;

    const bool ok = comm_tx_queue_send(env, pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "requestSnapshot() -> %s", ok ? "OK" : "FAIL");
    return ok;
}

} // namespace CommTxApi
