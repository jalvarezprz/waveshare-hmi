#include "comm/command_contract.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <mutex>            // <- para std::once_flag / std::call_once
#include "esp_timer.h"
#include "esp_random.h"

namespace Comm::CommandContract {

static std::atomic<uint32_t> g_seq{0};

uint64_t ts_ms() {
    return esp_timer_get_time() / 1000ULL;
}

std::string new_msg_id() {
    // Seed una sola vez (thread-safe) para desalinear secuencias entre resets
    static std::once_flag once;
    std::call_once(once, []{
        uint32_t r = esp_random();
        g_seq.store((r ^ static_cast<uint32_t>(ts_ms())) & 0x00FFFFFFu,
                    std::memory_order_relaxed);
    });

    const uint32_t n = g_seq.fetch_add(1, std::memory_order_relaxed);

    char buf[32];
    // En este toolchain uint32_t == unsigned long → usa %08lu con cast explícito
    std::snprintf(buf, sizeof(buf), "HC-%08lu", static_cast<unsigned long>(n));
    return std::string(buf);
}

static inline void json_append_num(std::string& s, const char* key, uint64_t val, bool last=false) {
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(val));
    s += "\""; s += key; s += "\":";
    s += buf;
    if (!last) s += ",";
}

static inline void json_append_str(std::string& s, const char* key, const char* val, bool last=false) {
    s += "\""; s += key; s += "\":";
    s += "\""; s += (val ? val : ""); s += "\"";
    if (!last) s += ",";
}

// Body mínimo del comando DO
std::string make_body_do(const char* path, const char* op) {
    std::string b;
    b.reserve(96);
    b += "{";
    json_append_str(b, "do", path ? path : "io/unknown");
    json_append_str(b, "op", op ? op : "toggle");
    // args (último campo del body): lo añadimos sin helper para no poner coma extra
    b += "\"args\":{}";
    b += "}";
    return b;
}

std::string make_command_do(const char* path,
                            const char* op,
                            bool expects_ack,
                            const char* msg_id_opt)
{
    const std::string msg_id = msg_id_opt ? std::string(msg_id_opt) : new_msg_id();
    const uint64_t tms = ts_ms();
    const std::string body = make_body_do(path, op);

    std::string j;
    j.reserve(256 + body.size());

    j += "{";
    json_append_str(j, "schema", kSchema);
    json_append_str(j, "msg_id", msg_id.c_str());
    json_append_num(j, "ts_ms", tms);
    json_append_str(j, "src", "HMI");     // informativo
    json_append_str(j, "dst", "CORE");
    json_append_str(j, "kind", kKindCommand);
    json_append_str(j, "topic", kTopicControlDo);
    json_append_num(j, "qos", 0);
    // expects_ack
    j += "\"expects_ack\":";
    j += expects_ack ? "true" : "false";
    j += ",";
    // body (último bloque)
    j += "\"body\":";
    j += body;
    j += "}";

    return j;
}

} // namespace Comm::CommandContract
