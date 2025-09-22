#pragma once
#include <cstdint>

// API de alto nivel de TX del HMI
namespace CommTxApi {

/// Inicializa la pasarela de TX (Wi-Fi STA + ESP-NOW + tarea TX).
/// Devuelve true si queda lista para enviar.
bool init();

/// Enviar un comando LED sencillo (payload binario).
bool sendLed(uint8_t led_idx, bool on);

/// Solicitar un snapshot/telemetría inmediata (payload vacío).
bool requestSnapshot();

} // namespace CommTxApi
