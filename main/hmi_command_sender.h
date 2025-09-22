#pragma once
#include <cstddef>
#include <string>

namespace Hmi::CommandSender {

using TxFn = bool(*)(const char* data, size_t len);  // alternativa opcional

/** Registra un callback alternativo para el envío (tests, mocks…). */
void set_tx_fn(TxFn fn);

/** Envía JSON crudo (c-string). */
bool send_raw_json(const char* json);

/** Envía JSON crudo (std::string). */
bool send_raw_json(const std::string& j);

/** Helper alto nivel: construye y envía un comando DO. */
bool send_do(const char* path, const char* op, bool expects_ack = true);

} // namespace Hmi::CommandSender
