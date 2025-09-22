#include "sdkconfig.h"
#include "comm/tx/comm_tx_gateway.h"

#include "comm/tx/comm_tx_queue.h"
#include "comm/comm_proto_v1.h"          // <-- contrato común (sustituye a “comm_commitment.h”)
#include "comm/comm_diag.h"
#include "comm/rx/comm_rx_queue.h"       // para loopback (si se activa)

// #if !CONFIG_COMM_LOOPBACK
#if 1
#include "comm/transport/comm_transport_espnow.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "comm_tx_gateway";

static TaskHandle_t s_task{nullptr};

//#if !CONFIG_COMM_LOOPBACK
#if 1
// Peer por defecto: broadcast; se sobrescribe con CONFIG_COMM_PEER_MAC_STR si existe
static uint8_t s_peer_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static uint8_t s_channel =
#ifdef CONFIG_COMM_ESPNOW_CHANNEL
    (uint8_t)CONFIG_COMM_ESPNOW_CHANNEL;
#else
    1;
#endif
#endif // !CONFIG_COMM_LOOPBACK

static void task_tx_(void*)
{
    AppV1::Envelope env{};
    for (;;)
    {
        if (!comm_tx_queue_recv(env, portMAX_DELAY)) continue;

        // Seguridad básica de longitud
        if (env.hdr.len > sizeof(env.payload)) {
            ESP_LOGW(TAG, "Descartado: len=%u > payload=%u",
                     (unsigned)env.hdr.len, (unsigned)sizeof(env.payload));
            continue;
        }

        CommDiag::incTxQueued();

//#if CONFIG_COMM_LOOPBACK
#if 0
        // Inyecta en la cola RX local (simula que "llega por radio")
        (void)comm_rx_queue_send(env, 0);
        CommDiag::incTxLoopback();

        // Si el emisor pidiese ACK, aquí podrías generar uno sintético
        // (bloque omitido para simplificar; no usamos AppMsgFlags aquí)

        ESP_LOGI(TAG, "LOOPBACK TX→RX type=0x%02X seq=%u len=%u",
                 env.hdr.type, env.hdr.seq, env.hdr.len);
#else
        // Envío real por ESP-NOW
        const size_t frame_len = sizeof(AppV1::Header) + env.hdr.len;
        bool ok = comm_espnow_send(s_peer_mac, &env, frame_len);
        ESP_LOGI(TAG, "TX ESPNOW type=0x%02X seq=%u len=%u -> %s",
                 env.hdr.type, env.hdr.seq, env.hdr.len, ok ? "OK" : "ERR");
        // El resultado final (ok/fail) lo contabiliza el callback global de send
#endif
    }
}

bool comm_tx_gateway_start()
{
    if (s_task) return true;

//#if !CONFIG_COMM_LOOPBACK
#if 1
    // Inicializa Wi-Fi (STA sin IP) + ESP-NOW y registra callbacks
    if (!comm_espnow_init_sta()) {
        ESP_LOGE(TAG, "Fallo en comm_espnow_init_sta()");
        return false;
    }

    // Lee la MAC del peer desde sdkconfig si está definida
    #ifdef CONFIG_COMM_PEER_MAC_STR
    uint8_t tmp[6];
    if (comm_parse_mac_str(CONFIG_COMM_PEER_MAC_STR, tmp)) {
        std::memcpy(s_peer_mac, tmp, 6);
    }
    #endif

    // Alta del peer (unencrypted). Con broadcast no es estrictamente necesario
    (void)comm_espnow_add_peer(s_peer_mac, s_channel);

    // Log de la MAC local para emparejado manual
    uint8_t self[6]; comm_espnow_get_self_mac(self);
    ESP_LOGI(TAG, "STA MAC local: %02X:%02X:%02X:%02X:%02X:%02X (canal=%u)",
             self[0], self[1], self[2], self[3], self[4], self[5], s_channel);
#endif

    if (xTaskCreate(task_tx_, "comm_tx", 4096, nullptr, 8, &s_task) != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear la tarea TX");
        s_task = nullptr;
        return false;
    }
    return true;
}
