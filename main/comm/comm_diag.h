#pragma once
#include <cstdint>

/** Contadores de comunicaciones (acumulados desde boot). */
namespace CommDiag {
    void  reset();
    void  incTxQueued();
    void  incTxLoopback();
    void  incTxOk();
    void  incTxFail();
    void  incRxPkt();
    void  incRxDropped();
    void  incRxAck();
    void  setLastSeq(uint8_t seq);

    struct Snapshot {
        uint32_t txQueued;
        uint32_t txLoopback;
        uint32_t txOk;
        uint32_t txFail;
        uint32_t rxPkt;
        uint32_t rxDropped;
        uint32_t rxAck;
        uint8_t  lastSeq;
    };

    Snapshot get();
}
