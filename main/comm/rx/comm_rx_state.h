#pragma once
/**
 * @file comm_rx_state.h
 * @brief Almacén central de telemetría recibida desde el CORE.
 *
 * - Soporta 10 temperaturas (layout canónico para la pantalla 2×5).
 * - Mantiene compatibilidad con la API previa de 8 sensores.
 * - Acceso thread-safe mediante mutex de FreeRTOS.
 */

#include <array>
#include <cstdint>
#include <cstddef>   // <-- para size_t

namespace CommRxState {

/** Número de sensores soportados en la nueva API. */
constexpr size_t kNumTemps = 10;

/** Snapshot completo y coherente del estado. */
struct Snapshot {
    std::array<float, kNumTemps> temps;  ///< Temperaturas en °C (NaN si no válido)
    std::array<bool,  kNumTemps> valid;  ///< true si la posición tiene dato válido
    uint64_t ts_ms = 0;                  ///< instante (ms) del último update
    uint32_t seq   = 0;                  ///< secuencia lógica (si aplica)
};

void init();

/** Establece las 10 temperaturas (NaN para “sin dato”). */
void setTemps(const std::array<float, kNumTemps>& vals, uint32_t seq = 0, uint64_t ts_ms = 0);

/** Obtiene el snapshot completo (copia por valor, thread-safe). */
Snapshot getSnapshot();

/** Acceso directo a solo el array de 10 temperaturas (copia). */
std::array<float, kNumTemps> getTemps();

/* ==================== API LEGADA (8 sensores) ==================== */
void setTelemetryRaw(const int16_t* temps8, uint8_t count, uint8_t seq);
std::array<float, 8> getTempsCelsius8();

/* Alias de compatibilidad con código antiguo (comm_demo_loopback.cpp) */
inline std::array<float, 8> getTempsCelsius() { return getTempsCelsius8(); }

} // namespace CommRxState
