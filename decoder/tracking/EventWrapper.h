#ifndef EVENT_WRAPPER_H
#define EVENT_WRAPPER_H

#include <TVector3.h>
#include "EvChannel.h"
#include "EvStruct.h"
#include "THaEvData.h"
#include "THaSlotData.h"
#include "CodaDecoder.h"
#include "MPDModule.h"
#include <iostream>

/*
// event types
enum EvType {
    CODA_PRST = 0xffd1,
    CODA_GO = 0xffd2,
    CODA_END = 0xffd4,
    CODA_PHY1 = 0xff50,
    CODA_PHY2 = 0xff70,
    CODA_EPICS = 0x83,
};
*/

class EventWrapper
{
public:
    EventWrapper();
    ~EventWrapper();

    void LoadEvent(const uint32_t *buf, const uint32_t len);

    UInt_t GetNumHits(UInt_t crate, UInt_t slot, UInt_t chan) const;
    // number of raw words in crate, slot
    UInt_t GetNumRaw(UInt_t crate, UInt_t slot) const;
    // raw words for hit 0,1,2.. on crate, slot
    UInt_t GetRawData(UInt_t crate, UInt_t slot, UInt_t hit) const;
    // To retrieve data by crate, slot, channel, and hit# (hit=0,1,2,..)
    UInt_t GetRawData(UInt_t crate, UInt_t slot, UInt_t chan, UInt_t hit) const;
    // To get element #i of the raw evbuffer
    UInt_t GetRawData(UInt_t i) const;
    // Get raw element i within crate
    UInt_t GetRawData(UInt_t crate, UInt_t i) const;
    UInt_t GetData(UInt_t crate, UInt_t slot, UInt_t chan, UInt_t hit) const;
    UInt_t GetEvNum() const { return event_num; }
    UInt_t GetEvLength() const { return fBufLen; }
    void print() const;

private:
    int event_num = 0;

    uint32_t fBufLen;
    uint32_t *buffer;

    Decoder::MPDModule *fMPD = nullptr;
    Decoder::THaSlotData *sldata = nullptr;
};

#endif
