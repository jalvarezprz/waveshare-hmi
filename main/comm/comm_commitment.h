#pragma once
#include <cstdint>
#include <array>

/** Versión de protocolo de aplicación */
inline constexpr uint8_t APP_PROTO_VER = 1;

/** Tipos de mensaje de aplicación */
enum class AppMsgType : uint8_t {
    CmdLed          = 0x01,
    ReqSnapshot     = 0x02,
    TelemetryDs18x  = 0x10,
    Heartbeat       = 0x11,
    Ack             = 0xF0,
    Error           = 0xF1
};

/** Banderas de mensaje */
enum class AppMsgFlags : uint8_t {
    None   = 0x00,
    Ack    = 0x01,
    ReqAck = 0x02
};

/** Cabecera compacta de aplicación */
struct AppHeader {
    uint8_t  ver;
    uint8_t  type;     // AppMsgType
    uint8_t  seq;
    uint8_t  flags;    // AppMsgFlags
    uint16_t ts10ms;
    uint8_t  len;      // bytes de payload
} __attribute__((packed));

/** Envelope genérico para colas TX/RX */
struct AppEnvelope {
    AppHeader header{};
    std::array<uint8_t, 240> payload{}; // margen para ESPNOW
    int rssi{0};                        // si transporte lo aporta
} __attribute__((packed));

/** Payloads típicos */
struct AppCmdLedPayload     { uint8_t idx; uint8_t state; } __attribute__((packed));
struct AppTelemetryPayload  { int16_t temps[8]; }           __attribute__((packed));
