#pragma once
#include <cstddef>
#include <cstdint>

/** Inicializa la pasarela TX (WiFi/ESPNOW, task de envío, etc.). */
bool comm_tx_gateway_start();
