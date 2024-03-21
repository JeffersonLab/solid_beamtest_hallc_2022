#ifndef MPDModule_
#define MPDModule_

/////////////////////////////////////////////////////////////////////
//
//   MPDModule
//   This is the MPD INFN (GEM readout for SBS) module decoder
//   Note, it must inherit from VmeModule.
//   Feel free to copy this and make appropriate changes.
//
//   other steps
//   1. Register (add "DoRegister" call and include the header)
//   note: the number (4444) that is registered must appear in db_cratemap.dat
//   2. Add to namespace Decoder.h
//   3. Add to Makefile
//   4. Add to haDecode_LinkDef.h
//   5. Add line(s) for [crate,slot] in db_cratemap.dat
//
//   if the pre-compiler flag "LIKEV792" is defined, the decoding is
//   sort of like a V792 ... as an example.
//
/////////////////////////////////////////////////////////////////////

//#define LIKEV792x 0

// (jc2) What was this used for? Seems to conflict with a value defined
// in Caen775. So I've commented out.
//#define NTDCCHAN   32
#define MAXHIT 2048

#include "VmeModule.h"

namespace Decoder
{
    class MPDModule : public VmeModule
    {
    public:
        MPDModule() = default;
        MPDModule(Int_t crate, Int_t slot);
        virtual ~MPDModule();

        using VmeModule::GetData;
        using VmeModule::LoadSlot;

        virtual UInt_t GetData(UInt_t adc, UInt_t sample, UInt_t chan) const;
        virtual void Init();
        virtual void Init(const char *configstr);
        virtual void Clear(const Option_t *opt = "");
        virtual Int_t Decode(const UInt_t *p); // { return 0; };

        //#ifdef LIKEV792x
        // Loads slot data.  if you don't define this, the base class's method is used
        virtual UInt_t LoadSlot(THaSlotData *sldat, const UInt_t *evbuffer, UInt_t pos, UInt_t len);
        //#endif

    private:
        bool fOnlineZeroSuppression; // if true, assumes that raw-data are already zero-suppressed and baseline-subtracted

        // configuration parameters
        Int_t fAcqMode;      // normal, zero suppression, histogram, synch ...
        Int_t fSamplePeriod; // 25 ns, 75 ns ...
        Int_t fNumSample;    // number of sample / event

        Int_t fNumADC; // number of ADC fifos (number of front end cards served by the MPD)

        // current indices
        Int_t fIdxA; // ADC
        Int_t fIdxS; // Sample
        Int_t fIdxC; // Channel

        Int_t fIdxMPD; // MPD ID

        Int_t fCountS; // Sample Counter from electronics
        Int_t fCountW; // Word

        Int_t fNumHits;
        //
        UInt_t fBlockHeader;    // Default = 0
        UInt_t fBlockTrailer;   // Default = 1
        UInt_t fEventHeader;    // Default = 2
        UInt_t fTriggerTime;    // Default = 3
        UInt_t fMPDFrameHeader; // Default = 5
        UInt_t fMPDEventInfo;   // Default = 12
        UInt_t fMPDDebugHeader; // Default = 13 (unclear if we will need to care about this
        UInt_t fDataNotValid;   // Default = 14
        UInt_t fFillerWord;     // Default = 15;
        // UInt_t fAPVHeader;   //Default = 0x4

        UInt_t fSLOTID_VTP; // default = 11

        // Default "reference channel" for common-mode flags:
        UInt_t fChan_CM_flags;       // default = 512
        UInt_t fChan_TimeStamp_low;  // default = 513
        UInt_t fChan_TimeStamp_high; // default = 514
        UInt_t fChan_MPD_EventCount; // default = 515
        UInt_t fChan_MPD_Debug;      // default = 516

        static TypeIter_t fgThisType;
    };
}

#endif
