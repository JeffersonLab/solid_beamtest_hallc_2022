#include "SSPDecoder.h"

using namespace ssp;

inline void print_word(uint32_t word)
{
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << word << std::dec << "\n";
}

void SSPDecoder::DecodeEvent(SSPEvent &event, const uint32_t *buf, size_t buflen)
const
{
    event.Clear();

    if (!buflen) {
        return;
    }

    uint32_t header = buf[0];
    if (!(header & 0x80000000) || (((header >> 27) & 0xF) != EventHeader)) {
        std::cout << "SSPDecoder Error: incorrect event header:";
        print_word(header);
        return;
    }

    int tag_idx=0;
    int timeH=0;
    int timeL=0;
    unsigned long int timestamp=0;
    int nevent=0;


    uint32_t type = FillerWord;
    uint32_t slot = (header >> 22) & 0x1F;

    for (size_t iw = 1; iw < buflen; ++iw) {
        uint32_t data = buf[iw];

        bool new_type = (data & 0x80000000);
        if (new_type) {
            type = (data >> 27) & 0xF;
            tag_idx = 0;
        }else{// Data type continuation, bit 31 = 0
            tag_idx++;
        }

        uint32_t trigNum, dev_id, ievent_cnt;
        switch (type) {
        case BlockTrailer:
             break; 
        case EventHeader:
             trigNum = (data >>  0 ) & 0x3FFFFF;
             event.tTrigNum=trigNum;
             nevent ++;
             break;
        case TriggerTime:
            if(tag_idx == 0)
            {
              timeH = (data >> 0) & 0xFFFFFF;
              timestamp=0;
              timestamp |= timeH;
            }
            else if(tag_idx == 1)
            {
              timeL = (data >> 0) & 0xFFFFFF;
              timestamp |= (timeL << 24);
            }
            event.tTrigTime= timestamp/125000000.;


            // std::cout << "Trigger Time: " << (data & 0xFFFFFF) << "\n";
            break;
        case DeviceID:
            dev_id = (data >> 22) & 0x1F;
            ievent_cnt = (data >> 0) & 0x3FFFFF;
            // std::cout << "Device ID: " << dev_id << "\n";
            break;
        case TDCWord:
            {
                uint32_t edge = (data >> 26) & 0x1;
                //uint32_t edge = (data >> 26) & 0x01;
                uint32_t fpga = (data >> 16) & 0xFF;
                uint32_t time = (data >> 0 ) & 0x7FFF;
                //uint32_t time = (data >> 0 ) & 0xFFFF;
                uint32_t channel = fpga;
                //uint32_t channel = dev_id * 192 + fpga;
                size_t i = event.Nedges;
                event.channel[i] = channel;
                event.edge[i] = edge;
                event.fiber[i] = dev_id;
                event.slot[i] = slot;
                event.time[i] = time;
                event.Nedges++;
                // std::cout << "Channel " << channel << ": " << edge << ", " << time << "\n";
            }
            break;
        default:
            break;
        }
    }
    //std::cout<<nevent<<std::endl;
}
