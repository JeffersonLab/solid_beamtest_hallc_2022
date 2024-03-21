#include "EventWrapper.h"

EventWrapper::EventWrapper()
{
    sldata = new Decoder::THaSlotData(3, 20);
    sldata -> define(
            3,  // crate
            20, // slot
            1024, // default maximum channels
            0,  // legacy prameter, not used.
            1   // default number of hits per channel
            );

    // place holder
    fMPD = new Decoder::MPDModule(3, 20);
    fMPD -> Init();
}

void EventWrapper::LoadEvent(const uint32_t *buf, const uint32_t len)
{
    buffer = const_cast<uint32_t*>(buf);
    fBufLen = len;

    if(!fMPD) {
        std::cout<<__func__<<" ERROR: fMPD not initialized..."<<std::endl;
        return;
    }

    fMPD -> Clear();
    sldata -> clearEvent();
    fMPD -> LoadSlot(sldata, buffer, 0, fBufLen);
}


EventWrapper::~EventWrapper()
{
    // place holder
}

UInt_t EventWrapper::GetNumHits(UInt_t crate, UInt_t slot, UInt_t chan) const
{
    // Number hits in crate, slot, channel
    assert(sldata != nullptr);
    if (sldata)
        return sldata->getNumHits(chan);
    return 0;
}

// number of raw words in crate, slot
UInt_t EventWrapper::GetNumRaw(UInt_t crate, UInt_t slot) const
{
    // Number of raw words in crate, slot
    assert(sldata != nullptr);
    if (sldata)
        return sldata->getNumRaw();
    return 0;
}

// raw words for hit 0,1,2.. on crate, slot
UInt_t EventWrapper::GetRawData(UInt_t crate, UInt_t slot, UInt_t hit) const
{
    // Raw words in crate, slot
    assert(sldata);
    return sldata->getRawData(hit);
}

// To retrieve data by crate, slot, channel, and hit# (hit=0,1,2,..)
UInt_t EventWrapper::GetRawData(UInt_t crate, UInt_t slot, UInt_t chan, UInt_t hit) const
{
    // Return the Rawdata in crate, slot, channel #chan and hit# hit
    assert(sldata);
    return sldata->getRawData(chan, hit);
}

// To get element #i of the raw evbuffer
UInt_t EventWrapper::GetRawData(UInt_t i) const
{
    // Raw words in evbuffer at location #i.
    assert(buffer && i < GetEvLength());
    return buffer[i];
}

// Get raw element i within crate
UInt_t EventWrapper::GetRawData(UInt_t crate, UInt_t i) const
{
    // Raw words in evbuffer within crate #crate.
    //assert(crate < rocdat.size());
    //return GetRawData(rocdat[crate].pos + i);
    std::cout<<__func__<<" ERROR::: this function is not implemented yet..."<<std::endl;
    exit(0);
    return 0;
}

UInt_t EventWrapper::GetData(UInt_t crate, UInt_t slot, UInt_t chan, UInt_t hit) const
{
    // Return the data in crate, slot, channel #chan and hit# hit
    assert(sldata);
    return sldata->getData(chan, hit);
}

void EventWrapper::print() const
{
    assert(sldata);
    sldata->print();
}
