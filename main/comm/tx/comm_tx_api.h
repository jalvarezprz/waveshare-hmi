#pragma once
#include <cstdint>

/** API semántica para que la UI envíe comandos */
namespace CommTxApi {
    bool init();                         ///< init opcional si se requiere
    bool sendLed(uint8_t idx, bool on);  ///< Encola comando LED (ReqAck)
    bool requestSnapshot();              ///< Encola petición de telemetría
}
