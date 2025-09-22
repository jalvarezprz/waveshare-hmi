#pragma once
#include <cstddef>
#include <cstdint>

/**
 * @file comm_tx_json_bridge.h
 * @brief Envío de JSON grandes fragmentados vía AppV1::JsonBridge (0x30).
 *
 * API pública:
 *   - comm_tx_json_send(json, len)
 *   - comm_tx_json_bridge_send(json, len)  [alias inline]
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encola un JSON completo para ser enviado fragmentado.
 *
 * @param json Puntero al buffer JSON (texto UTF-8).
 * @param len  Longitud del JSON (en bytes, sin terminador).
 * @return true si se encoló correctamente, false en error.
 */
bool comm_tx_json_send(const char* json, size_t len);

/**
 * @brief Alias para compatibilidad con código que usa comm_tx_json_bridge_send.
 */
static inline bool comm_tx_json_bridge_send(const char* json, size_t len) {
    return comm_tx_json_send(json, len);
}

#ifdef __cplusplus
}
#endif
