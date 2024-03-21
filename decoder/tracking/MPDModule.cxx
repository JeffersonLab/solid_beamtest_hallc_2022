/////////////////////////////////////////////////////////////////////
//
//   MPDModule
//   This is the MPD module decoder; based on SkeletonModule
//   (https://github.com/JeffersonLab/analyzer)
//
//   E. Cisbani
//   Original Version:   2015/Dec
//
//   v5 Version based on documentation by Paulo Musico
//   Seamus Riordan
//   sriordan@anl.gov
//   Aug 31, 2018
//
//   v5 Version with online SSP zero supression, based on documentation
//   from Ben (DAQ group) which I got from Danning Di.
//   Juan Carlos Cornejo <cornejo@jlab.org> - 2018/10/23
//
/////////////////////////////////////////////////////////////////////

/*
  #define MPD_VERSION_TAG 0xE0000000
  #define MPD_EVENT_TAG   0x10000000
  #define MPD_MODULE_TAG  0x20000000
  #define MPD_ADC_TAG     0x30000000
  #define MPD_HEADER_TAG  0x40000000
  #define MPD_DATA_TAG    0x0
  #define MPD_TRAILER_TAG 0x50000000
*/

// #define SSP_DATADEF(b) ((b&0x80000000)>>31)
// #define SSP_TAG(b)     ((b&0x78000000)>>27)
// #define SSP_SAMPLE(b,c) ((b>>c)&0xFFF)|(((b>>c)&0x1000)?0xFFFFF000:0x0)

#include "MPDModule.h"
#include "THaSlotData.h"
#include "THaCrateMap.h"
#include <limits>
#include <vector>
#include <map>
#include <set>
#include <iostream>

using namespace std;

namespace Decoder
{
    Module::TypeIter_t MPDModule::fgThisType =
        DoRegister(ModuleType("Decoder::MPDModule", 3561));

    MPDModule::MPDModule(Int_t crate, Int_t slot) : VmeModule(crate, slot)
    {
        fDebugFile = nullptr;
        Init(); // Should this be called here? not clear...
    }

    MPDModule::~MPDModule()
    {
    }

    void MPDModule::Init()
    {
        VmeModule::Init();
        //    Config(0,25,6,16,128); // should be called by the user (but how?)
        fDebugFile = nullptr;
        Clear();
        //    fName = "MPD Module (INFN MPD for GEM and more), use Config to dynamic config";
        fName = "MPD Module";

        fOnlineZeroSuppression = false; // If this is false, then we want to calculate and subtract the common-mode from each ADC sample:
        fBlockHeader = 0;               // 0000 = 0
        fBlockTrailer = 1;              // 0001 = 1
        fEventHeader = 2;               // 0010 = 2
        fTriggerTime = 3;               // 0011 = 3
        fMPDFrameHeader = 5;            // 0101 = 5
        fMPDEventInfo = 12;             // 1100 = 12
        fMPDDebugHeader = 13;           // 1101 = 13
        fDataNotValid = 14;             // 1110 = 14
        fFillerWord = 15;               // 1111 = 15
        //    fAPVHeader   = 0x4;

        fSLOTID_VTP = 20; // GEn-II is 11, hall c is 20

        fNumSample = 6;

        fChan_CM_flags = 640; // reference channel for common-mode and zero suppression flags
        fChan_TimeStamp_low = 641;
        fChan_TimeStamp_high = 642;
        fChan_MPD_EventCount = 643;
        fChan_MPD_Debug = 644;
    }

    void MPDModule::Init(const char *configstr)
    { // parse (optional) configuration parameters:
        Init();
        // allow users to configure the various dummy/reference channels via the crate map:
        vector<ConfigStrReq> req = {{"chan_cmflags", fChan_CM_flags},
                                    {"chan_timestamp_low", fChan_TimeStamp_low},
                                    {"chan_timestamp_high", fChan_TimeStamp_high},
                                    {"chan_event_count", fChan_MPD_EventCount},
                                    {"chan_MPD_debug", fChan_MPD_Debug}};
        ParseConfigStr(configstr, req);

        assert(fChan_CM_flags < THaCrateMap::MAXCHAN);
        assert(fChan_TimeStamp_low < THaCrateMap::MAXCHAN);
        assert(fChan_TimeStamp_high < THaCrateMap::MAXCHAN);
        assert(fChan_MPD_EventCount < THaCrateMap::MAXCHAN);
        assert(fChan_MPD_Debug < THaCrateMap::MAXCHAN);
    }

    // This version ASSUMES that there is no online zero suppression, so all 128 APV channels are present in every event!

    UInt_t MPDModule::LoadSlot(THaSlotData *sldat, const UInt_t *evbuffer, UInt_t pos, UInt_t len)
    {
        //std::cout << "Calling MPDModule::LoadSlot()... (pos, len) = " << pos << ", " << len << "..."<<std::endl;
        // AJRP: LoadSlot method for MPD SSP data format to be used in Hall A during the GMN run:
        const UInt_t *datawords = &(evbuffer[pos]);

        fWordsSeen = 0;

        Int_t status;

        UInt_t thisword;

        UInt_t iword = 0;

        // Get the slot number for this call to LoadSlot:
        UInt_t this_slot = sldat->getSlot();
        //std::cout<<"xinzhan debug: this_slot = "<<this_slot<<std::endl;
        // UInt_t this_crate = sldat->getCrate();

        // UInt_t mask, shift;
        UInt_t slot = MAXSLOT + 1, apv_id = 0, apv_chan = 0, fiber = 0;
        //std::cout<<"xinzhan debug: slot = "<<slot<<std::endl;
        // UInt_t mpd_id=0, apv_chan_40, apv_chan65;

        // UInt_t prev_slot=0;

        UInt_t effChan = 0;
        // UInt_t effChan_old = MAXCHAN+1; //set "old" effective channel to something impossible so that the first channel will always trigger "loading" of the cm_flags:

        // The use of a map here insures that the cm_flags will only be "loaded" into the "slot" once per APV card:
        std::map<UInt_t, UInt_t> cm_flags_vs_chan;                  // key = "effective channel", mapped value = common-mode flags (and possibly other information)
        std::map<UInt_t, UInt_t> TimeStampL_vs_fiber;               // least significant 24 bits of time stamp
        std::map<UInt_t, UInt_t> TimeStampH_vs_fiber;               // most significant 24 bits of time stamp
        std::map<UInt_t, UInt_t> EventCount_vs_fiber;               // 20-bit "event count" variable:
        std::map<UInt_t, std::vector<UInt_t>> MPDdebugInfo_vs_chan; // TBD: key = effective channel, mapped value = vector of (3) data words encoding calculated common-mode corrections for six time samples:

        UInt_t TIMESTAMP_LO = 0, TIMESTAMP_HI = 0, EVENT_COUNT = 0;
        UInt_t eventinfo_wordcount = 0;

        bool CM_OR = false;
        bool ENABLE_CM = false;
        bool BUILD_ALL_SAMPLES = true;

        //bool is_SSP = false; // assume VTP by default which only has the MPD data, no block header/trailer.
        bool is_SSP = true; // for hall c setup, this must be true, (hall c is using ssp)

        bool found_this_slot = false;
        bool found_MPD_header = false;

        UInt_t mpd_strip_count = 0;
        UInt_t mpd_word_count = 0;

        // UInt_t old_type_tag=16;
        UInt_t type_tag = 16; // intialize to something that is NOT one of the recognized data types

        // following the MPDRawParser in ROOT_GUI_multicrate, loop on all the data in the ROC bank (which corresponds to one "crate"), and populate the
        // temporary data structure above, ONLY if slot == this_slot
        //  Since all the data from one slot is (or should be) in a contiguous block, we should

        // temporary storage for information needed to store "strip hits"
        // UInt_t
        UInt_t ADCsamples[fNumSample]; // temporary storage for ADC time samples. will this compile? Hope so...

        UInt_t hitwords[3]; // temporary storage for the three words needed to extract the information for one "hit"

        UInt_t CMwordcount = 0;
        UInt_t CMwords[3]; // temporary storage for the three words containing the common-mode values for the six time samples of an APV:

        while (iword < len)
        {
            thisword = datawords[iword++];

            // check whether this is a data-type defining or data-type continuation word:
            UInt_t word_type = (thisword & 0x80000000) >> 31;

            if (word_type == 1)
            { 
                // data-type defining: extract data type from bits 30-27:
                // old_type_tag = type_tag;
                type_tag = (thisword & 0x78000000) >> 27;
                //std::cout << "Data-type defining word, type tag, fBlockHeader = " << type_tag << ", " << fBlockHeader << std::endl;
                if (type_tag == fBlockHeader)
                { 
                    // if we see a block header, this is SSP data:
                    is_SSP = true;

                    found_MPD_header = false;

                    // prev_slot = slot;
                    // extract "SLOTID" from bits 26-22 and compare to this_slot
                    slot = (thisword & 0x07C00000) >> 22; // 7C = 0111 1100
                    if (slot == this_slot)
                        found_this_slot = true;

                    // std::cout << "Found block header, SLOTID = " << slot << std::endl;
                }

                if (type_tag == fBlockTrailer)
                {                   // end of a block of SSP data. set the is_SSP flag to false unless and until we see another block header word
                    is_SSP = false; //
                    found_MPD_header = false;
                }

                // Question: ask Ben: how would/could we use the trigger time words to correct for APV trigger jitter?

                if (type_tag == fMPDFrameHeader)
                { 
                    // extract "flags", FIBER, and MPD_ID
                    // TO-DO: figure out how to store this information in the slot data
                    //(maybe define some arbitrary "dummy" channel to hold this info, as well as trigger time words)
                    ENABLE_CM = TESTBIT(thisword, 26);
                    BUILD_ALL_SAMPLES = TESTBIT(thisword, 25);
                    CM_OR = TESTBIT(thisword, 24);

                    // NEW firmware to support up to 40 MPDs per VTP
                    // to extract bits 21-16:
                    //  0x003F0000 = 0000 0000 0011 1111 0000 0000 0000 0000

                    // FIBER number was in bits 20-16:
                    // fiber = (thisword & 0x001F0000)>>16;

                    // NOW the fiber number is in bits 21-16:
                    fiber = (thisword & 0x003F0000) >> 16;

                    // MPD ID is in bits 0-4, but we basically ignore it:
                    // mpd_id = (thisword & 0x0000001F);

                    found_MPD_header = true;

                    // now how should we "load" the data into the "slot"?
                    // Each "hit" in this channel
                    //  UInt_t flags = 2*ENABLE_CM + BUILD_ALL_SAMPLES;

                    // status = sldat->loadData( fChan_CM_flags, flags, fiber );
                    // std::cout << "found MPD frame header, fiber, mpd_id, ENABLE_CM, BUILD_ALL_SAMPLES, is_SSP, SLOT, THIS_SLOT = " << fiber << ", " << mpd_id << ", "
                    // 	    << ENABLE_CM << ", " << BUILD_ALL_SAMPLES << ", "
                    // 	    << is_SSP << ", " << slot << ", " << this_slot << std::endl;

                    // reset "word" and "strip" counters:
                    mpd_word_count = 0;
                    mpd_strip_count = 0;

                    if (!is_SSP)
                    {
                        slot = fSLOTID_VTP; // always 11
                        found_this_slot = (slot == this_slot);
                    }
                }

                if (type_tag == fMPDEventInfo)
                { // if we see this, next three words give coarse timestamp, fine timestamp, and event counter:
                    eventinfo_wordcount = 1;
                    // The lower 16 bits of coarse timestamp and the 8-bit fine timestamp are in
                    //  bits 0-23 of thisword:
                    TIMESTAMP_LO = thisword & 0x00FFFFFF;
                }

                if (type_tag == fMPDDebugHeader)
                {
                    CMwordcount = 0;
                    CMwords[CMwordcount] = thisword;
                }
                // std::cout<<"xinzhan debug: found_this_slot: "<<found_this_slot<<std::endl;
            }

            else if (found_this_slot)
            {
                // data-type continuation: behavior depends on type_tag. If the most recently found "slot"
                // doesn't match the one that we want, then do nothing.
                // For NOW, let's focus mainly on the MPD Frame decoding and worry about anything and everything else later:

                if (type_tag == fMPDFrameHeader && found_MPD_header)
                {
                    // the data continuation words should come in bundles of three * N, where N is the total number of "hits" (i.e., fired strips) in the MPD:

                    hitwords[mpd_word_count % 3] = thisword; // Set hitwords before incrementing word count

                    mpd_word_count++; // increment word count;

                    mpd_strip_count = mpd_word_count / 3;

                    // this should take care of the issue of missing the last "hit":
                    if (mpd_word_count % 3 == 0 && mpd_strip_count > 0)
                    {
                        // extract information from the three "hit words" and "load" the data into the "slot":
                        // load up the ADC samples:
                        for (int iw = 0; iw < 3; iw++)
                        {
                            // In words: take the bitwise OR of the first 12 bits of hitwords[iw] with 0xFFFFF000 or 0x0
                            // depending on the value of bit 12 (13th bit) of hitwords[iw].
                            // This is essentially implementing the two's complement representation of a 13-bit signed integer
                            ADCsamples[2 * iw] = (hitwords[iw] & 0xFFF) | ((hitwords[iw] & 0x1000) ? 0xFFFFF000 : 0x0);
                            ADCsamples[2 * iw + 1] = ((hitwords[iw] >> 13) & 0xFFF) | (((hitwords[iw] >> 13) & 0x1000) ? 0xFFFFF000 : 0x0);
                        }

                        // APV_ID is in bits 26-30 of hitword[2]:
                        //  0x 0111 1100 .... = 0x7C000000
                        apv_id = (hitwords[2] & 0x7C000000) >> 26;

                        // APV channel num (bits 4:0) is in bits 26:30 of hitword[0];
                        // APV channel num (bits 6:5) is in bits 26:30 of hitword[1] (but we actually only want bits 26 and 27 AFAIK:
                        UInt_t apv_chan40 = (hitwords[0] & 0x7C000000) >> 26;
                        UInt_t apv_chan65 = (hitwords[1] & 0x0C000000) >> 26;
                        apv_chan = (apv_chan65 << 5) | apv_chan40;

                        effChan = (fiber << 4) | apv_id;

                        UInt_t cm_flags = 4 * CM_OR + 2 * ENABLE_CM + BUILD_ALL_SAMPLES;

                        cm_flags_vs_chan[effChan] = cm_flags;

                        for (int is = 0; is < 6; is++)
                        {
                            // if( ADCsamples[is] > 0xFFF && !ENABLE_CM ) {
                            // 	std::cout << "negative ADC sample encountered when CM not enabled, raw data word = " << ADCsamples[is] << ", signed int representation = " << Int_t(ADCsamples[is]) << std::endl;
                            // 	std::cout << "This is not expected" << std::endl;
                            // }
                            // This loads each of the six ADC samples as a new "hit" into sldat, with "effChan" as the unique "channel" number,
                            // the ADC samples as the "data", and the APV channel number as the "rawData"
                            // std::cout << "decoded one strip hit: (crate, slot, fiber, apv_id, apv_chan, effChan, isample, ADCsamples[isample]) = ("
                            // 		<< sldat->getCrate() << ", " << slot << ", " << fiber << ", " << apv_id << ", " << apv_chan << ", " << effChan << ", "
                            // 		<< is <<  ", " << int(ADCsamples[is]) << ")" << std::endl;

                            // Since we need only two bits to encode the ENABLE_CM and BUILD_ALL_SAMPLES flags, we can

                            status = sldat->loadData("adc", effChan, ADCsamples[is], apv_chan);
                            //std::cout<<"xb debug: "<<ADCsamples[is]<<", apv_chan: "<<apv_chan<<std::endl;
                            if (status != SD_OK)
                                return -1;
                        }
                    }
                }

                if (type_tag == fMPDEventInfo && found_MPD_header)
                {
                    if (eventinfo_wordcount == 1)
                    { // upper 24 bits of the coarse timestamp:
                        TIMESTAMP_HI = thisword & 0x00FFFFFF;
                        eventinfo_wordcount++;
                    }
                    else if (eventinfo_wordcount == 2)
                    {
                        // event counter is in the first 20 bits:
                        EVENT_COUNT = thisword & 0x000FFFFF;

                        // fill map with most recently reported MPD fiber number as the key:
                        TimeStampL_vs_fiber[fiber] = TIMESTAMP_LO;
                        TimeStampH_vs_fiber[fiber] = TIMESTAMP_HI;
                        EventCount_vs_fiber[fiber] = EVENT_COUNT;

                        eventinfo_wordcount = 0;
                    }
                }

                if (type_tag == fMPDDebugHeader && found_MPD_header)
                {
                    CMwordcount++;
                    CMwords[CMwordcount % 3] = thisword;
                    if (CMwordcount == 2)
                    {
                        MPDdebugInfo_vs_chan[effChan].clear();
                        for (int iw = 0; iw < 3; iw++)
                        {
                            MPDdebugInfo_vs_chan[effChan].push_back(CMwords[iw]);
                        }
                    }
                }
            }

            fWordsSeen++;
        }

        // std::cout << "Loading common-mode flags for this event: " << std::endl;
        // now load the cm flags:
        for (auto iapv = cm_flags_vs_chan.begin(); iapv != cm_flags_vs_chan.end(); ++iapv)
        {
            // std::cout << "effChan = " << iapv->first << ", ENABLE_CM = " << (iapv->second / 2) << ", BUILD_ALL_SAMPLES = " << (iapv->second % 2 ) << std::endl;
            // The "flags" word is the mapped value (iapv->second)
            // The effective channel is the key (iapv->first)
            sldat->loadData(fChan_CM_flags, iapv->second, iapv->first);
        }

        // It is ASSUMED that time stamp low, time stamp high, and event count
        for (auto ifiber = TimeStampL_vs_fiber.begin(); ifiber != TimeStampL_vs_fiber.end(); ++ifiber)
        {
            UInt_t fiber = ifiber->first;
            sldat->loadData(fChan_TimeStamp_low, TimeStampL_vs_fiber[fiber], fiber);
            sldat->loadData(fChan_TimeStamp_high, TimeStampH_vs_fiber[fiber], fiber);
            sldat->loadData(fChan_MPD_EventCount, EventCount_vs_fiber[fiber], fiber);
        }

        // Now loop over all APVs seen in the data and load the MPD debug headers into the appropriate dummy channel:
        for (auto iapv = MPDdebugInfo_vs_chan.begin(); iapv != MPDdebugInfo_vs_chan.end(); ++iapv)
        {
            UInt_t chan = iapv->first;
            for (UInt_t iw = 0; iw < MPDdebugInfo_vs_chan[chan].size(); iw++)
            {
                sldat->loadData(fChan_MPD_Debug, MPDdebugInfo_vs_chan[chan][iw], chan);
            }
        }

        return fWordsSeen;
    }

    // //Unclear if these are used by anything: comment for now (AJRP)
    UInt_t MPDModule::GetData(UInt_t adc, UInt_t sample, UInt_t chan) const
    {
        return 0;
    }

    void MPDModule::Clear(const Option_t *opt)
    {
        VmeModule::Clear(opt);
    }

    Int_t MPDModule::Decode(const UInt_t *pdat)
    {
        return 0;
    }
}
