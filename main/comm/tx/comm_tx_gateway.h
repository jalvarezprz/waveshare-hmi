#pragma once

/** Gateway TX: tarea que lee la cola TX y envía por el transporte (más adelante ESP-NOW) */
bool comm_tx_gateway_start();  ///< crea la tarea TX
