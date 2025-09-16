#include "comm_tx_api.h"
#include "comm_tx_queue.h"
#include "comm_commitment.h"

namespace {
    uint8_t g_seq{0};
    inline uint8_t next_seq() { return ++g_seq; }
}

namespace CommTxApi {
bool init() { return true; }

bool sendLed(uint8_t idx, bool on) {
    AppEnvelope env{};
    env.header.ver   = APP_PROTO_VER;
    env.header.type  = static_cast<uint8_t>(AppMsgType::CmdLed);
    env.header.seq   = next_seq();
    env.header.flags = static_cast<uint8_t>(AppMsgFlags::ReqAck);
    env.header.ts10ms= 0;
    env.header.len   = sizeof(AppCmdLedPayload);
    auto* p = reinterpret_cast<AppCmdLedPayload*>(env.payload.data());
    p->idx = idx; p->state = on ? 1 : 0;
    return comm_tx_queue_send(env);
}

bool requestSnapshot() {
    AppEnvelope env{};
    env.header.ver   = APP_PROTO_VER;
    env.header.type  = static_cast<uint8_t>(AppMsgType::ReqSnapshot);
    env.header.seq   = next_seq();
    env.header.flags = 0;
    env.header.ts10ms= 0;
    env.header.len   = 0;
    return comm_tx_queue_send(env);
}
}
