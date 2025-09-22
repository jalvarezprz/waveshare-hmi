#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "comm/comm_proto_v1.h"   // AppV1::Envelope

/**
 * @file comm_tx_queue.h
 * @brief Cola de transmisión de mensajes AppV1 (CORE ↔ HMI).
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa la cola de transmisión. */
void comm_tx_queue_init();

/** Envía un envelope a la cola de TX. */
bool comm_tx_queue_send(const AppV1::Envelope& env, TickType_t to);

/** Recibe un envelope desde la cola de TX. */
bool comm_tx_queue_recv(AppV1::Envelope& out, TickType_t to);

/** Número de elementos en la cola. */
size_t comm_tx_queue_level();

#ifdef __cplusplus
}
#endif
