#include "comm_rx_state.h"
#include "freertos/semphr.h"
#include <algorithm>
#include <cmath>

namespace CommRxState {
static std::array<int16_t,8> s_vals{};
static std::array<uint8_t,8> s_valid{};
static uint8_t  s_seq{0};
static uint32_t s_tick{0};
static SemaphoreHandle_t s_mtx{nullptr};

static constexpr int MAX_L = 4;
static std::array<QueueHandle_t, MAX_L> s_listeners{};

static void notify_() {
    Event ev{ Event::Type::TelemetryUpdated, s_seq, s_tick };
    for (auto& q : s_listeners) if (q) (void)xQueueSend(q, &ev, 0);
}

bool init() {
    if (!s_mtx) s_mtx = xSemaphoreCreateMutex();
    s_vals.fill(0); s_valid.fill(0); s_seq = 0; s_tick = 0;
    s_listeners.fill(nullptr);
    return s_mtx != nullptr;
}

void setTelemetryRaw(const int16_t* temps8, uint8_t count, uint8_t seq) {
    if (!s_mtx && !init()) return;
    const uint8_t n = (count > 8) ? 8 : count;
    xSemaphoreTake(s_mtx, portMAX_DELAY);
    for (uint8_t i=0;i<n;++i){ s_vals[i]=temps8[i]; s_valid[i]=1; }
    for (uint8_t i=n;i<8;++i){ s_vals[i]=0; s_valid[i]=0; }
    s_seq = seq; s_tick = xTaskGetTickCount();
    xSemaphoreGive(s_mtx);
    notify_();
}

std::array<float,8> getTempsCelsius() {
    std::array<int16_t,8> v; std::array<uint8_t,8> ok;
    xSemaphoreTake(s_mtx, portMAX_DELAY);
    v = s_vals; ok = s_valid;
    xSemaphoreGive(s_mtx);
    std::array<float,8> out{};
    for (int i=0;i<8;++i) out[i] = ok[i]? (v[i]/100.0f) : NAN;
    return out;
}

bool subscribe(QueueHandle_t q) {
    if (!q) return false;
    for (auto& s : s_listeners) {
        if (s==q) return true;
        if (!s) { s=q; return true; }
    }
    return false;
}
void unsubscribe(QueueHandle_t q) {
    for (auto& s : s_listeners) if (s==q) { s=nullptr; return; }
}
}
