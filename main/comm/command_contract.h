#pragma once
#include <string>
#include <cstdint>

/**
 * Contrato de órdenes HMI → CORE (v0.1)
 * - Envelope fijo:
 *    schema="hvac.v0.1", kind="command", topic="control/do"
 * - Body mínimo:
 *    { "do": "<path>", "op": "<on|off|toggle>", "args": { ... } }
 *
 * Helper principal: make_command_do(path, op, expects_ack)
 * Devuelve el JSON listo para enviar por el canal actual (ESPNOW → JSON_BRIDGE).
 *
 * Nota: para simplificar (y evitar dependencias), el JSON se construye como string.
 *       Asumimos que path/op no llevan caracteres especiales que requieran escape.
 */
namespace Comm::CommandContract {

constexpr const char* kSchema          = "hvac.v0.1";
constexpr const char* kKindCommand     = "command";
constexpr const char* kTopicControlDo  = "control/do";

/** Genera un msg_id único con prefijo HC- (p.ej., "HC-00001234"). */
std::string new_msg_id();

/** Timestamp en milisegundos desde boot (uint64 → decimal). */
uint64_t ts_ms();

/**
 * Construye el JSON de comando DO.
 * @param path         Ruta, p.ej. "io/led_builtin"
 * @param op           "on" | "off" | "toggle"
 * @param expects_ack  true si se espera ACK del CORE
 * @param msg_id_opt   si no es nullptr, se usa tal cual; si es nullptr, se genera
 * @return             JSON textual
 */
std::string make_command_do(const char* path,
                            const char* op,
                            bool expects_ack = true,
                            const char* msg_id_opt = nullptr);

/** Variante que solo devuelve el body (útil para tests o logging). */
std::string make_body_do(const char* path, const char* op);

} // namespace Comm::CommandContract
