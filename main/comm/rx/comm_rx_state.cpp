#include "comm_rx_state.h"
#include <cmath>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"

namespace CommRxState {

static const char* TAG = "CommRxState";

// Estado interno
static Snapshot              s_snap{};
static SemaphoreHandle_t     s_mutex = nullptr;
static bool                  s_inited = false;

static inline uint64_t now_ms() {
    return static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);
}

static void ensure_init() {
    if (s_inited) return;
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) {
        // Si no hay mutex, seguimos pero sin protección (muy improbable)
        ESP_LOGE(TAG, "Mutex create failed");
    }
    // Estado inicial: NaN + valid=false
    for (size_t i = 0; i < kNumTemps; ++i) {
        s_snap.temps[i] = NAN;
        s_snap.valid[i] = false;
    }
    s_snap.ts_ms = 0;
    s_snap.seq   = 0;
    s_inited = true;
}

/* ====================== API nueva (10 sensores) ====================== */

void init() {
    ensure_init();
}

void setTemps(const std::array<float, kNumTemps>& vals, uint32_t seq, uint64_t ts_ms) {
    ensure_init();
    if (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        s_snap.temps = vals;
        for (size_t i = 0; i < kNumTemps; ++i) {
            s_snap.valid[i] = !std::isnan(vals[i]);
        }
        s_snap.seq   = seq;
        s_snap.ts_ms = (ts_ms != 0) ? ts_ms : now_ms();
        xSemaphoreGive(s_mutex);
    } else {
        // Fallback sin mutex
        s_snap.temps = vals;
        for (size_t i = 0; i < kNumTemps; ++i) {
            s_snap.valid[i] = !std::isnan(vals[i]);
        }
        s_snap.seq   = seq;
        s_snap.ts_ms = (ts_ms != 0) ? ts_ms : now_ms();
    }
}

Snapshot getSnapshot() {
    ensure_init();
    Snapshot out;
    if (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        out = s_snap;
        xSemaphoreGive(s_mutex);
    } else {
        // Sin mutex, copia “best effort”
        out = s_snap;
    }
    return out;
}

std::array<float, kNumTemps> getTemps() {
    ensure_init();
    std::array<float, kNumTemps> out{};
    if (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        out = s_snap.temps;
        xSemaphoreGive(s_mutex);
    } else {
        out = s_snap.temps;
    }
    return out;
}

/* ====================== API legada (8 sensores) ====================== */

void setTelemetryRaw(const int16_t* temps8, uint8_t count, uint8_t seq) {
    // Conversión típica: DS18x (Q4.4, Q8.4, entero*16, etc.)
    // Aquí suponemos entero*16 → °C = raw / 16.0f. Ajusta si tu protocolo es distinto.
    ensure_init();

    std::array<float, kNumTemps> vals{};
    // Rellena NaN por defecto
    for (size_t i = 0; i < kNumTemps; ++i) vals[i] = NAN;

    const uint8_t n = (count > 8) ? 8 : count;
    for (uint8_t i = 0; i < n; ++i) {
        vals[i] = static_cast<float>(temps8[i]) / 16.0f;
    }
    setTemps(vals, seq, now_ms());
}

std::array<float, 8> getTempsCelsius8() {
    ensure_init();
    std::array<float, 8> out{};
    for (size_t i = 0; i < 8; ++i) {
        out[i] = s_snap.temps[i];
    }
    return out;
}

} // namespace CommRxState
