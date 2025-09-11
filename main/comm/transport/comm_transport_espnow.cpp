#include "comm_transport_espnow.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_log.h"
#include <cstring>
#include <cstdio>   // para sscanf

#include "comm/rx/comm_rx_queue.h"   // para inyectar en RX
#include "comm/comm_commitment.h"    // AppEnvelope, etc.
#include "comm/comm_diag.h"

#include "nvs_flash.h"
#include "esp_netif.h"

static const char* TAG = "comm_espnow";

static bool s_inited = false;

// Reemplaza tu wifi_init_sta_no_ip() por este:
static esp_err_t wifi_init_sta_no_ip()
{
    // 1) NVS para Wi-Fi
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(err);
    }

    // 2) esp-netif (stack de red, aunque no pidamos IP)
    static bool s_netif_inited = false;
    if (!s_netif_inited) {
        ESP_ERROR_CHECK(esp_netif_init());
        s_netif_inited = true;
    }

    // 3) Event loop (idempotente)
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }

    // 4) Wi-Fi en modo STA (sin conectar a AP)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    return ESP_OK;
}

static void espnow_recv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len)
{
    // Seguridad básica: tamaño mínimo de nuestro sobre
    if (!data || len < (int)sizeof(AppHeader)) {
        CommDiag::incRxDropped();
        return;
    }

    // Copia directa al AppEnvelope y a la cola RX
    AppEnvelope env{};
    size_t copy = len > sizeof(env) ? sizeof(env) : (size_t)len;
    std::memcpy(&env, data, copy);

    // Contabiliza y entrega
    CommDiag::incRxPkt();
    CommDiag::setLastSeq(env.header.seq);
    (void)comm_rx_queue_send(env, 0);
}

static void espnow_send_cb(const uint8_t* mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        CommDiag::incTxOk();
    } else {
        CommDiag::incTxFail();
    }
}

bool comm_espnow_init_sta()
{
    if (s_inited) return true;

    // Wi-Fi STA sin IP
    wifi_init_sta_no_ip();

    // Inicializa ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    s_inited = true;

    uint8_t mac[6]; comm_espnow_get_self_mac(mac);
    ESP_LOGI(TAG, "ESP-NOW listo (STA MAC: %02X:%02X:%02X:%02X:%02X:%02X)",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return true;
}

bool comm_espnow_add_peer(const uint8_t mac[6], uint8_t channel)
{
    if (!s_inited) return false;
    esp_now_peer_info_t p{};
    std::memcpy(p.peer_addr, mac, 6);
    p.ifidx = WIFI_IF_STA;
    p.channel = channel;
    p.encrypt = false;
    esp_err_t err = esp_now_add_peer(&p);
    if (err == ESP_ERR_ESPNOW_EXIST) return true;
    ESP_ERROR_CHECK(err);
    return true;
}

bool comm_espnow_send(const uint8_t mac[6], const void* data, size_t len)
{
    if (!s_inited || !mac || !data || len == 0) return false;
    esp_err_t err = esp_now_send(mac, reinterpret_cast<const uint8_t*>(data), len);
    return (err == ESP_OK);
}

bool comm_parse_mac_str(const char* str, uint8_t* mac)
{
    if (!str || !mac) return false;

    unsigned int v[6] = {0};  // Inicializar a 0
    int n = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
                   &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);

    if (n != 6) return false;

    for (int i = 0; i < 6; ++i) {
        mac[i] = static_cast<uint8_t>(v[i]);
    }

    return true;
}

void comm_espnow_get_self_mac(uint8_t mac[6])
{
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
}
