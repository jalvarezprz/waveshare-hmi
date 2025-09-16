#include "comm_diag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace CommDiag {
static SemaphoreHandle_t s_mtx = nullptr;
static uint32_t txQueued=0, txLoopback=0, txOk=0, txFail=0, rxPkt=0, rxDropped=0, rxAck=0;
static uint8_t  lastSeq=0;

static inline void ensure() { if(!s_mtx) s_mtx = xSemaphoreCreateMutex(); }

void reset(){ ensure(); xSemaphoreTake(s_mtx, portMAX_DELAY);
  txQueued=txLoopback=txOk=txFail=rxPkt=rxDropped=rxAck=0; lastSeq=0;
  xSemaphoreGive(s_mtx);
}
#define M_INC(x) do{ ensure(); xSemaphoreTake(s_mtx, portMAX_DELAY); ++(x); xSemaphoreGive(s_mtx);}while(0)
#define M_SET(s,v) do{ ensure(); xSemaphoreTake(s_mtx, portMAX_DELAY); (s)=(v); xSemaphoreGive(s_mtx);}while(0)

void incTxQueued(){  M_INC(txQueued); }
void incTxLoopback(){M_INC(txLoopback); }
void incTxOk(){      M_INC(txOk); }
void incTxFail(){    M_INC(txFail); }
void incRxPkt(){     M_INC(rxPkt); }
void incRxDropped(){ M_INC(rxDropped); }
void incRxAck(){     M_INC(rxAck); }
void setLastSeq(uint8_t s){ M_SET(lastSeq, s); }

CommDiag::Snapshot get(){
  ensure(); xSemaphoreTake(s_mtx, portMAX_DELAY);
  Snapshot snap{txQueued,txLoopback,txOk,txFail,rxPkt,rxDropped,rxAck,lastSeq};
  xSemaphoreGive(s_mtx);
  return snap;
}
} // namespace
