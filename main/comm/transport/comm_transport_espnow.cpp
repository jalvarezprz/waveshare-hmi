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

/* ========= Header “en el aire” (AJUSTA si el tuyo difiere) ========= */
#pragma pack(push, 1)
struct AppHeaderRadio {
    uint8_t  ver;
    uint8_t  type;
    uint16_t seq;
    // Nota: algunos diseños llevan len aquí; para 0x30 no te fíes de ese len,
    //       usa frame_len - sizeof(AppHeaderRadio).
};
#pragma pack(pop)

/* =================== Helpers =================== */

static inline const char* mac_to_str(const uint8_t* mac, char* buf, size_t buflen) {
    if (!mac || !buf || buflen < 18) return "";
    std::snprintf(buf, buflen, "%02X:%02X:%02X:%02X:%02X:%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

/* =================== RX callback =================== */

static void espnow_rx_cb(const esp_now_recv_info_t* info, const uint8_t* frame, int frame_len)
{
    if (!frame || frame_len < (int)sizeof(AppHeaderRadio)) {
        ESP_LOGW(TAG, "RX frame corto (%d)", frame_len);
        return;
    }

    const AppHeaderRadio* h = reinterpret_cast<const AppHeaderRadio*>(frame);
    const uint8_t* payload  = frame + sizeof(AppHeaderRadio);
    int payload_len         = frame_len - (int)sizeof(AppHeaderRadio);
    if (payload_len < 0) payload_len = 0;

    AppEnvelope env{}; // estructura propia del proyecto (comm_commitment.h)
    env.header.ver = h->ver;
    env.header.type = h->type;
    env.header.seq = h->seq;

    // ***** CLAVE: copiar SIEMPRE el payload real que llegó por radio *****
    env.header.len = (uint16_t)((payload_len >= 0 && payload_len <= 0xFFFF) ? payload_len : 0);

    if (payload_len > 0) {
        const size_t MAX = env.payload.size();  // std::array<uint8_t, N>
        size_t n = (payload_len <= (int)MAX) ? (size_t)payload_len : MAX;
        std::memcpy(env.payload.data(), payload, n);
        if ((int)n < payload_len) {
            ESP_LOGW(TAG, "Payload truncado RX: %d -> %u", payload_len, (unsigned)n);
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
/* Firmas usadas por comm_tx_gateway.cpp y main.cpp                      */

bool comm_espnow_init_sta()
{
    esp_err_t err;

    // --- NVS obligatorio para Wi-Fi / ESP-NOW ---
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Partición llena o versión distinta → borra y vuelve a init
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
    // Acepta con ":" o "-" (ej. "AA:BB:CC:DD:EE:FF")
    int n = std::sscanf(s, "%2x%*[:\-]%2x%*[:\-]%2x%*[:\-]%2x%*[:\-]%2x%*[:\-]%2x",
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
    peer.channel = channel;          // 0 = canal actual
    peer.encrypt = false;

    esp_err_t err;

    // Si ya existe, elimínalo antes de añadir
    if (esp_now_is_peer_exist(mac)) {
        esp_now_del_peer(mac);
    }

    err = esp_now_add_peer(&peer);
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
