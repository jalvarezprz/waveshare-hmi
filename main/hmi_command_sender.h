#pragma once
#include <cstddef>
#include <cstdint>

namespace Hmi { namespace CommandSender {

using TxFn = bool(*)(const char* data, size_t len); // alternativa opcional

// Instala TX personalizado; pasa nullptr para usar la ruta por defecto (cola TX)
void set_tx_fn(TxFn fn);

// Enviar JSON “tal cual” (ya formateado por command_contract)
bool send_raw_json(const char* json);

// Helper alto nivel para comandos DO (usa command_contract)
bool send_do(const char* path, const char* op, bool expects_ack);

}} // namespace Hmi::CommandSender
