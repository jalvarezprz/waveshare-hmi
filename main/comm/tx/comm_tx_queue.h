#pragma once
#include "comm_commitment.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/** Cola TX: interfaz sencilla para encolar AppEnvelope hacia el gateway */
bool          comm_tx_queue_init(size_t depth = 16);
QueueHandle_t comm_tx_queue_handle();
bool          comm_tx_queue_send(const AppEnvelope& env, TickType_t to = 0);
bool          comm_tx_queue_recv(AppEnvelope& env, TickType_t to = portMAX_DELAY);
