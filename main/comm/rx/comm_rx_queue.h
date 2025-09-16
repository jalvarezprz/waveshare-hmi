#pragma once
#include "comm_commitment.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/** Cola RX: interfaz para que el gateway (o loopback) inyecte paquetes recibidos */
bool          comm_rx_queue_init(size_t depth = 16);
QueueHandle_t comm_rx_queue_handle();
bool          comm_rx_queue_send(const AppEnvelope& env, TickType_t to = 0);
bool          comm_rx_queue_recv(AppEnvelope& env, TickType_t to = portMAX_DELAY);
