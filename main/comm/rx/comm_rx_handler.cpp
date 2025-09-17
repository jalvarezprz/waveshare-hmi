#include "comm_rx_handler.h"
#include "comm_rx_queue.h"
#include "comm_commitment.h"
#include "comm_diag.h"

#include "comm/rx/comm_rx_state.h"

#include "cJSON.h"

#include <map>
#include <vector>
#include <string>
#include <array>
#include <cstring>   // strcmp, memcpy
#include <cmath>     // NAN
#include <algorithm> // std::min

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// =====================================================================================
// Reensamblado de 0x30 (JSON_BRIDGE) como stream de sub-mensajes:
// Formato de cada sub-mensaje (LE):
//   [u16 code] [u16 index] [u16 hdr_len] [u16 payload_len] [payload]
// Observado en logs: code = 0x22B5 (texto/etiquetas) y 0x22B6 (JSON).
//   - 0x22B5: payload ASCII (p.ej. "MP_BOILER..." → lo guardamos como "topic_hint").
//   - 0x22B6: payload JSON (empieza por '{' o '['): lo parseamos.
// =====================================================================================

struct PendingMsg {
    std::vector<uint8_t> buf;   // buffer acumulado del stream (seq)
    uint32_t chunks = 0;        // nº fragmentos 0x30 recibidos con este seq
    std::string topic_hint;     // último 0x22B5 recibido para asociar con el siguiente JSON
};
static std::map<uint16_t, PendingMsg> s_streams; // clave: env.header.seq

static const char* TAG = "comm_rx_handler";
static TaskHandle_t s_task{nullptr};

// Lectura LE de 16 bits
static inline uint16_t rd16le(const uint8_t* p) {
    return (uint16_t)(p[0] | (uint16_t(p[1]) << 8));
}

// Utils de log
static std::string hex_preview(const uint8_t* p, size_t n, size_t maxn = 24) {
    char tmp[4];
    std::string out;
    size_t m = std::min(n, maxn);
    for (size_t i = 0; i < m; ++i) {
        snprintf(tmp, sizeof(tmp), "%02X", (unsigned)p[i]);
        if (i) out.push_back(' ');
        out += tmp;
    }
    if (n > m) out += " ...";
    return out;
}

static void log_json_preview(const char* prefix, const std::string& s) {
    std::string pv = s.substr(0, 80);
    ESP_LOGI(TAG, "%s %.80s%s",
             prefix, pv.c_str(), (s.size() > pv.size() ? " ..." : ""));
}

static void handle_json_payload(uint16_t stream_seq,
                                const std::string& topic_hint,
                                const uint8_t* js, size_t len)
{
    if (len == 0) return;

    // Sanidad rápida
    if (!(js[0] == '{' || js[0] == '[')) {
        ESP_LOGW(TAG, "Submensaje 0x22B6 no comienza por '{'/'[' (0x%02X). HEX: %s",
                 (unsigned)js[0], hex_preview(js, std::min((size_t)16, len)).c_str());
        return;
    }

    std::string json_str((const char*)js, len);
    ESP_LOGI(TAG, "SubJSON listo: len=%u%s",
             (unsigned)json_str.size(),
             topic_hint.empty() ? "" : " (con topic_hint previo)");
    if (!topic_hint.empty())
        ESP_LOGI(TAG, "topic_hint='%s'", topic_hint.c_str());
    log_json_preview("Preview:", json_str);

    // Parseo robusto
    cJSON* root = cJSON_ParseWithLength(json_str.c_str(), json_str.size());
    if (!root) {
        ESP_LOGW(TAG, "JSON parse error (seq=%u).", (unsigned)stream_seq);
        return;
    }

    // Logs de campos comunes si existen
    cJSON* kind  = cJSON_GetObjectItem(root, "kind");
    cJSON* topic = cJSON_GetObjectItem(root, "topic");
    if (cJSON_IsString(kind) || cJSON_IsString(topic)) {
        const char* k = cJSON_IsString(kind)  ? kind->valuestring  : "-";
        const char* t = cJSON_IsString(topic) ? topic->valuestring : "-";
        ESP_LOGI(TAG, "JSON kind=\"%s\" topic=\"%s\"", k, t);
    }

    // Caso concreto: batch de temperaturas
    if (cJSON_IsString(kind) && cJSON_IsString(topic) &&
        std::strcmp(kind->valuestring,  "measurement") == 0 &&
        std::strcmp(topic->valuestring, "measurement/temp_batch") == 0)
    {
        cJSON* readings = cJSON_GetObjectItem(root, "readings");
        if (cJSON_IsArray(readings)) {
            std::array<float, 10> vals{}; vals.fill(NAN);

            int i = 0; cJSON* it = nullptr;
            cJSON_ArrayForEach(it, readings) {
                if (i >= 10) break;
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
            // Publicar a la capa de estado/GUI
            CommRxState::setTemps(vals, /*seq*/stream_seq, /*ts_ms*/0);
        } else {
            ESP_LOGW(TAG, "measurement/temp_batch sin 'readings' array");
        }
    }

    cJSON_Delete(root);
}

// Consume del buffer acumulado tantos sub-mensajes completos como haya.
static void process_json_stream(uint16_t stream_seq, PendingMsg& st)
{
    size_t offset = 0;
    const uint8_t* data = st.buf.data();
    size_t avail = st.buf.size();

    while (true) {
        // Necesitamos al menos la cabecera de 8 bytes
        if (avail - offset < 8) break;

        const uint16_t code     = rd16le(data + offset + 0); // 0x22B5 / 0x22B6
        const uint16_t index    = rd16le(data + offset + 2); // contador/índice observado en logs
        const uint16_t hdr_len  = rd16le(data + offset + 4); // suele ser 6 (observado), no lo usamos
        const uint16_t pay_len  = rd16le(data + offset + 6);

        ESP_LOGI(TAG, "JSON_BRIDGE submsg: seq=%u code=0x%04X index=%u hdr_len=%u payload_len=%u",
                 (unsigned)stream_seq, (unsigned)code, (unsigned)index,
                 (unsigned)hdr_len, (unsigned)pay_len);

        const size_t need = 8u + (size_t)pay_len;
        const size_t have = avail - offset;
        if (have < need) {
            ESP_LOGD(TAG, "Submensaje incompleto: tengo=%u necesito=%u → espero más",
                     (unsigned)have, (unsigned)need);
            break;
        }

        const uint8_t* payload = data + offset + 8;

        if (code == 0x22B6) {
            // JSON
            handle_json_payload(stream_seq, st.topic_hint, payload, pay_len);
            // Consumido un JSON → invalidar hint (se asume “consume once”)
            st.topic_hint.clear();
        } else if (code == 0x22B5) {
            // Texto/etiquetas/tópico antes del JSON. Guardamos y mostramos preview.
            std::string txt((const char*)payload, (size_t)pay_len);
            // Asegurar que sea imprimible; si no, mostramos HEX
            bool printable = true;
            for (uint8_t c : txt) {
                if (c == 0) break;
                if ((c < 0x20 || c > 0x7E) && c != '\n' && c != '\r' && c != '\t') {
                    printable = false; break;
                }
            }
            if (printable) {
                // Recortar posibles nulos finales
                while (!txt.empty() && txt.back() == '\0') txt.pop_back();
                st.topic_hint = txt;
                ESP_LOGI(TAG, "topic_hint(0x22B5): '%s'", st.topic_hint.c_str());
            } else {
                ESP_LOGW(TAG, "0x22B5 no ASCII imprimible. HEX: %s",
                         hex_preview(payload, std::min((size_t)24, (size_t)pay_len)).c_str());
                st.topic_hint.clear();
            }
        } else {
            // Desconocido: log + preview
            ESP_LOGW(TAG, "code desconocido 0x%04X. HEX payload: %s",
                     (unsigned)code, hex_preview(payload, std::min((size_t)24, (size_t)pay_len)).c_str());
        }

        // Avanzar al siguiente submensaje
        offset += need;
    }

    // Compactar buffer (eliminar lo consumido)
    if (offset > 0) {
        if (offset >= st.buf.size()) {
            st.buf.clear();
        } else {
            st.buf.erase(st.buf.begin(), st.buf.begin() + (ptrdiff_t)offset);
        }
    }

    // Guardia de memoria
    constexpr size_t MAX_STREAM_BUF = 128 * 1024; // 128 KB
    if (st.buf.size() > MAX_STREAM_BUF) {
        ESP_LOGW(TAG, "Buffer de stream 0x30 demasiado grande (%u). Se limpia.", (unsigned)st.buf.size());
        st.buf.clear();
        st.topic_hint.clear();
    }
}

static void handle_json_bridge_stream(uint16_t stream_seq, const uint8_t* frag, size_t frag_len)
{
    auto& st = s_streams[stream_seq];
    st.chunks++;

    // Anexar fragmento tal cual
    const size_t before = st.buf.size();
    st.buf.insert(st.buf.end(), frag, frag + frag_len);
    const size_t after = st.buf.size();

    ESP_LOGI(TAG, "JSON_BRIDGE seq=%u chunk #%u (this=%u, total=%u)",
             (unsigned)stream_seq, (unsigned)st.chunks,
             (unsigned)frag_len, (unsigned)after);

    // Procesar lo que ya esté completo en el buffer
    process_json_stream(stream_seq, st);
}

static void task_rx_(void*)
{
    AppEnvelope env{};
    for (;;) {
        if (!comm_rx_queue_recv(env, portMAX_DELAY)) continue;

        CommDiag::incRxPkt();
        CommDiag::setLastSeq(env.header.seq);

        if (env.header.ver != APP_PROTO_VER) {
            ESP_LOGW(TAG, "proto mismatch");
            continue;
        }

        // 0x30 → JSON_BRIDGE (stream de submensajes)
        if (env.header.type == 0x30) {
            const uint8_t* data = env.payload.data();   // fragmento puro
            const size_t   len  = env.header.len;

            handle_json_bridge_stream(env.header.seq, data, len);
            continue;
        }

        // Resto de tipos
        auto type = static_cast<AppMsgType>(env.header.type);
        switch (type) {
            case AppMsgType::TelemetryDs18x: {
                if (env.header.len >= sizeof(AppTelemetryPayload)) {
                    auto* p = reinterpret_cast<const AppTelemetryPayload*>(env.payload.data());
                    CommRxState::setTelemetryRaw(p->temps, 8, env.header.seq);
                } else {
                    ESP_LOGW(TAG, "bad len for Telemetry");
                    CommDiag::incRxDropped();
                }
            } break;

            case AppMsgType::Ack:
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
