#include "comm/tx/comm_tx_json_bridge.h"
#include "comm/comm_proto_v1.h"
#include "comm/tx/comm_tx_queue.h"

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "comm_tx_json_bridge";

// Máximo de datos JSON por fragmento dentro del payload v1
static constexpr uint16_t MAX_CHUNK = 200; // 240 - sizeof(app_v1_json_chunk_hdr_t)

// Secuencia de mensajes JSON (independiente del seq v1)
static uint16_t g_msg_seq = 0;

bool comm_tx_json_send(const char* json, size_t len)
{
    if (!json || len == 0) {
        ESP_LOGW(TAG, "JSON vacío o puntero nulo");
        return false;
    }

    const uint16_t msg_seq     = ++g_msg_seq;
    const uint16_t chunk_count = (len + MAX_CHUNK - 1) / MAX_CHUNK;

    ESP_LOGI(TAG, "PREP JSON len=%u -> %u fragmentos (msg_seq=%u)",
             (unsigned)len, (unsigned)chunk_count, (unsigned)msg_seq);

    uint16_t offset    = 0;
    uint16_t chunk_idx = 0;

    while (offset < len) {
        const uint16_t part_len = (len - offset > MAX_CHUNK)
                                  ? MAX_CHUNK
                                  : static_cast<uint16_t>(len - offset);

        AppV1::Envelope env{};
        env.hdr.ver    = 1;
        env.hdr.type   = AppV1::JsonBridge;          // 0x30
        env.hdr.seq    = 0;                          // lo fijará la pasarela TX
        env.hdr.flags  = 0;
        env.hdr.ts10ms = 0;

        auto* jh = reinterpret_cast<AppV1::app_v1_json_chunk_hdr_t*>(env.payload);
        jh->msg_seq     = msg_seq;
        jh->chunk_idx   = chunk_idx;
        jh->chunk_count = chunk_count;
        jh->chunk_len   = part_len;
        jh->crc16       = 0; // provisional

        // Copiamos el trozo de JSON detrás del header de fragmento
        std::memcpy(env.payload + sizeof(*jh), json + offset, part_len);

        // Calcula el CRC16 tal como lo espera el receptor
        jh->crc16 = AppV1::json_fragment_crc16(
            jh,
            reinterpret_cast<const uint8_t*>(json + offset),
            part_len
        );

        env.hdr.len = static_cast<uint16_t>(sizeof(*jh) + part_len);

        const bool ok = comm_tx_queue_send(env, pdMS_TO_TICKS(50));
        ESP_LOGI(TAG,
                 "FRAG msg_seq=%u idx=%u/%u part_len=%u crc16=0x%04X -> %s",
                 (unsigned)msg_seq,
                 (unsigned)chunk_idx,
                 (unsigned)chunk_count,
                 (unsigned)part_len,
                 (unsigned)jh->crc16,
                 ok ? "OK" : "FAIL");

        if (!ok) return false;

        offset    += part_len;
        chunk_idx += 1;
    }

    ESP_LOGI(TAG, "JSON msg_seq=%u enviado en %u fragmentos (len=%u)",
             (unsigned)msg_seq, (unsigned)chunk_idx, (unsigned)len);
    return true;
}
