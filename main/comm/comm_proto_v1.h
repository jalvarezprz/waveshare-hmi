#pragma once
/**
 * @file comm_proto_v1.h   (HMI)
 * @brief Tipos del protocolo de aplicación v1 (CORE ↔ HMI).
 *
 * Framing v1 (ESPNOW):
 *   Header (10 bytes): {ver,type,seq,flags,ts10ms,len}
 *   Envelope: Header + payload (<=240 bytes)
 *
 * Fragmentación JSON (payload type = 0x30):
 *   app_v1_json_chunk_hdr_t (10 bytes): {msg_seq,chunk_idx,chunk_count,chunk_len,crc16}
 *   seguido de chunk_len bytes de JSON.
 *
 * CRC16=CRC-16/IBM (reflected) poly=0x8005 (0xA001), init=0xFFFF, xorout=0x0000
 */

#include <stdint.h>
#include <stddef.h>

#if defined(__GNUC__)
#  define APP_PACKED __attribute__((packed))
#else
#  define APP_PACKED
#endif

namespace AppV1 {

/** Tipos de mensaje de aplicación (mantener sincronizados con CORE). */
enum : uint8_t {
    CmdLed      = 0x01,
    ReqSnapshot = 0x02,
    Telemetry   = 0x10,
    Heartbeat   = 0x11,
    JsonBridge  = 0x30,   ///< Payload=fragmentos JSON con app_v1_json_chunk_hdr_t + datos
    Ack         = 0xF0,
    Error       = 0xF1,
};

/** Alias usado por código existente. */
static constexpr uint8_t APP_V1_TYPE_JSON_BRIDGE = JsonBridge;

/** Cabecera v1 (10 bytes exactos). */
struct APP_PACKED Header {
    uint8_t  ver;      ///< debe ser 1
    uint8_t  type;     ///< AppV1::<tipo>
    uint8_t  seq;      ///< nº de secuencia (8 bits)
    uint8_t  flags;    ///< reservado
    uint32_t ts10ms;   ///< timestamp en unidades de 10 ms (LSB-first)
    uint16_t len;      ///< longitud del payload (0..240) (LSB-first)
};
static_assert(sizeof(Header) == 10, "AppV1::Header must be 10 bytes");

/** Tamaño máximo del payload v1 (bytes) => 250 total - 10 de cabecera. */
static constexpr uint16_t APP_V1_MAX_PAYLOAD = 240;

/** (Opcional) Sobre v1 completo. */
struct APP_PACKED Envelope {
    Header  hdr;
    uint8_t payload[APP_V1_MAX_PAYLOAD];
};
static_assert(sizeof(Envelope) == 250, "AppV1::Envelope must be 250 bytes");

/**
 * Cabecera de cada fragmento JSON (10 bytes exactos).
 * CRC16-IBM calculado sobre:
 *   - los primeros 8 bytes de este header (sin el propio crc16)
 *   - seguidos de los 'chunk_len' bytes del JSON.
 */
struct APP_PACKED app_v1_json_chunk_hdr_t {
    uint16_t msg_seq;     ///< Secuencia del mensaje JSON completo
    uint16_t chunk_idx;   ///< Índice del fragmento [0..chunk_count-1]
    uint16_t chunk_count; ///< Nº total de fragmentos
    uint16_t chunk_len;   ///< Bytes de JSON que siguen a este header
    uint16_t crc16;       ///< CRC16-IBM sobre (este header sin crc16) + datos
};
static_assert(sizeof(app_v1_json_chunk_hdr_t) == 10, "json chunk hdr must be 10 bytes");

/** Valida y obtiene Header desde un buffer RX. */
inline bool parse_header_ok(const void* data, size_t rxlen, const Header*& out)
{
    if (!data) return false;
    if (rxlen < sizeof(Header)) return false;
    const Header* h = reinterpret_cast<const Header*>(data);

    if (h->ver != 1)      return false;
    if (h->len > APP_V1_MAX_PAYLOAD) return false;
    if (rxlen < (sizeof(Header) + static_cast<size_t>(h->len))) return false;

    out = h;
    return true;
}

/* ======================= CRC16-IBM (reflected) ===========================
 * Parámetros: width=16, poly=0x8005 (reflected=0xA001), init=0xFFFF,
 * refin=true, refout=true, xorout=0x0000.
 * Equivalente a "CRC-16/ARC".
 * ======================================================================= */
inline uint16_t crc16_le_IBM(const uint8_t* data, size_t len, uint16_t init = 0xFFFF)
{
    uint16_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(data[i]);
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 0x0001) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
    }
    return crc; // xorout=0x0000
}

/** CRC de un fragmento JSON tal y como lo verifica el receptor. */
inline uint16_t json_fragment_crc16(const void* frag_hdr_ptr,
                                    const uint8_t* json_part,
                                    uint16_t part_len)
{
    const auto* fh = reinterpret_cast<const app_v1_json_chunk_hdr_t*>(frag_hdr_ptr);

    // 8 bytes del header sin el campo crc16
    uint16_t crc = crc16_le_IBM(reinterpret_cast<const uint8_t*>(fh),
                                sizeof(app_v1_json_chunk_hdr_t) - sizeof(fh->crc16),
                                0xFFFF);
    // Después, los bytes del fragmento
    crc = crc16_le_IBM(json_part, part_len, crc);
    return crc;
}

/** Helper opcional para log (sin dependencias de ESP-IDF aquí). */
inline void format_json_chunk_hdr(const app_v1_json_chunk_hdr_t& h, char* out, size_t n)
{
    // No usamos printf aquí para no acoplar, el receptor hará su log.
    if (!out || n == 0) return;
    // Formato compacto: msg_seq/chunk_idx/chunk_count chunk_len crc16
    // (la conversión real a texto mejor hacerla en el .cpp usando snprintf)
    out[0] = 0;
}

} // namespace AppV1
