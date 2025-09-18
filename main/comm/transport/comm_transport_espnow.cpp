#include <cstring>
#include <cstdio>
#include <array>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "comm_commitment.h"          // AppEnvelope, AppHeader, AppMsgType, APP_PROTO_VER
#include "comm/rx/comm_rx_queue.h"    // comm_rx_queue_send()

static const char* TAG = "comm_transport_espnow";

/* ========= Cabeceras “en el aire” detectables (empaquetadas) ========= */
#pragma pack(push, 1)
// AppV1 real (lo que envía el CORE)
struct AppV1HeaderShim {
    uint8_t  ver;
    uint8_t  type;
    uint8_t  seq;
    uint8_t  flags;
    uint16_t ts10ms;  // LE
    uint16_t len;     // LE = bytes de payload inmediatamente después de esta cabecera
};

// Formato legacy mínimo que se usó antes (sin len): ver(1), type(1), seq(2)
struct LegacyHeader {
    uint8_t  ver;
    uint8_t  type;
    uint16_t seq;
};
#pragma pack(pop)

/* =================== Helpers =================== */

static inline const char* mac_to_str(const uint8_t* mac, char* buf, size_t buflen) {
    if (!mac || !buf || buflen < 18) return "";
    std::snprintf(buf, buflen, "%02X:%02X:%02X:%02X:%02X:%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

static bool looks_like_appv1(const uint8_t* p, int n)
{
    if (!p || n < (int)sizeof(AppV1HeaderShim)) return false;
    const auto* h = reinterpret_cast<const AppV1HeaderShim*>(p);
    if (h->ver == 0 || h->ver > 2) return false;     // ver 1..2
    // El tipo puede ser 0x30 (JSON_BRIDGE) u otros (ACK/compat, etc.)
    // Chequeo básico de len coherente:
    const int rest = n - (int)sizeof(AppV1HeaderShim);
    const int pay  = (int)h->len;
    if (pay <= 0 || pay > rest) return false;
    return true;
}

/* =================== RX callback =================== */

static void espnow_rx_cb(const esp_now_recv_info_t* info, const uint8_t* frame, int frame_len)
{
    if (!frame || frame_len <= 0) {
        ESP_LOGW(TAG, "RX frame vacío (%d)", frame_len);
        return;
    }

    AppEnvelope env{};
    const uint8_t* payload = nullptr;
    int payload_len = 0;

    // 1) Preferimos AppV1 “real” del aire (8B + len)
    if (looks_like_appv1(frame, frame_len)) {
        const auto* h = reinterpret_cast<const AppV1HeaderShim*>(frame);
        payload     = frame + sizeof(AppV1HeaderShim);
        payload_len = (int)h->len;

        env.header.ver   = h->ver;
        env.header.type  = h->type;
        env.header.seq   = h->seq;          // ojo: en JSON_BRIDGE cambia por chunk
        env.header.len   = (uint16_t)((payload_len >= 0 && payload_len <= 0xFFFF) ? payload_len : 0);
        // flags/ts10ms no están en AppEnvelope; si te interesan, extiéndelo.

        // Log útil para depurar
        if (h->type == 0x30) {
            ESP_LOGI(TAG, "RX AppV1: type=0x%02X seq=%u len=%u (JSON_BRIDGE)",
                     (unsigned)h->type, (unsigned)h->seq, (unsigned)h->len);
        } else {
            ESP_LOGI(TAG, "RX AppV1: type=0x%02X seq=%u len=%u",
                     (unsigned)h->type, (unsigned)h->seq, (unsigned)h->len);
        }
    }
    // 2) Si no parece AppV1, intentamos legacy (ver,type,seq) sin len
    else if (frame_len >= (int)sizeof(LegacyHeader)) {
        const auto* h = reinterpret_cast<const LegacyHeader*>(frame);
        payload     = frame + sizeof(LegacyHeader);
        payload_len = frame_len - (int)sizeof(LegacyHeader);

        env.header.ver   = h->ver;
        env.header.type  = h->type;
        env.header.seq   = h->seq;
        env.header.len   = (uint16_t)((payload_len >= 0 && payload_len <= 0xFFFF) ? payload_len : 0);

        ESP_LOGI(TAG, "RX Legacy: type=0x%02X seq=%u len=%u",
                 (unsigned)h->type, (unsigned)h->seq, (unsigned)payload_len);
    }
    // 3) Si no encaja en nada, descartamos con log
    else {
        ESP_LOGW(TAG, "RX frame no reconoce cabecera (len=%d). Se descarta.", frame_len);
        return;
    }

    // Copiar payload al sobre del proyecto (capado al tamaño del array)
    if (payload_len > 0) {
        const size_t MAX = env.payload.size();
        size_t n = (payload_len <= (int)MAX) ? (size_t)payload_len : MAX;
        std::memcpy(env.payload.data(), payload, n);
        if ((int)n < payload_len) {
            ESP_LOGW(TAG, "Payload RX truncado: %d -> %u", payload_len, (unsigned)n);
        }
    }

    // Enviar a la cola de procesado
    if (!comm_rx_queue_send(env, 0)) {
        char from[18]; mac_to_str(info ? info->src_addr : nullptr, from, sizeof(from));
        ESP_LOGW(TAG, "comm_rx_queue_send llena (seq=%u, from=%s)", env.header.seq, from);
    }
}

/* =================== TX callback (log) =================== */

static void espnow_tx_cb(const uint8_t* mac_addr, esp_now_send_status_t status)
{
    char to[18]; mac_to_str(mac_addr, to, sizeof(to));
    ESP_LOGI(TAG, "SEND_CB to %s -> %s", to,
             (status == ESP_NOW_SEND_SUCCESS) ? "OK" : "FAIL");
}

/* =================== API esperada por el proyecto =================== */

bool comm_espnow_init_sta()
{
    esp_err_t err;

    // --- NVS obligatorio para Wi-Fi / ESP-NOW ---
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init: %s", esp_err_to_name(err));
        return false;
    }

    // --- Netif + loop (idempotentes) ---
    err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_netif_init: %s", esp_err_to_name(err));
        return false;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default: %s", esp_err_to_name(err));
        return false;
    }

    // --- Wi-Fi STA ---
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) {
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(err));
        return false;
    }
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK && err != ESP_ERR_WIFI_MODE) {
        ESP_LOGE(TAG, "esp_wifi_set_mode: %s", esp_err_to_name(err));
        return false;
    }
    err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_MODE) {
        ESP_LOGE(TAG, "esp_wifi_start: %s", esp_err_to_name(err));
        return false;
    }

    // --- ESP-NOW + callbacks ---
    err = esp_now_init();
    if (err != ESP_OK && err != ESP_ERR_ESPNOW_EXIST) {
        ESP_LOGE(TAG, "esp_now_init: %s", esp_err_to_name(err));
        return false;
    }
    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_rx_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_tx_cb));

    ESP_LOGI(TAG, "ESP-NOW init STA OK");
    return true;
}

bool comm_espnow_get_self_mac(uint8_t* mac_out)
{
    if (!mac_out) return false;
    esp_err_t err = esp_read_mac(mac_out, ESP_MAC_WIFI_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_read_mac: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool comm_parse_mac_str(const char* s, uint8_t* mac_out)
{
    if (!s || !mac_out) return false;
    unsigned int m[6] = {};
    // Acepta ":" o "-" (ej. "AA:BB:CC:DD:EE:FF")
    int n = std::sscanf(s, "%2x%*[:\\-]%2x%*[:\\-]%2x%*[:\\-]%2x%*[:\\-]%2x%*[:\\-]%2x",
                        &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);
    if (n != 6) return false;
    for (int i = 0; i < 6; ++i) mac_out[i] = (uint8_t)m[i];
    return true;
}

bool comm_espnow_add_peer(const uint8_t* mac, uint8_t channel)
{
    if (!mac) return false;

    esp_now_peer_info_t peer{};
    std::memset(&peer, 0, sizeof(peer));
    std::memcpy(peer.peer_addr, mac, 6);
    peer.ifidx   = WIFI_IF_STA;                     // ⬅️ añadido
    peer.channel = channel;                         // 0 = canal actual
    peer.encrypt = false;

    if (esp_now_is_peer_exist(mac)) {
        esp_now_del_peer(mac);
    }

    esp_err_t err = esp_now_add_peer(&peer);
    if (err != ESP_OK) {
        char s[18]; mac_to_str(mac, s, sizeof(s));
        ESP_LOGE(TAG, "esp_now_add_peer(%s): %s", s, esp_err_to_name(err));
        return false;
    }
    char s[18]; mac_to_str(mac, s, sizeof(s));
    ESP_LOGI(TAG, "Peer añadido: %s (ch=%u)", s, (unsigned)peer.channel);
    return true;
}

bool comm_espnow_send(const uint8_t* mac, const void* data, unsigned len)
{
    if (!mac || (!data && len > 0)) return false;
    esp_err_t err = esp_now_send(mac, reinterpret_cast<const uint8_t*>(data), len);
    if (err != ESP_OK) {
        char s[18]; mac_to_str(mac, s, sizeof(s));
        ESP_LOGE(TAG, "esp_now_send(%s,len=%u): %s", s, len, esp_err_to_name(err));
        return false;
    }
    return true;
}
