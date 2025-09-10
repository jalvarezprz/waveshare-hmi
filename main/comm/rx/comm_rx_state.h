#pragma once
#include <array>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace CommRxState {
/** Evento simple para notificar nueva telemetría */
struct Event {
    enum class Type : uint8_t { TelemetryUpdated = 1 };
    Type     type;
    uint8_t  seq;
    uint32_t tick;
};

bool init();

void setTelemetryRaw(const int16_t* temps8, uint8_t count, uint8_t seq);

std::array<float,8> getTempsCelsius();

bool subscribe(QueueHandle_t q);
void unsubscribe(QueueHandle_t q);
}
