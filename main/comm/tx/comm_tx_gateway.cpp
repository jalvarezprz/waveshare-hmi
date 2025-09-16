#include "sdkconfig.h"
#include "comm_tx_gateway.h"

#include "comm_tx_queue.h"
#include "comm_commitment.h"       // AppEnvelope, AppHeader, enums, APP_PROTO_VER
#include "comm/comm_diag.h"
#include "comm/rx/comm_rx_queue.h" // para inyectar en RX en modo loopback

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
    AppEnvelope env{};
    for (;;)
    {
        if (!comm_tx_queue_recv(env, portMAX_DELAY)) continue;

        // Seguridad básica de longitud (no deberíamos enviar basura)
        if (env.header.len > sizeof(env.payload)) {
            ESP_LOGW(TAG, "Descartado: len=%u > payload=%u", env.header.len, (unsigned)sizeof(env.payload));
            continue;
        }

        CommDiag::incTxQueued();

//#if CONFIG_COMM_LOOPBACK
#if 0
        // Inyecta en la cola RX local (simula que "llega por radio")
        (void)comm_rx_queue_send(env, 0);
        CommDiag::incTxLoopback();

        // Si el emisor pide ACK, generamos uno sintético
        if (env.header.flags & static_cast<uint8_t>(AppMsgFlags::ReqAck)) {
            AppEnvelope ack{};
            ack.header.ver   = APP_PROTO_VER;
            ack.header.type  = static_cast<uint8_t>(AppMsgType::Ack);
            ack.header.seq   = env.header.seq;
            ack.header.flags = 0;
            ack.header.ts10ms= 0;
            ack.header.len   = 0;
            (void)comm_rx_queue_send(ack, 0);
        }

        ESP_LOGI(TAG, "LOOPBACK TX→RX type=0x%02X seq=%u len=%u",
                 env.header.type, env.header.seq, env.header.len);
#else
        // Envío real por ESP-NOW
        const size_t frame_len = sizeof(AppHeader) + env.header.len;
        bool ok = comm_espnow_send(s_peer_mac, &env, frame_len);
        ESP_LOGI(TAG, "TX ESPNOW type=0x%02X seq=%u len=%u -> %s",
                 env.header.type, env.header.seq, env.header.len, ok ? "OK" : "ERR");
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

    // Alta del peer (unencrypted). Con broadcast (FF:FF:...) esp-now no exige alta.
    // Aun así, dar de alta permite fijar canal si se usa unicast.
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
