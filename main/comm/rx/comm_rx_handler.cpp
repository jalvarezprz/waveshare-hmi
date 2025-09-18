#include "comm_rx_handler.h"
#include "comm_rx_queue.h"
#include "comm_commitment.h"
#include "comm_diag.h"

#include "comm/rx/comm_rx_state.h"
#include "comm/comm_proto_v1.h"   // contrato oficial CORE ↔ HMI (Header y app_v1_json_chunk_hdr_t)

#include "cJSON.h"

#include <map>
#include <vector>
#include <string>
#include <array>
#include <cstring>   // memcpy, strcmp
#include <cmath>     // NAN
#include <algorithm> // std::min, std::max
#include <cstdio>    // snprintf

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

using namespace AppV1;

// =====================================================================================
// Reensamblado de JSON_BRIDGE (type=0x30).
// Cada payload trae un app_v1_json_chunk_hdr_t (10 bytes) + datos JSON.
// =====================================================================================

static const char* TAG = "comm_rx_handler";
static TaskHandle_t s_task{nullptr};

struct PendingJson {
    uint16_t msg_seq = 0;
    uint16_t chunk_count = 0;
    std::vector<std::vector<uint8_t>> parts; // parts[i] = datos del chunk i
    std::vector<uint8_t> assembled;          // buffer final cuando se completa
    uint32_t received = 0;                   // nº de chunks distintos recibidos
    size_t total_bytes = 0;                  // suma de chunk_len recibidos
};

static std::map<uint16_t, PendingJson> s_pending; // clave: msg_seq

// Utils de log
static std::string hex_preview(const uint8_t* p, size_t n, size_t maxn = 24) {
    char tmp[4];
    std::string out;
    size_t m = std::min(n, maxn);
    for (size_t i = 0; i < m; ++i) {
        std::snprintf(tmp, sizeof(tmp), "%02X", (unsigned)p[i]);
        if (i) out.push_back(' ');
        out += tmp;
    }
    if (n > m) out += " ...";
    return out;
}

static void log_json_preview(const char* prefix, const std::string& s) {
    const size_t N = 120;
    std::string pv = s.substr(0, N);
    ESP_LOGI(TAG, "%s %.120s%s", prefix, pv.c_str(), (s.size() > pv.size() ? " ..." : ""));
}

// Parseo específico del batch de temperaturas
// Parseo específico del batch de temperaturas
static void parse_and_publish(const std::string& json_str, uint16_t msg_seq)
{
    if (json_str.empty()) return;

    // Sanidad rápida
    if (!(json_str[0] == '{' || json_str[0] == '[')) {
        ESP_LOGW(TAG, "JSON (msg=%u) no empieza por '{'/'[' (0x%02X)",
                 msg_seq, (unsigned)(uint8_t)json_str[0]);
        return;
    }

    log_json_preview("JSON completo:", json_str);

    cJSON* root = cJSON_ParseWithLength(json_str.c_str(), json_str.size());
    if (!root) {
        ESP_LOGW(TAG, "JSON parse error (msg=%u).", msg_seq);
        return;
    }

    cJSON* kind  = cJSON_GetObjectItem(root, "kind");
    cJSON* topic = cJSON_GetObjectItem(root, "topic");

    if (cJSON_IsString(kind) || cJSON_IsString(topic)) {
        const char* k = cJSON_IsString(kind)  ? kind->valuestring  : "-";
        const char* t = cJSON_IsString(topic) ? topic->valuestring : "-";
        ESP_LOGI(TAG, "JSON kind=\"%s\" topic=\"%s\"", k, t);
    }

    // Caso que nos interesa: measurement/temp_batch
    if (cJSON_IsString(kind) && cJSON_IsString(topic) &&
        std::strcmp(kind->valuestring,  "measurement") == 0 &&
        std::strcmp(topic->valuestring, "measurement/temp_batch") == 0)
    {
        // *** Cambio clave: buscar 'payload.readings' ***
        cJSON* payload  = cJSON_GetObjectItem(root, "payload");
        cJSON* readings = nullptr;

        if (cJSON_IsObject(payload)) {
            readings = cJSON_GetObjectItem(payload, "readings");
        }
        // Compatibilidad por si llegara en la raíz
        if (!readings) {
            readings = cJSON_GetObjectItem(root, "readings");
        }

        if (cJSON_IsArray(readings)) {
            std::array<float, 10> vals{}; vals.fill(NAN);
            int i = 0;
            cJSON* it = nullptr;
            cJSON_ArrayForEach(it, readings) {
                if (i >= (int)vals.size()) break;
                cJSON* id    = cJSON_GetObjectItem(it, "id");
                cJSON* value = cJSON_GetObjectItem(it, "value");
                cJSON* unit  = cJSON_GetObjectItem(it, "unit");
                if (cJSON_IsString(id) && cJSON_IsNumber(value)) {
                    vals[i] = (float)value->valuedouble;
                    ESP_LOGI(TAG, "Temp[%d] %s = %.2f %s",
                             i, id->valuestring, vals[i],
                             (cJSON_IsString(unit) ? unit->valuestring : ""));
                    ++i;
                }
            }
            // Publicar en estado/GUI
            CommRxState::setTemps(vals, /*seq_hint*/msg_seq, /*ts_ms*/0);
        } else {
            ESP_LOGW(TAG, "measurement/temp_batch sin 'payload.readings' array");
        }
    }

    cJSON_Delete(root);
}


// Al recibir un fragmento JSON_BRIDGE
static void handle_json_bridge_chunk(const uint8_t* data_in, size_t len_in)
{
    const uint8_t* data = data_in;
    size_t len = len_in;

    if (!data || len < sizeof(app_v1_json_chunk_hdr_t)) {
        ESP_LOGW(TAG, "JSON_BRIDGE: trama demasiado corta (%u)", (unsigned)len);
        return;
    }

    // Logs de depuración básicos (tamaño de la cabecera esperada)
    ESP_LOGI(TAG, "HMI: sizeof(app_v1_json_chunk_hdr_t) = %u",
             (unsigned)sizeof(app_v1_json_chunk_hdr_t));

    // Dump muy corto de los primeros bytes recibidos
    if (len >= 16) {
        ESP_LOGI(TAG, "HMI: RAW[0..15] = [ %s ]", hex_preview(data, 16, 16).c_str());
    }

    // -------------------------------------------------------------------------
    // FIX PRÁCTICO (robusto):
    // Algunos frames llegan como [Header v1 completo | Header JSON_BRIDGE | datos].
    // El payload que nos pasa el transporte empieza 6 bytes antes (en ts10ms+len).
    // Este bloque detecta ese patrón y aplica offset +6 de forma segura en TODOS
    // los tamaños (también en el último fragmento pequeño).
    // -------------------------------------------------------------------------
    if (len >= 6 + sizeof(app_v1_json_chunk_hdr_t)) {
        const uint8_t* p = data;

        // Campos del header v1 que nos llegan delante: ts10ms(4B LE) + len(2B LE)
        const uint16_t v1_len = (uint16_t)p[4] | ((uint16_t)p[5] << 8);

        // Candidato a header JSON en p+6:
        const auto* h2 = reinterpret_cast<const app_v1_json_chunk_hdr_t*>(p + 6);

        // Sanidad del header JSON candidato
        const bool sane_counts =
            (h2->chunk_count > 0) &&
            (h2->chunk_idx < h2->chunk_count);

        const bool sane_lengths =
            (h2->chunk_len > 0) &&
            (h2->chunk_len <= APP_V1_MAX_PAYLOAD);

        // Coherencia de longitudes
        const size_t need_b = 6 + sizeof(*h2) + (size_t)h2->chunk_len;

        const bool length_match =
            (v1_len == (uint16_t)(sizeof(*h2) + h2->chunk_len)) ||
            (need_b == len) ||
            (need_b <  len && v1_len == (uint16_t)(sizeof(*h2) + h2->chunk_len));

        if (sane_counts && sane_lengths && length_match) {
            ESP_LOGW(TAG, "JSON_BRIDGE: offset fixup aplicado (+6 bytes) (parece header v1 incrustado antes del fragmento)");
            data += 6;
            len  -= 6;
        }
    }

    // Ya con el puntero corregido si aplicó el fix:
    if (len < sizeof(app_v1_json_chunk_hdr_t)) {
        ESP_LOGW(TAG, "JSON_BRIDGE: tras fix, trama demasiado corta (%u)", (unsigned)len);
        return;
    }

    const app_v1_json_chunk_hdr_t* hdr =
        reinterpret_cast<const app_v1_json_chunk_hdr_t*>(data);

    const uint16_t msg_seq   = hdr->msg_seq;
    const uint16_t chunk_idx = hdr->chunk_idx;
    const uint16_t chunk_cnt = hdr->chunk_count;
    const uint16_t chunk_len = hdr->chunk_len;

    ESP_LOGI(TAG, "HMI-HDR msg_seq=%u idx=%u/%u chunk_len=%u crc16=0x%04X",
             (unsigned)msg_seq, (unsigned)chunk_idx, (unsigned)chunk_cnt,
             (unsigned)chunk_len, (unsigned)hdr->crc16);

    // Bytes disponibles realmente en este paquete
    size_t avail = len - sizeof(app_v1_json_chunk_hdr_t);
    size_t bytes_to_copy = std::min<size_t>(chunk_len, avail);
    const uint8_t* payload = data + sizeof(app_v1_json_chunk_hdr_t);

    // CRC opcional (ahora el CORE lo calcula; validamos si el valor parece "real")
    if (hdr->crc16 > 0x00FF) {
        uint16_t crc_calc = AppV1::json_fragment_crc16(hdr, payload, (uint16_t)bytes_to_copy);
        if (crc_calc != hdr->crc16) {
            ESP_LOGW(TAG,
                    "JSON_BRIDGE CRC mismatch msg=%u idx=%u (calc=0x%04X hdr=0x%04X) → se ignora",
                    (unsigned)msg_seq, (unsigned)chunk_idx,
                    crc_calc, hdr->crc16);
            // Podemos optar por descartar el fragmento. Aquí solo avisamos y seguimos,
            // pero NO lo insertamos si no coincide.
            return;
        }
    }

    ESP_LOGI(TAG, "JSON_BRIDGE msg=%u idx=%u/%u chunk_len=%u avail=%u → copy=%u",
             (unsigned)msg_seq,
             (unsigned)(chunk_idx + 1),
             (unsigned)chunk_cnt,
             (unsigned)chunk_len,
             (unsigned)avail,
             (unsigned)bytes_to_copy);

    // Inicializa/valida entrada pending
    auto& entry = s_pending[msg_seq];
    if (entry.received == 0) {
        entry.msg_seq     = msg_seq;
        entry.chunk_count = chunk_cnt;
        entry.parts.assign(chunk_cnt, {});
        entry.assembled.clear();
        entry.total_bytes = 0;
    } else if (entry.chunk_count != chunk_cnt) {
        ESP_LOGW(TAG, "JSON_BRIDGE: chunk_count cambió (%u -> %u) en msg=%u. Reinicio.",
                 (unsigned)entry.chunk_count, (unsigned)chunk_cnt, (unsigned)msg_seq);
        entry = PendingJson{};
        entry.msg_seq     = msg_seq;
        entry.chunk_count = chunk_cnt;
        entry.parts.assign(chunk_cnt, {});
    }

    // Rango válido
    if (chunk_idx >= entry.parts.size()) {
        ESP_LOGW(TAG, "JSON_BRIDGE: idx=%u fuera de rango (cnt=%u)",
                 (unsigned)chunk_idx, (unsigned)entry.parts.size());
        return;
    }

    // Evita contar doble
    bool first_time_for_idx = entry.parts[chunk_idx].empty();
    if (first_time_for_idx) {
        entry.parts[chunk_idx].assign(payload, payload + bytes_to_copy);
        entry.received++;
        entry.total_bytes += bytes_to_copy;
    } else if (entry.parts[chunk_idx].size() != bytes_to_copy) {
        entry.total_bytes -= entry.parts[chunk_idx].size();
        entry.parts[chunk_idx].assign(payload, payload + bytes_to_copy);
        entry.total_bytes += bytes_to_copy;
    }

    // ¿Completado?
    if (entry.received == entry.chunk_count) {
        entry.assembled.clear();
        entry.assembled.reserve(entry.total_bytes);
        for (uint16_t i = 0; i < entry.chunk_count; ++i) {
            const auto& part = entry.parts[i];
            if (part.empty()) {
                ESP_LOGW(TAG, "JSON_BRIDGE: falta idx=%u en msg=%u", (unsigned)i, (unsigned)msg_seq);
                return;
            }
            entry.assembled.insert(entry.assembled.end(), part.begin(), part.end());
        }

        ESP_LOGI(TAG, "JSON_BRIDGE ensamblado msg=%u bytes=%u",
                 (unsigned)msg_seq, (unsigned)entry.assembled.size());

        std::string json_str(reinterpret_cast<const char*>(entry.assembled.data()),
                             entry.assembled.size());

        parse_and_publish(json_str, msg_seq);

        s_pending.erase(msg_seq);
    }

    // Guardia: máximo de mensajes pendientes
    constexpr size_t MAX_PENDING_MSGS = 16;
    if (s_pending.size() > MAX_PENDING_MSGS) {
        ESP_LOGW(TAG, "JSON_BRIDGE: demasiados mensajes pendientes (%u). Purga uno.",
                 (unsigned)s_pending.size());
        s_pending.erase(s_pending.begin());
    }
}

// =====================================================================================
// Bucle RX: consume la cola y enruta por tipos de mensaje
// =====================================================================================

static void task_rx_(void*)
{
    AppEnvelope env{};
    for (;;) {
        if (!comm_rx_queue_recv(env, portMAX_DELAY)) continue;

        CommDiag::incRxPkt();
        CommDiag::setLastSeq(env.header.seq);

        if (env.header.ver != 1) {
            ESP_LOGW(TAG, "Proto mismatch (ver=%u != %u)", env.header.ver, 1);
            continue;
        }

        if (env.header.type == APP_V1_TYPE_JSON_BRIDGE) {
            const uint8_t* data = env.payload.data();
            const size_t   len  = env.header.len;
            ESP_LOGI(TAG, "RX JSON_BRIDGE raw len=%u", (unsigned)len);
            handle_json_bridge_chunk(data, len);
            continue;
        }

        switch (env.header.type) {
            case Telemetry: {
                if (env.header.len >= sizeof(AppTelemetryPayload)) {
                    auto* p = reinterpret_cast<const AppTelemetryPayload*>(env.payload.data());
                    CommRxState::setTelemetryRaw(p->temps, 8, env.header.seq);
                } else {
                    ESP_LOGW(TAG, "bad len for Telemetry");
                    CommDiag::incRxDropped();
                }
            } break;

            case Ack:
                CommDiag::incRxAck();
                ESP_LOGI(TAG, "RX ACK seq=%u", env.header.seq);
                break;

            default:
                ESP_LOGI(TAG, "RX type=0x%02X seq=%u len=%u",
                         env.header.type, env.header.seq, env.header.len);
                break;
        }
    }
}

bool comm_rx_handler_start()
{
    if (s_task) return true;
    return xTaskCreate(task_rx_, "comm_rx", 6144, nullptr, 7, &s_task) == pdPASS;
}
