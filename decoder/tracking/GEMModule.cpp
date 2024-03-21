#include "GEMModule.h"
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <TGraph.h>
#include <TGraphErrors.h>
#include "Database/Database.h"
#include <TRotation.h>
#include <iomanip>
#include <TFile.h>
#include <TObject.h>

static inline void xb(const char *str = __builtin_FUNCTION())
{
    static int xb_debug_counter = 0;
    xb_debug_counter++;
    std::cout << "++++++++ XB Debug:: Code reached here: " << str << "() " << xb_debug_counter << " ++++++++" << std::endl;
}

std::ostream &operator<<(std::ostream &os, const mpdmap_t &m)
{
    os << std::setfill(' ') << std::setw(4) << m.crate
       << std::setfill(' ') << std::setw(4) << m.slot
       << std::setfill(' ') << std::setw(4) << m.mpd_id
       << std::setfill(' ') << std::setw(4) << m.gem_id
       << std::setfill(' ') << std::setw(4) << m.adc_id
       << std::setfill(' ') << std::setw(4) << m.i2c
       << std::setfill(' ') << std::setw(4) << m.pos
       << std::setfill(' ') << std::setw(4) << m.invert
       << std::setfill(' ') << std::setw(4) << m.axis
       << std::setfill(' ') << std::setw(4) << m.index
       << std::endl;
    return os;
}

GEMModule::GEMModule(const char *name, const char *discription)
{
    fName = name;
    // FIXME:  To database
    // Set Default values for fZeroSuppress and fZeroSuppressRMS:
    fZeroSuppress = kFALSE;
    fZeroSuppressRMS = 5.0; // threshold in units of RMS:

    fNegSignalStudy = kFALSE;

    fPedestalMode = kTRUE;
    fPedHistosInitialized = kFALSE;

    fSubtractPedBeforeCommonMode = false; // only affects the pedestal-mode analysis

    // Default online zero suppression to FALSE: actually I wonder if it would be better to use this in
    //  Moved this to MPDModule, since this should be done during the decoding of the raw APV data:
    fOnlineZeroSuppression = kFALSE;

    fCommonModeFlag = 0;    //"sorting" method
    fCommonModeOnlFlag = 3; // 3 = Danning method during GMn, 4 = Danning method during GEn
    // Default: discard highest and lowest 28 strips for "sorting method" common-mode calculation:
    fCommonModeNstripRejectHigh = 28;
    fCommonModeNstripRejectLow = 28;

    fCommonModeNumIterations = 3;
    fCommonModeMinStripsInRange = 10;
    fMakeCommonModePlots = false;
    fCommonModePlotsInitialized = false;
    fCommonModePlots_DBoverride = false;

    fMakeEventInfoPlots = false;
    fEventInfoPlotsInitialized = false;

    fPedSubFlag = 1; // default to online ped subtraction, as that is the mode we will run in most of the time

    // Set default values for decode map parameters:
    fN_APV25_CHAN = 128;
    fN_MPD_TIME_SAMP = 6;
    fMPDMAP_ROW_SIZE = 9;

    // We should probably get rid of this as it's not used, only leads to confusion:
    fNumberOfChannelInFrame = 129;

    fSamplePeriod = 24.0; // nanoseconds:

    fSigma_hitshape = 0.0004; // 0.4 mm; controls cluster-splitting algorithm
    // for( Int_t i = 0; i < N_MPD_TIME_SAMP; i++ ){
    //     fadc[i] = NULL;
    // }

    // Default clustering parameters:
    fThresholdSample = 50.0;
    fThresholdStripSum = 250.0;
    fThresholdClusterSum = 500.0;

    fADCasymCut = 1.1;
    fTimeCutUVdiff = 30.0;
    fCorrCoeffCut = 0.5;

    fFiltering_flag1D = 0; //"soft" cuts
    fFiltering_flag2D = 0; //"soft" cuts

    // default these offsets to zero:
    fUStripOffset = 0.0;
    fVStripOffset = 0.0;

    fMakeEfficiencyPlots = true;
    fEfficiencyInitialized = false;

    // We want to change the default values for these dummy channels to accommodate up to 40 MPDs per VTP:
    // fChan_CM_flags = 512; //default to 512:
    // fChan_TimeStamp_low = 513;
    // fChan_TimeStamp_high = 514;
    // fChan_MPD_EventCount = 515;
    // fChan_MPD_Debug = 516;

    // Start dummy channels at 640 by default, to accommodate up to 40 MPDs per VTP crate
    fChan_CM_flags = 640; // default to 640 (so as not to step on up to 40 MPDs per VTP crate):
    fChan_TimeStamp_low = 641;
    fChan_TimeStamp_high = 642;
    fChan_MPD_EventCount = 643;
    fChan_MPD_Debug = 644;

    UInt_t MAXNSAMP_PER_APV = fN_APV25_CHAN * fN_MPD_TIME_SAMP;
    // arrays to hold raw data from one APV card:
    fStripAPV.resize(MAXNSAMP_PER_APV);
    fRawStripAPV.resize(MAXNSAMP_PER_APV);
    fRawADC_APV.resize(MAXNSAMP_PER_APV);
    fPedSubADC_APV.resize(MAXNSAMP_PER_APV);
    fCommonModeSubtractedADC_APV.resize(MAXNSAMP_PER_APV);

    fCM_online.resize(fN_MPD_TIME_SAMP);

    // default to
    // fMAX2DHITS = 250000;
    fMAX2DHITS = 100000;

    fRMS_ConversionFactor = sqrt(fN_MPD_TIME_SAMP); //=2.45

    fIsMC = false; // need to set default value!

    fAPVmapping = SBSGEM::kUVA_XY; // default to UVA X/Y style APV mapping, but require this in the database::

    InitAPVMAP();

    fModuleGain = 1.0;
    //  std::cout << "SBSGEMModule constructor invoked, name = " << name << std::endl;

    // Number of sigmas for defining common-mode max for online zero suppression
    fCommonModeRange_nsigma = 5.0;

    fSuppressFirstLast = 0; // suppress strips peaking in first or last time sample by default:
    // fUseStripTimingCuts = false;

    fStripTau = 56.0; // ns, default value. Eventually load this from DB. This is not actually used as of yet.
    fUseStripTimingCuts = false;
    fUseTSchi2cut = false;
    fStripMaxTcut_central = 87.0; // ns
    fStripMaxTcut_width = 25.0;   // ns
    fStripAddTcut_width = 25.0;   // ns
    fStripAddCorrCoeffCut = 0.5;
    fStripTSchi2Cut = 10.0; // not yet clear what is a good value for this.

    fGoodStrip_TSfrac_mean.resize(fN_MPD_TIME_SAMP);
    fGoodStrip_TSfrac_sigma.resize(fN_MPD_TIME_SAMP);

    // Define some defaults for these:
    double fracmean_default[6] = {0.055, 0.135, 0.203, 0.224, 0.208, 0.174};
    double fracsigma_default[6] = {0.034, 0.039, 0.019, 0.021, 0.032, 0.035};

    for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
    {
        if (isamp < fN_MPD_TIME_SAMP && isamp < 6)
        {
            fGoodStrip_TSfrac_mean[isamp] = fracmean_default[isamp];
            fGoodStrip_TSfrac_sigma[isamp] = fracsigma_default[isamp];
        }
    }

    fPulseShapeInitialized = false;

    fMeasureCommonMode = true;
    fNeventsCommonModeLookBack = 15;

    fCorrectCommonMode = false;
    fCorrectCommonModeMinStrips = 20;
    fCorrectCommonMode_Nsigma = 5.0;

    fCommonModeBinWidth_Nsigma = 1.0;  // Bin width +/- 1 sigma by default
    fCommonModeScanRange_Nsigma = 4.0; // Scan window +/- 4 sigma
    fCommonModeStepSize_Nsigma = 0.2;  // sigma/5 for step size:

    fCommonModeDanningMethod_NsigmaCut = 3.0; // Default to 3 sigma

    fClusteringFlag = 0;    //"standard" clustering based on sum of six time samples on a strip
    fDeconvolutionFlag = 1; // Set "keep strip" flag based on deconvoluted ADC samples
}

GEMModule::~GEMModule()
{
    delete fStripTimeFunc;
}

void GEMModule::Clear()
{ // we will want to clear out many more things too
    // Modify this a little bit so we only clear out the "hit counters", not necessarily the
    // arrays themselves, to make the decoding more efficient:

    fNstrips_hit = 0;
    fNstrips_hitU = 0;
    fNstrips_hitV = 0;
    fNstrips_hitU_neg = 0;
    fNstrips_hitV_neg = 0;
    fNdecoded_ADCsamples = 0;
    fIsDecoded = false;

    fTrackPassedThrough = 0;

    // numbers of strips passing basic zero suppression thresholds and timing cuts:
    fNstrips_keep = 0;
    fNstrips_keepU = 0;
    fNstrips_keepV = 0;
    // numbers of strips passing basic zero suppression thresholds, timing cuts, and higher max. sample and strip sum thresholds for
    //  local max:
    fNstrips_keep_lmax = 0;
    fNstrips_keep_lmaxU = 0;
    fNstrips_keep_lmaxV = 0;

    fNclustU = 0;
    fNclustV = 0;
    fNclustU_pos = 0;
    fNclustV_pos = 0;
    fNclustU_neg = 0;
    fNclustV_neg = 0;
    fNclustU_total = 0;
    fNclustV_total = 0;
    // later we may need to check whether this is a performance bottleneck:
    fUclusters.clear();
    fVclusters.clear();
    fN2Dhits = 0;
    // similar here:
    fHits.clear();

    fCM_online.assign(fN_MPD_TIME_SAMP, 0.0);

    // fStripAxis.clear();
    //  fADCsamples1D.clear();
    //  fStripTrackIndex.clear();
    //  fRawADCsamples1D.clear();

    // fStripIsU.clear();
    // fStripIsV.clear();
    // fStripADCavg.clear();

    // fUstripIndex.clear();
    // fVstripIndex.clear();
    // fStrip.clear();
    // fAxis.clear();
    // fADCsamples.clear();
    // fRawADCsamples.clear();
    // fADCsums.clear();
    // fKeepStrip.clear();
    // fMaxSamp.clear();
    // fADCmax.clear();
    // fTmean.clear();
    // fTsigma.clear();
    // fTcorr.clear();
}

void GEMModule::LoadConfig(const TDatime &date)
{
    std::cout << "[SBSGEMModule::LoadConfig]" << std::endl;

    Int_t status;

    //FILE *file = Podd::OpenDBFile("../tracking_config/db_bb.gem.dat", date, "GEMModule::OpenDBFile()", "r", 0);
    FILE *file = Podd::OpenDBFile("./tracking/tracking_config/db_bb.gem.dat", date, "GEMModule::OpenDBFile()", "r", 0);

    if (!file)
    {
        std::cout << "Cannot open file: ./tracking/tracking_config/db_bb.gem.dat" << std::endl;
    }
    std::vector<Double_t> rawpedu, rawpedv;
    std::vector<Double_t> rawrmsu, rawrmsv;

    // UShort_t layer;

    // I think we can set up the entire database parsing in one shot here (AJRP). Together with the call to ReadGeometry, this should define basically everything we need.
    // After loading, we will run some checks on the loaded information:
    // copying commented structure of DBRequest here for reference (see VarDef.h):
    //  struct DBRequest {
    //    const char*      name;     // Key name
    //    void*            var;      // Pointer to data (default to Double*)
    //    VarType          type;     // (opt) data type (see VarType.h, default Double_t)
    //    UInt_t           nelem;    // (opt) number of array elements (0/1 = 1 or auto)
    //    Bool_t           optional; // (opt) If true, missing key is ok
    //    Int_t            search;   // (opt) Search for key along name tree
    //    const char*      descript; // (opt) Key description (if 0, same as name)
    //  };

    // default these offsets to zero:
    fUStripOffset = 0.0;
    fVStripOffset = 0.0;

    // std::cout << "Before loadDB, fCommonModePlotsInitialized = " << fCommonModePlotsInitialized << std::endl;

    int cmplots_flag = fMakeCommonModePlots ? 1 : 0;
    int zerosuppress_flag = fZeroSuppress ? 1 : 0;
    int negsignalstudy_flag = fNegSignalStudy ? 1 : 0;
    int onlinezerosuppress_flag = fOnlineZeroSuppression ? 1 : 0;

    int eventinfoplots_flag = fMakeEventInfoPlots ? 1 : 0;

    int usestriptimingcuts = fUseStripTimingCuts ? 1 : 0;
    int useTSchi2cut = fUseTSchi2cut ? 1 : 0;
    int suppressfirstlast = fSuppressFirstLast;
    int usecommonmoderollingaverage = fMeasureCommonMode ? 1 : 0;

    int correctcommonmode = fCorrectCommonMode ? 1 : 0;

    std::vector<double> TSfrac_mean_temp;
    std::vector<double> TSfrac_sigma_temp;

    const DBRequest request[] = {
        {"chanmap", &fChanMapData, kIntV, 0, 0, 0},    // mandatory: decode map info
        {"apvmap", &fAPVmapping, kUInt, 0, 1, 1},      // optional, allow search up the tree if all modules in a setup have the same APV mapping
        {"pedu", &rawpedu, kDoubleV, 0, 1, 0},         // optional raw pedestal info (u strips)
        {"pedv", &rawpedv, kDoubleV, 0, 1, 0},         // optional raw pedestal info (v strips)
        {"rmsu", &rawrmsu, kDoubleV, 0, 1, 0},         // optional pedestal rms info (u strips)
        {"rmsv", &rawrmsv, kDoubleV, 0, 1, 0},         // optional pedestal rms info (v strips)
        {"layer", &fLayer, kUShort, 0, 0, 0},          // mandatory: logical tracking layer must be specified for every module:
        {"nstripsu", &fNstripsU, kUInt, 0, 0, 1},      // mandatory: number of strips in module along U axis
        {"nstripsv", &fNstripsV, kUInt, 0, 0, 1},      // mandatory: number of strips in module along V axis
        {"uangle", &fUAngle, kDouble, 0, 0, 1},        // mandatory: Angle of "U" strips wrt X axis
        {"vangle", &fVAngle, kDouble, 0, 0, 1},        // mandatory: Angle of "V" strips wrt X axis
        {"uoffset", &fUStripOffset, kDouble, 0, 1, 1}, // optional: position of first U strip
        {"voffset", &fVStripOffset, kDouble, 0, 1, 1}, // optional: position of first V strip
        {"upitch", &fUStripPitch, kDouble, 0, 0, 1},   // mandatory: Pitch of U strips
        {"vpitch", &fVStripPitch, kDouble, 0, 0, 1},   // mandatory: Pitch of V strips
        {"ugain", &fUgain, kDoubleV, 0, 1, 0},         //(optional): Gain of U strips by APV card (ordered by strip position, NOT by order of appearance in decode map)
        {"vgain", &fVgain, kDoubleV, 0, 1, 0},         //(optional): Gain of V strips by APV card (ordered by strip position, NOT by order of appearance in decode map)
        {"modulegain", &fModuleGain, kDouble, 0, 1, 1},
        {"threshold_sample", &fThresholdSample, kDouble, 0, 1, 1},         //(optional): threshold on max. ADC sample to keep strip (baseline-subtracted)
        {"threshold_stripsum", &fThresholdStripSum, kDouble, 0, 1, 1},     //(optional): threshold on sum of ADC samples on a strip (baseline-subtracted)
        {"threshold_clustersum", &fThresholdClusterSum, kDouble, 0, 1, 1}, //(optional): threshold on sum of all ADCs over all strips in a cluster (baseline-subtracted)
        {"ADCasym_cut", &fADCasymCut, kDouble, 0, 1, 1},                   //(optional): filter 2D hits by ADC asymmetry, |Asym| < cut
        {"deltat_cut", &fTimeCutUVdiff, kDouble, 0, 1, 1},                 //(optional): filter 2D hits by U/V time difference
        {"corrcoeff_cut", &fCorrCoeffCut, kDouble, 0, 1, 1},
        {"filterflag1D", &fFiltering_flag1D, kInt, 0, 1, 1},
        {"filterflag2D", &fFiltering_flag2D, kInt, 0, 1, 1},
        {"peakprominence_minsigma", &fThresh_2ndMax_nsigma, kDouble, 0, 1, 1},      //(optional): reject overlapping clusters with peak prominence less than this number of sigmas
        {"peakprominence_minfraction", &fThresh_2ndMax_fraction, kDouble, 0, 1, 1}, //(optional): reject overlapping clusters with peak prominence less than this fraction of height of nearby higher peak
        {"maxnu_charge", &fMaxNeighborsU_totalcharge, kUShort, 0, 1, 1},            //(optional): cluster size restriction along U for total charge calculation
        {"maxnv_charge", &fMaxNeighborsV_totalcharge, kUShort, 0, 1, 1},            //(optional): cluster size restriction along V for total charge calculation
        {"maxnu_pos", &fMaxNeighborsU_hitpos, kUShort, 0, 1, 1},                    //(optional): cluster size restriction for position reconstruction
        {"maxnv_pos", &fMaxNeighborsV_hitpos, kUShort, 0, 1, 1},                    //(optional): cluster size restriction for position reconstruction
        {"sigmahitshape", &fSigma_hitshape, kDouble, 0, 1, 1},                      //(optional): width parameter for cluster-splitting algorithm
        {"zerosuppress", &zerosuppress_flag, kUInt, 0, 1, 1},                       //(optional, search): toggle offline zero suppression (default = true).
        {"zerosuppress_nsigma", &fZeroSuppressRMS, kDouble, 0, 1, 1},               //(optional, search):
        {"do_neg_signal_study", &negsignalstudy_flag, kUInt, 0, 1, 1},              //(optional, search): toggle doing negative signal analysis
        {"onlinezerosuppress", &onlinezerosuppress_flag, kUInt, 0, 1, 1},           //(optional, search)
        {"commonmode_meanU", &fCommonModeMeanU, kDoubleV, 0, 1, 0},                 //(optional, don't search)
        {"commonmode_meanV", &fCommonModeMeanV, kDoubleV, 0, 1, 0},                 //(optional, don't search)
        {"commonmode_rmsU", &fCommonModeRMSU, kDoubleV, 0, 1, 0},                   //(optional, don't search)
        {"commonmode_rmsV", &fCommonModeRMSV, kDoubleV, 0, 1, 0},                   //(optional, don't search)
        {"commonmode_flag", &fCommonModeFlag, kInt, 0, 1, 1},                       // optional, search up the tree
        {"commonmode_online_flag", &fCommonModeOnlFlag, kInt, 0, 1, 1},             // optional, search up the tree
        {"commonmode_nstriplo", &fCommonModeNstripRejectLow, kInt, 0, 1, 1},        // optional, search up the tree:
        {"commonmode_nstriphi", &fCommonModeNstripRejectHigh, kInt, 0, 1, 1},       // optional, search:
        {"commonmode_niter", &fCommonModeNumIterations, kInt, 0, 1, 1},
        {"commonmode_minstrips", &fCommonModeMinStripsInRange, kInt, 0, 1, 1},
        {"commonmode_range_nsigma", &fCommonModeRange_nsigma, kDouble, 0, 1, 1},
        {"commonmode_danning_nsigma_cut", &fCommonModeDanningMethod_NsigmaCut, kDouble, 0, 1, 1},
        {"plot_common_mode", &cmplots_flag, kInt, 0, 1, 1},
        {"plot_event_info", &eventinfoplots_flag, kInt, 0, 1, 1},
        {"chan_cm_flags", &fChan_CM_flags, kUInt, 0, 1, 1}, // optional, search up the tree: must match the value in crate map!
        {"chan_timestamp_low", &fChan_TimeStamp_low, kUInt, 0, 1, 1},
        {"chan_timestamp_high", &fChan_TimeStamp_high, kUInt, 0, 1, 1},
        {"chan_event_count", &fChan_MPD_EventCount, kUInt, 0, 1, 1},
        {"pedsub_online", &fPedSubFlag, kInt, 0, 1, 1},
        {"max2Dhits", &fMAX2DHITS, kUInt, 0, 1, 1}, // optional, search up tree
        {"usestriptimingcut", &usestriptimingcuts, kInt, 0, 1, 1},
        {"useTSchi2cut", &useTSchi2cut, kInt, 0, 1, 1},
        {"maxstrip_t0", &fStripMaxTcut_central, kDouble, 0, 1, 1},
        {"maxstrip_tcut", &fStripMaxTcut_width, kDouble, 0, 1, 1},
        {"addstrip_tcut", &fStripAddTcut_width, kDouble, 0, 1, 1},
        {"addstrip_ccor_cut", &fStripAddCorrCoeffCut, kDouble, 0, 1, 1},
        {"goodstrip_TSfrac_mean", &TSfrac_mean_temp, kDoubleV, 0, 1, 1},
        {"goodstrip_TSfrac_sigma", &TSfrac_sigma_temp, kDoubleV, 0, 1, 1},
        {"suppressfirstlast", &suppressfirstlast, kInt, 0, 1, 1},
        {"use_commonmode_rolling_average", &usecommonmoderollingaverage, kInt, 0, 1, 1},
        {"commonmode_nevents_lookback", &fNeventsCommonModeLookBack, kUInt, 0, 1, 1},
        {"correct_common_mode", &correctcommonmode, kInt, 0, 1, 1},
        {"correct_common_mode_minstrips", &fCorrectCommonModeMinStrips, kUInt, 0, 1, 1},
        {"correct_common_mode_nsigma", &fCorrectCommonMode_Nsigma, kDouble, 0, 1, 1},
        {"commonmode_binwidth_nsigma", &fCommonModeBinWidth_Nsigma, kDouble, 0, 1, 1},
        {"commonmode_scanrange_nsigma", &fCommonModeScanRange_Nsigma, kDouble, 0, 1, 1},
        {"commonmode_stepsize_nsigma", &fCommonModeStepSize_Nsigma, kDouble, 0, 1, 1},
        {"deconvolution_tau", &fStripTau, kDouble, 0, 1, 1},
        {"CMbiasU", &fCMbiasU, kDoubleV, 0, 1, 1},
        {"CMbiasV", &fCMbiasV, kDoubleV, 0, 1, 1},
        {"clustering_flag", &fClusteringFlag, kInt, 0, 1, 1},
        {"deconvolution_flag", &fDeconvolutionFlag, kInt, 0, 1, 1},
        {0}};
    //std::cout << "XB DEBUG: module name: " << GetName() << std::endl;
    std::string _temp_prefix = std::string("bb.gem.") + std::string(GetName()) + std::string(".");
    const char *fPrefix = _temp_prefix.c_str();
    //std::cout << "XB DEBUG: module name: " << fPrefix << std::endl;
    // status = LoadDB(file, date, request, fPrefix, 1); // The "1" after fPrefix means search up the tree
    status = Podd::LoadDatabase(file, date, request, fPrefix, 1, "GEMModule::LoadDB");

    if (status != 0)
    {
        fclose(file);
    }

    if (!fCommonModePlots_DBoverride)
        fMakeCommonModePlots = cmplots_flag != 0;
    fZeroSuppress = zerosuppress_flag != 0;
    fOnlineZeroSuppression = onlinezerosuppress_flag != 0;

    fNegSignalStudy = negsignalstudy_flag != 0;

    fMakeEventInfoPlots = eventinfoplots_flag != 0;

    fUseStripTimingCuts = usestriptimingcuts != 0;
    fUseTSchi2cut = useTSchi2cut != 0;

    fSuppressFirstLast = suppressfirstlast;

    // fMeasureCommonMode = usecommonmoderollingaverage != 0;
    fMeasureCommonMode = true; // we're not turning this off
    fCorrectCommonMode = correctcommonmode != 0;

    if (fUseTSchi2cut && TSfrac_mean_temp.size() == fN_MPD_TIME_SAMP && TSfrac_sigma_temp.size() == fN_MPD_TIME_SAMP)
    {
        fGoodStrip_TSfrac_mean = TSfrac_mean_temp;
        fGoodStrip_TSfrac_sigma = TSfrac_sigma_temp;
    }

    //  std::cout << "After loadDB, fCommonModePlotsInitialized = " << fCommonModePlotsInitialized << std::endl;

    if (/*fAPVmapping < SBSGEM::kINFN ||*/ fAPVmapping > SBSGEM::kMC)
    {
        std::cout << "Warning in SBSGEMModule::Decode for module " << GetName() << ": invalid APV mapping choice, defaulting to UVA X/Y." << std::endl
                  << " Analysis results may be incorrect" << std::endl;
        fAPVmapping = SBSGEM::kUVA_XY;
    }

    // prevent the user from defining something silly for the common-mode stuff:
    fCommonModeNstripRejectLow = std::min(50, std::max(0, fCommonModeNstripRejectLow));
    fCommonModeNstripRejectHigh = std::min(50, std::max(0, fCommonModeNstripRejectHigh));
    fCommonModeNumIterations = std::min(10, std::max(2, fCommonModeNumIterations));
    fCommonModeMinStripsInRange = std::min(fN_APV25_CHAN - 25, std::max(1, fCommonModeMinStripsInRange));

    double x = fSamplePeriod / fStripTau;

    fDeconv_weights[0] = exp(x - 1.0) / x;
    fDeconv_weights[1] = -2.0 * exp(-1.0) / x;
    fDeconv_weights[2] = exp(-1.0 - x) / x;

    std::cout << GetName() << " fThresholdStripSum " << fThresholdStripSum << " fThresholdSample " << fThresholdSample << std::endl;

    if (fIsMC)
    {
        fCommonModeFlag = -1;
        fPedestalMode = false;
        fOnlineZeroSuppression = true;
        fAPVmapping = SBSGEM::kMC;
    }

    fPxU = cos(fUAngle * TMath::DegToRad());
    fPyU = sin(fUAngle * TMath::DegToRad());
    fPxV = cos(fVAngle * TMath::DegToRad());
    fPyV = sin(fVAngle * TMath::DegToRad());

    fAPVch_by_Ustrip.clear();
    fAPVch_by_Vstrip.clear();
    fMPDID_by_Ustrip.clear();
    fMPDID_by_Vstrip.clear();
    fADCch_by_Ustrip.clear();
    fADCch_by_Vstrip.clear();

    // Count APV cards by axis. Each APV card must have one decode map entry:
    fNAPVs_U = 0;
    fNAPVs_V = 0;

    fMPDmap.clear();

    Int_t nentry = fChanMapData.size() / fMPDMAP_ROW_SIZE;

    fCommonModeResultContainer_by_APV.resize(nentry);
    fCommonModeRollingAverage_by_APV.resize(nentry);
    fCommonModeRollingRMS_by_APV.resize(nentry);
    fNeventsRollingAverage_by_APV.resize(nentry);

    fCMbiasResultContainer_by_APV.resize(nentry);
    fCommonModeOnlineBiasRollingAverage_by_APV.resize(nentry);
    fCommonModeOnlineBiasRollingRMS_by_APV.resize(nentry);
    fNeventsOnlineBias_by_APV.resize(nentry);

    for (Int_t mapline = 0; mapline < nentry; mapline++)
    {
        mpdmap_t thisdata;
        thisdata.crate = fChanMapData[0 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.slot = fChanMapData[1 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.mpd_id = fChanMapData[2 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.gem_id = fChanMapData[3 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.adc_id = fChanMapData[4 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.i2c = fChanMapData[5 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.pos = fChanMapData[6 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.invert = fChanMapData[7 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.axis = fChanMapData[8 + mapline * fMPDMAP_ROW_SIZE];
        thisdata.index = mapline;

        // Populate relevant quantities mapped by strip index:
        for (int ich = 0; ich < fN_APV25_CHAN; ich++)
        {
            int strip = GetStripNumber(ich, thisdata.pos, thisdata.invert);
            if (thisdata.axis == SBSGEM::kUaxis)
            {
                fAPVch_by_Ustrip[strip] = ich;
                fMPDID_by_Ustrip[strip] = thisdata.mpd_id;
                fADCch_by_Ustrip[strip] = thisdata.adc_id;
            }
            else
            {
                fAPVch_by_Vstrip[strip] = ich;
                fMPDID_by_Vstrip[strip] = thisdata.mpd_id;
                fADCch_by_Vstrip[strip] = thisdata.adc_id;
            }
        }

        if (thisdata.axis == SBSGEM::kUaxis)
        {
            fNAPVs_U++;
        }
        else
        {
            fNAPVs_V++;
        }

        fMPDmap.push_back(thisdata);

        fEventCount_by_APV.push_back(0);

        fT0_by_APV.push_back(0);
        fTcoarse_by_APV.push_back(0);
        fTfine_by_APV.push_back(0);
        fTimeStamp_ns_by_APV.push_back(0);

        // fCommonModeRollingFirstEvent_by_APV[mapline] = 0.0;
        fCommonModeResultContainer_by_APV[mapline].resize(fNeventsCommonModeLookBack * fN_MPD_TIME_SAMP);
        fCommonModeRollingAverage_by_APV[mapline] = 0.0;
        fCommonModeRollingRMS_by_APV[mapline] = 10.0;
        fNeventsRollingAverage_by_APV[mapline] = 0; // Really will be the number of time samples = 6 * number of events

        fCMbiasResultContainer_by_APV[mapline].resize(fNeventsCommonModeLookBack * fN_MPD_TIME_SAMP);
        fCommonModeOnlineBiasRollingAverage_by_APV[mapline] = 0.0;
        fCommonModeOnlineBiasRollingRMS_by_APV[mapline] = 10.0;
        fNeventsOnlineBias_by_APV[mapline] = 0;
    }

    // if a different number of decode map entries is counted than the expectation based on the number of strips,
    // e.g., because we commented out one or more decode map entries, then we go with the larger of the two numbers.
    // This isn't perfectly idiot-proof, but prevents the kind of undefined behavior we want to avoid:
    // if( fNAPVs_U != fNstripsU/fN_APV25_CHAN ){
    fNAPVs_U = std::max(fNAPVs_U, fNstripsU / fN_APV25_CHAN);
    //}
    fNAPVs_V = std::max(fNAPVs_V, fNstripsV / fN_APV25_CHAN);

    // resize vectors that hold APV-card specific parameters:
    //  fUgain.resize( fNAPVs_U );
    //  fVgain.resize( fNAPVs_V );
    //  fCommonModeMeanU.resize( fNAPVs_U );
    //  fCommonModeMeanV.resize( fNAPVs_V );
    //  fCommonModeRMSU.resize( fNAPVs_U );
    //  fCommonModeRMSV.resize( fNAPVs_V );

    std::cout << fName << " mapped to " << nentry << " APV25 chips, module gain = " << fModuleGain << std::endl;

    // Geometry info is required to be present in the database for each module:
    Int_t err = ReadGeometry(file, date, true);
    if (err)
    {
        fclose(file);
        std::cout << "XB: " << __func__ << " Error: failed to load module geometry information: " << file << std::endl;
    }

    // Initialize all pedestals to zero, RMS values to default:
    fPedestalU.clear();
    fPedestalU.resize(fNstripsU);

    fPedRMSU.clear();
    fPedRMSU.resize(fNstripsU);

    // std::cout << "got " << rawpedu.size() << " u pedestal mean values and " << rawrmsu.size() << " u pedestal rms values" << std::endl;
    // std::cout << "got " << rawpedv.size() << " v pedestal mean values and " << rawrmsv.size() << " v pedestal rms values" << std::endl;

    // for( int i=0; i<rawpedu.size(); i++ ){
    //   cout << i << ", " << rawpedu[i] << endl;
    // }

    for (UInt_t istrip = 0; istrip < fNstripsU; istrip++)
    {
        fPedestalU[istrip] = 0.0;
        fPedRMSU[istrip] = 10.0; // placeholder to be replaced by value from database

        if (rawpedu.size() == fNstripsU)
        {
            fPedestalU[istrip] = rawpedu[istrip];
        }
        else if (rawpedu.size() == fNstripsU / 128)
        {
            fPedestalU[istrip] = rawpedu[istrip / 128];
        }
        else if (!rawpedu.empty())
        {
            fPedestalU[istrip] = rawpedu[0];
        }

        if (rawrmsu.size() == fNstripsU)
        {
            fPedRMSU[istrip] = rawrmsu[istrip];
        }
        else if (rawrmsu.size() == fNstripsU / 128)
        {
            fPedRMSU[istrip] = rawrmsu[istrip / 128];
        }
        else if (!rawrmsu.empty())
        {
            fPedRMSU[istrip] = rawrmsu[0];
        }
    }

    // Initialize all pedestals to zero, RMS values to default:
    fPedestalV.clear();
    fPedestalV.resize(fNstripsV);

    fPedRMSV.clear();
    fPedRMSV.resize(fNstripsV);

    for (UInt_t istrip = 0; istrip < fNstripsV; istrip++)
    {
        fPedestalV[istrip] = 0.0;
        fPedRMSV[istrip] = 10.0;

        if (rawpedv.size() == fNstripsV)
        {
            fPedestalV[istrip] = rawpedv[istrip];
        }
        else if (rawpedv.size() == fNstripsV / 128)
        {
            fPedestalV[istrip] = rawpedv[istrip / 128];
        }
        else if (!rawpedv.empty())
        {
            fPedestalV[istrip] = rawpedv[0];
        }

        if (rawrmsv.size() == fNstripsV)
        {
            fPedRMSV[istrip] = rawrmsv[istrip];
        }
        else if (rawrmsv.size() == fNstripsV / 128)
        {
            fPedRMSV[istrip] = rawrmsv[istrip / 128];
        }
        else if (!rawrmsv.empty())
        {
            fPedRMSV[istrip] = rawrmsv[0];
        }
    }

    // //resize all the "decoded strip" arrays to their maximum possible values for this module:
    UInt_t nstripsmax = fNstripsU + fNstripsV;

    fStrip.resize(nstripsmax);
    fAxis.resize(nstripsmax);
    fADCsamples.resize(nstripsmax);
    fRawADCsamples.resize(nstripsmax);
    fADCsamples_deconv.resize(nstripsmax);
    // The lines below are problematic and unnecessary
    for (unsigned int istrip = 0; istrip < nstripsmax; istrip++)
    {
        fADCsamples[istrip].resize(fN_MPD_TIME_SAMP);
        fRawADCsamples[istrip].resize(fN_MPD_TIME_SAMP);
        fADCsamples_deconv[istrip].resize(fN_MPD_TIME_SAMP);
    }

    fADCsums.resize(nstripsmax);
    fADCsumsDeconv.resize(nstripsmax);
    fStripADCavg.resize(nstripsmax);
    fStripIsU.resize(nstripsmax);
    fStripIsV.resize(nstripsmax);
    fStripOnTrack.resize(nstripsmax);
    fStripIsNeg.resize(nstripsmax);
    fStripIsNegU.resize(nstripsmax);
    fStripIsNegV.resize(nstripsmax);
    fStripIsNegOnTrack.resize(nstripsmax);
    fStripIsNegOnTrackU.resize(nstripsmax);
    fStripIsNegOnTrackV.resize(nstripsmax);
    fStripRaw.resize(nstripsmax);
    fStripEvent.resize(nstripsmax);
    fStripMPD.resize(nstripsmax);
    fStripADC_ID.resize(nstripsmax);
    fStripTrackIndex.resize(nstripsmax);
    fKeepStrip.resize(nstripsmax);
    fMaxSamp.resize(nstripsmax);
    fMaxSampDeconv.resize(nstripsmax);
    fMaxSampDeconvCombo.resize(nstripsmax);
    fADCmax.resize(nstripsmax);
    fADCmaxDeconv.resize(nstripsmax);
    fADCmaxDeconvCombo.resize(nstripsmax);
    fTmean.resize(nstripsmax);
    fTmeanDeconv.resize(nstripsmax);
    fTsigma.resize(nstripsmax);
    fStripTdiff.resize(nstripsmax);
    fStripTSchi2.resize(nstripsmax);
    fStripCorrCoeff.resize(nstripsmax);
    fStripTfit.resize(nstripsmax);
    fTcorr.resize(nstripsmax);
    // Storing these by individual strip is redundant but convenient:
    fStrip_ENABLE_CM.resize(nstripsmax);
    fStrip_CM_GOOD.resize(nstripsmax);
    fStrip_BUILD_ALL_SAMPLES.resize(nstripsmax);

    fStripUonTrack.resize(nstripsmax);
    fStripVonTrack.resize(nstripsmax);

    fADCsamples1D.resize(nstripsmax * fN_MPD_TIME_SAMP);
    fRawADCsamples1D.resize(nstripsmax * fN_MPD_TIME_SAMP);
    fADCsamplesDeconv1D.resize(nstripsmax * fN_MPD_TIME_SAMP);

    // default all common-mode mean and RMS values to 0 and 10 respectively if they were
    //  NOT loaded from the DB and/or they are loaded with the wrong size:
    if (fCommonModeMeanU.size() != fNAPVs_U)
    {
        fCommonModeMeanU.resize(fNAPVs_U);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_U; iAPV++)
        {
            fCommonModeMeanU[iAPV] = 0.0;
        }
    }

    if (fCommonModeRMSU.size() != fNAPVs_U)
    {
        fCommonModeRMSU.resize(fNAPVs_U);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_U; iAPV++)
        {
            fCommonModeRMSU[iAPV] = 10.0;
        }
    }

    // default all common-mode mean and RMS values to 0 and 10 respectively if they were
    //  NOT loaded from the DB and/or they were loaded with the wrong size:
    if (fCommonModeMeanV.size() != fNAPVs_V)
    {
        fCommonModeMeanV.resize(fNAPVs_V);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_V; iAPV++)
        {
            fCommonModeMeanV[iAPV] = 0.0;
        }
    }

    if (fCommonModeRMSV.size() != fNAPVs_V)
    {
        fCommonModeRMSV.resize(fNAPVs_V);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_V; iAPV++)
        {
            fCommonModeRMSV[iAPV] = 10.0;
        }
    }

    // Initialize default "CM correction bias" values to zero if they were not loaded from the DB:
    if (fCMbiasU.size() != fNAPVs_U)
    {
        fCMbiasU.resize(fNAPVs_U);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_U; iAPV++)
        {
            fCMbiasU[iAPV] = 0.0;
        }
    }

    if (fCMbiasV.size() != fNAPVs_V)
    {
        fCMbiasV.resize(fNAPVs_V);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_V; iAPV++)
        {
            fCMbiasV[iAPV] = 0.0;
        }
    }

    // default all gains to 1 if they were not loaded from the DB and/or if they were loaded with the
    // wrong size:
    if (fUgain.size() != fNAPVs_U)
    {
        fUgain.resize(fNAPVs_U);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_U; iAPV++)
        {
            fUgain[iAPV] = 1.0;
        }
    }

    // Multiply in Module gain:
    for (unsigned int iAPV = 0; iAPV < fNAPVs_U; iAPV++)
    {
        fUgain[iAPV] *= fModuleGain;
    }

    if (fVgain.size() != fNAPVs_V)
    {
        fVgain.resize(fNAPVs_V);
        for (unsigned int iAPV = 0; iAPV < fNAPVs_V; iAPV++)
        {
            fVgain[iAPV] = 1.0;
        }
    }

    for (unsigned int iAPV = 0; iAPV < fNAPVs_V; iAPV++)
    {
        fVgain[iAPV] *= fModuleGain;
    }

    if (fPedestalMode)
    {
        fZeroSuppress = false;
        fOnlineZeroSuppression = false;
        // fPedSubFlag = 0;
    }

    // for( UInt_t i = 0; i < rawped.size(); i++ ){
    //   if( (i % 2) == 1 ) continue;
    //   int idx = (int) rawped[i];

    //   if( idx < N_APV25_CHAN*nentry ){
    //     fPedestal[idx] = rawped[i+1];
    //   } else {

    //     std::cout << "[SBSGEMModule::ReadDatabase]  WARNING: " << " strip " << idx  << " listed but not enough strips in cratemap" << std::endl;
    //   }
    // }

    // for( UInt_t i = 0; i < rawrms.size(); i++ ){
    //   if( (i % 2) == 1 ) continue;
    //   int idx = (int) rawrms[i];
    //   if( idx < N_APV25_CHAN*nentry ){
    //     fRMS[idx] = rawrms[i+1];
    //   } else {
    //     std::cout << "[SBSGEMModule::ReadDatabase]  WARNING: " << " strip " << idx  << " listed but not enough strips in cratemap" << std::endl;
    //   }
    // }

    fclose(file);
}

Int_t GEMModule::ReadGeometry(FILE *file, const TDatime &date, Bool_t required)
{ // We start with a copy of THaDetectorBase::ReadGeometry and modify accordingly:
    // Read this detector's basic geometry information from the database.
    // Derived classes may override to read more advanced data.

    const char *const here = "ReadGeometry";

    std::vector<double> position, size, angles;
    Bool_t optional = !required;
    DBRequest request[] = {
        {"position", &position, kDoubleV, 0, optional, 0,
         "\"position\" (detector position [m])"},
        {"size", &size, kDoubleV, 0, optional, 1,
         "\"size\" (detector size [m])"},
        {"angle", &angles, kDoubleV, 0, true, 0,
         "\"angle\" (detector angles(s) [deg]"},
        {nullptr}};

    // Int_t err = LoadDB(file, date, request);
    std::string __temp_prefix = std::string("bb.gem.") + std::string(GetName()) + std::string(".");
    const char *fPrefix = __temp_prefix.c_str();
    Int_t err = Podd::LoadDatabase(file, date, request, fPrefix, 0, "GEMModule::ReadGeometry()::LoadDB");
    if (err)
    {
        std::cout << "XB::" << __func__ << " ERROR: failed to load module geometry." << std::endl;
    }

    if (!position.empty())
    {
        if (position.size() != 3)
        {
            Error(Here(here), "Incorrect number of values = %u for "
                              "detector position. Must be exactly 3. Fix database.",
                  static_cast<unsigned int>(position.size()));
            return 1;
        }
        fOrigin.SetXYZ(position[0], position[1], position[2]);
    }
    else
        fOrigin.SetXYZ(0, 0, 0);

    if (!size.empty())
    {
        if (size.size() != 3)
        {
            Error(Here(here), "Incorrect number of values = %u for "
                              "detector size. Must be exactly 3. Fix database.",
                  static_cast<unsigned int>(size.size()));
            return 2;
        }
        if (size[0] == 0 || size[1] == 0 || size[2] == 0)
        {
            Error(Here(here), "Illegal zero detector dimension. Fix database.");
            return 3;
        }
        if (size[0] < 0 || size[1] < 0 || size[2] < 0)
        {
            Warning(Here(here), "Illegal negative value for detector dimension. "
                                "Taking absolute. Check database.");
        }
        fSize[0] = 0.5 * TMath::Abs(size[0]);
        fSize[1] = 0.5 * TMath::Abs(size[1]);
        fSize[2] = TMath::Abs(size[2]);
    }
    else
        fSize[0] = fSize[1] = fSize[2] = 1.e38; // default large junk number

    if (!angles.empty())
    {
        if (angles.size() != 1 && angles.size() != 3)
        {
            Error(Here(here), "Incorrect number of values = %u for "
                              "detector angle(s). Must be either 1 or 3. Fix database.",
                  static_cast<unsigned int>(angles.size()));
            return 4;
        }
        // If one angle is given, it indicates a rotation about y, as before.
        // If three angles are given, they are interpreted as rotations about the X, Y, and Z axes, respectively:
        //
        if (angles.size() == 1)
        {
            DefineAxes(angles[0] * TMath::DegToRad());
        }
        else
        {
            TRotation RotTemp;

            // So let's review how to define the detector axes correctly.

            // THaDetectorBase::DetToTrackCoord(TVector3 p) returns returns p.X * fXax() + p.Y * fYax() + p.Z * fZax() + fOrigin
            // In the standalone code, we do Rot * (p) + fOrigin (essentially):
            // So in matrix form, when we do TRotation::RotateX(alpha), we get:

            // RotTemp * Point =  |  1    0            0         |    |  p.X()  |
            //                    |  0   cos(alpha) -sin(alpha)  | *  |  p.Y()  |
            //                    |  0   sin(alpha)  cos(alpha)  |    |  p.Z()  |
            //
            // This definition ***appears**** to be consistent with the "sense" of the rotation as applied by the standalone code.
            // The detector axes are defined as the columns of the rotation matrix. We will have to test that it is working correctly, however:

            RotTemp.RotateX(angles[0] * TMath::DegToRad());
            RotTemp.RotateY(angles[1] * TMath::DegToRad());
            RotTemp.RotateZ(angles[2] * TMath::DegToRad());

            fXax.SetXYZ(RotTemp.XX(), RotTemp.YX(), RotTemp.ZX());
            fYax.SetXYZ(RotTemp.XY(), RotTemp.YY(), RotTemp.ZY());
            fZax.SetXYZ(RotTemp.XZ(), RotTemp.YZ(), RotTemp.ZZ());
        }
    }
    else
        DefineAxes(0);

    return 0;
}

TVector3 GEMModule::TrackToDetCoord(const TVector3 &point) const
{
    // Convert/project coordinates of the given 'point' to coordinates in this
    // detector's reference frame (defined by fOrigin, fXax, and fYax).

    TVector3 v = point - fOrigin;
    // This works out to exactly the same as a multiplication with TRotation
    return TVector3(v.Dot(fXax), v.Dot(fYax), v.Dot(fZax));
}

TVector3 GEMModule::DetToTrackCoord(Double_t x, Double_t y) const
{
    // Convert x/y coordinates in detector plane to the track reference frame.

    return TVector3(x * fXax + y * fYax + fOrigin);
}

void GEMModule::fill_ADCfrac_vs_time_sample_goodstrip(Int_t hitindex, bool ismax)
{
    if (hitindex < 0 || hitindex >= fNstrips_hit)
        return;
    if (hADCfrac_vs_timesample_goodstrips == NULL)
        return;
    if (hADCfrac_vs_timesample_maxstrip == NULL)
        return;

    for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
    {
        hADCfrac_vs_timesample_goodstrips->Fill(isamp, fADCsamples[hitindex][isamp] / fADCsums[hitindex]);
        if (ismax)
        {
            hADCfrac_vs_timesample_maxstrip->Fill(isamp, fADCsamples[hitindex][isamp] / fADCsums[hitindex]);
        }
    }
}

// utility method to calculate correlation coefficient of U and V samples:
Double_t GEMModule::CorrCoeff(int nsamples, const std::vector<double> &Usamples, const std::vector<double> &Vsamples, int firstsample)
{
    Double_t sumu = 0.0, sumv = 0.0, sumu2 = 0.0, sumv2 = 0.0, sumuv = 0.0;

    if ((int)Usamples.size() < firstsample + nsamples || (int)Vsamples.size() < firstsample + nsamples)
    {
        return -10.0; // nonsense value, correlation coefficient by definition is -1 < c < 1
    }

    for (int isamp = firstsample; isamp < firstsample + nsamples; isamp++)
    {
        sumu += Usamples[isamp];
        sumv += Vsamples[isamp];
        sumu2 += pow(Usamples[isamp], 2);
        sumv2 += pow(Vsamples[isamp], 2);
        sumuv += Usamples[isamp] * Vsamples[isamp];
    }

    double nSAMP = double(nsamples);
    double mu = sumu / nSAMP;
    double mv = sumv / nSAMP;
    double varu = sumu2 / nSAMP - pow(mu, 2);
    double varv = sumv2 / nSAMP - pow(mv, 2);
    double sigu = sqrt(varu);
    double sigv = sqrt(varv);

    return (sumuv - nSAMP * mu * mv) / (nSAMP * sigu * sigv);
}

// This function calculates the chi2 of a vector of time samples with respect to the "Good Strip" averages:
double GEMModule::StripTSchi2(int hitindex)
{
    if (hitindex < 0 || hitindex > fNstrips_hit)
        return -1.;
    double chi2 = 0.0;
    for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
    {
        chi2 += pow((fADCsamples[hitindex][isamp] / fADCsums[hitindex] - fGoodStrip_TSfrac_mean[isamp]) / fGoodStrip_TSfrac_sigma[isamp], 2);
    }
    return chi2;
}

TVector2 GEMModule::UVtoXY(TVector2 UV)
{
    double det = fPxU * fPyV - fPyU * fPxV;

    double Utemp = UV.X();
    double Vtemp = UV.Y();

    double Xtemp = (fPyV * Utemp - fPyU * Vtemp) / det;
    double Ytemp = (fPxU * Vtemp - fPxV * Utemp) / det;

    return TVector2(Xtemp, Ytemp);
}

TVector2 GEMModule::XYtoUV(TVector2 XY)
{
    double Xtemp = XY.X();
    double Ytemp = XY.Y();

    double Utemp = Xtemp * fPxU + Ytemp * fPyU;
    double Vtemp = Xtemp * fPxV + Ytemp * fPyV;

    return TVector2(Utemp, Vtemp);
}

void GEMModule::find_2Dhits()
{ // version with no arguments calls 1D cluster finding with default (wide-open) track search constraints
    // these functions will fill the 1D cluster arrays:

    find_clusters_1D(SBSGEM::kUaxis); // u strips
    find_clusters_1D(SBSGEM::kVaxis); // v strips

    // Now make 2D clusters:

    //std::cout<<"GEMModuel: u clusters found: "<<fNclustU<<", v clusters found: "<<fNclustV<<std::endl;
    if (fNclustU > 0 && fNclustV > 0)
    {

        fxcmin = -1.e12;
        fxcmax = 1.e12;
        fycmin = -1.e12;
        fycmax = 1.e12;

        fill_2D_hit_arrays();
    }
}

void GEMModule::find_2Dhits(TVector2 constraint_center, TVector2 constraint_width)
{
    // constraint center and constraint width are assumed given in meters in "local" detector x,y coordinates.

    double ucenter = constraint_center.X() * fPxU + constraint_center.Y() * fPyU;
    double vcenter = constraint_center.X() * fPxV + constraint_center.Y() * fPyV;

    // To determine the constraint width along u/v, we need to transform the X/Y constraint widths, which define a rectangular region,
    // into U and V limits for 1D clustering:

    double umin, umax, vmin, vmax;

    double xmin = constraint_center.X() - constraint_width.X();
    double xmax = constraint_center.X() + constraint_width.X();
    double ymin = constraint_center.Y() - constraint_width.Y();
    double ymax = constraint_center.Y() + constraint_width.Y();

    // store these for later use:
    fxcmin = xmin;
    fxcmax = xmax;
    fycmin = ymin;
    fycmax = ymax;

    // check the four corners of the rectangle and compute the maximum values of u and v occuring at the four corners of the rectangular region:
    //  NOTE: we will ALSO enforce the 2D search region in X and Y when we combine 1D U/V clusters into 2D X/Y hits, which, depending on the U/V strip orientation
    //  can exclude some 2D hits that would have passed the U/V constraints defined by the corners of the X/Y rectangle, but been outside the X/Y constraint rectangle

    double u00 = xmin * fPxU + ymin * fPyU;
    double u01 = xmin * fPxU + ymax * fPyU;
    double u10 = xmax * fPxU + ymin * fPyU;
    double u11 = xmax * fPxU + ymax * fPyU;

    // this is some elegant-looking (compact) code, but perhaps algorithmically clunky:
    umin = std::min(u00, std::min(u01, std::min(u10, u11)));
    umax = std::max(u00, std::max(u01, std::max(u10, u11)));

    double v00 = xmin * fPxV + ymin * fPyV;
    double v01 = xmin * fPxV + ymax * fPyV;
    double v10 = xmax * fPxV + ymin * fPyV;
    double v11 = xmax * fPxV + ymax * fPyV;

    vmin = std::min(v00, std::min(v01, std::min(v10, v11)));
    vmax = std::max(v00, std::max(v01, std::max(v10, v11)));

    // The following routines will loop on all the strips and populate the 1D "cluster list" (vector<sbsgemcluster_t> )
    // Taking half the difference between max and min as the "width" is consistent with how it is used in
    //  find_clusters_1D; i.e., the peak is required to be within |peak position - constraint center| <= constraint width
    find_clusters_1D(SBSGEM::kUaxis, ucenter, (umax - umin) / 2.0); // U clusters
    find_clusters_1D(SBSGEM::kVaxis, vcenter, (vmax - vmin) / 2.0); // V clusters

    // Now make 2D clusters

    if (fNclustU > 0 && fNclustV > 0)
    {
        fill_2D_hit_arrays();
    }
}

void GEMModule::find_clusters_1D(SBSGEM::GEMaxis_t axis, Double_t constraint_center, Double_t constraint_width)
{
    //std::cout << "xb:: debug::" << __func__ << " called..." << std::endl;
    // Constraint center and constraint width are assumed to be given in "standard" Hall A units (meters) in module-local coordinates
    //  (SPECIFICALLY: constraint center and constraint width are assumed to refer to the direction measured by the strips being considered here)
    // This method will generally only be called by the reconstruction methods of SBSGEMTrackerBase

    // cout << "constraint center, constraint width = " << constraint_center << ", " << constraint_width << endl;

    if (!fIsDecoded)
    {
        std::cout << "find_clusters invoked before decoding for GEM Module " << GetName() << ", doing nothing" << std::endl;
        return;
    }

    UShort_t maxsep;
    UShort_t maxsepcoord;
    UInt_t Nstrips;
    Double_t pitch;
    Double_t offset = (axis == SBSGEM::kUaxis) ? fUStripOffset : fVStripOffset;

    // hopefully this compiles and works correctly:
    std::vector<sbsgemcluster_t> &clusters = (axis == SBSGEM::kUaxis) ? fUclusters : fVclusters;

    UInt_t &nclust = (axis == SBSGEM::kUaxis) ? fNclustU : fNclustV;
    UInt_t &nclust_pos = (axis == SBSGEM::kUaxis) ? fNclustU_pos : fNclustV_pos;
    UInt_t &nclust_neg = (axis == SBSGEM::kUaxis) ? fNclustU_neg : fNclustV_neg;
    UInt_t &nclust_tot = (axis == SBSGEM::kUaxis) ? fNclustU_total : fNclustV_total;

    nclust = 0;
    nclust_pos = 0;
    nclust_neg = 0;
    nclust_tot = 0;

    clusters.clear();

    if (axis == SBSGEM::kUaxis)
    {
        maxsep = fMaxNeighborsU_totalcharge;
        maxsepcoord = fMaxNeighborsU_hitpos;
        Nstrips = fNstripsU;
        pitch = fUStripPitch;
        // fNclustU = 0;
        // fUclusters.clear();
    }
    else
    { // V axis, no need to compare axis to kVaxis:
        maxsep = fMaxNeighborsV_totalcharge;
        maxsepcoord = fMaxNeighborsV_hitpos;
        Nstrips = fNstripsV;
        pitch = fVStripPitch;
        // fNclustV = 0;
        // fVclusters.clear();
    }

    std::set<UShort_t> striplist;        // sorted list of strips for 1D clustering
    std::map<UShort_t, UInt_t> hitindex; // key = strip ID, mapped value = index in decoded hit array, needed to access the other information efficiently:
    std::map<UShort_t, Double_t> pedrms_strip;
    std::map<UShort_t, Double_t> ADC_strip;    // These are the (configuration-dependent) quantities we use for clustering. They depend on the values of fClusteringFlag and fSuppressFirstLast and fDeconvolution_flag
    std::map<UShort_t, Double_t> ADC_maxsamp;  //
    std::map<UShort_t, Double_t> Tmean_strip;  // strip mean time with first and/or last samples removed (if applicable)
    std::map<UShort_t, Double_t> Tsigma_strip; // strip rms time with first and/or last samples removed (if applicable)

    std::set<UShort_t> striplist_neg; // same as above but for negative strips
    std::map<UShort_t, UInt_t> hitindex_neg;
    std::map<UShort_t, Double_t> pedrms_strip_neg;

    UShort_t FirstSampleCorrCoeff = 0;
    UShort_t NsampCorrCoeff = fN_MPD_TIME_SAMP;

    // if( fSuppressFirstLast != 0 ){
    //   if( fSuppressFirstLast > 0 ){ //suppress first and last time samples
    //     FirstSampleCorrCoeff = 1;
    //     NsampCorrCoeff = fN_MPD_TIME_SAMP-2;
    //   } else if( fSuppressFirstLast == -2 ){ //exclude last time sample only:
    //     FirstSampleCorrCoeff = 0;
    //     NsampCorrCoeff = fN_MPD_TIME_SAMP-1;
    //   } else { //negative value other than -2: exclude first time sample only:
    //     FirstSampleCorrCoeff = 1;
    //     NsampCorrCoeff = fN_MPD_TIME_SAMP-1;
    //   }
    // }

    //std::cout<<" total strips hit: "<<fNstrips_hit<<std::endl;
    for (int ihit = 0; ihit < fNstrips_hit; ihit++)
    {
        //std::cout<<"keep this strip: "<<ihit<<" : axis: "<<fAxis[ihit]<<", "<<fKeepStrip[ihit]<<std::endl;
        if (fAxis[ihit] == axis && fKeepStrip[ihit])
        {
            bool newstrip = (striplist.insert(fStrip[ihit])).second;

            if (newstrip)
            { // should always be true:
                hitindex[fStrip[ihit]] = ihit;
                if (axis == SBSGEM::kUaxis)
                {
                    pedrms_strip[fStrip[ihit]] = fPedRMSU[fStrip[ihit]];
                }
                else
                {
                    pedrms_strip[fStrip[ihit]] = fPedRMSV[fStrip[ihit]];
                }

                ADC_strip[fStrip[ihit]] = fADCsums[ihit];
                ADC_maxsamp[fStrip[ihit]] = fADCmax[ihit];
                Tmean_strip[fStrip[ihit]] = fTmean[ihit];
                Tsigma_strip[fStrip[ihit]] = fTsigma[ihit];

                // the commented lines below are experimental. Don't use for now:
                //  if( fClusteringFlag == 1 ){ //Use deconvoluted ADCs instead of shaped samples for the clustering
                //    ADC_strip[fStrip[ihit]] = fADCmaxDeconvCombo[ihit]; //max. of two adjacent deconvoluted time samples
                //    ADC_maxsamp[fStrip[ihit]] = fADCmaxDeconv[ihit]; //max. of any single deconvoluted time sample
                //    Tmean_strip[fStrip[ihit]] = fTmeanDeconv[ihit];
                //    //Tsigma_strip[fStrip[ihit]] = fTsigma[ihit];
                //  } else { //check first and last time sample suppression flags:
                //    if( fSuppressFirstLast != 0 ){ //exclude first and/or last time sample
                //      double Tmean_all = fTmean[ihit];
                //      double Tsigma_all = fTsigma[ihit];
                //      double oldTsum = fADCsums[ihit]*Tmean_all;
                //      double oldT2sum = fADCsums[ihit]*(pow(Tsigma_all,2) + pow(Tmean_all,2) );
                //      double tsampfirst = fSamplePeriod * 0.5;
                //      double tsamplast = fSamplePeriod * (fN_MPD_TIME_SAMP-0.5);
                //      double sampfirst = fADCsamples[ihit][0];
                //      double samplast = fADCsamples[ihit][fN_MPD_TIME_SAMP-1];

                //     double newTsum = oldTsum;
                //     double newT2sum = oldT2sum;

                //     if( fSuppressFirstLast > 0 ) { //exclude first and last time sample:
                //       ADC_strip[fStrip[ihit]] = fADCsums[ihit] - sampfirst - samplast;

                //       newTsum = oldTsum - tsampfirst * sampfirst - tsamplast * samplast;
                //       newT2sum = oldT2sum - pow(tsampfirst,2) * sampfirst - pow(tsamplast,2)*samplast;

                //     } else if( fSuppressFirstLast == -2 ){ //exclude last time sample only:
                //       ADC_strip[fStrip[ihit]] = fADCsums[ihit] - samplast;

                //       newTsum = oldTsum - tsamplast * samplast;
                //       newT2sum = oldT2sum - pow(tsamplast,2)*samplast;

                //     } else { //negative value other than -2: exclude first time sample only:
                //       ADC_strip[fStrip[ihit]] -= fADCsamples[ihit][0];

                //       newTsum = oldTsum - tsampfirst * sampfirst;
                //       newT2sum = oldT2sum - pow(tsampfirst,2)*sampfirst;

                //     }

                //     Tmean_strip[fStrip[ihit]] = newTsum / ADC_strip[fStrip[ihit]];
                //     Tsigma_strip[fStrip[ihit]] = sqrt( newT2sum/ ADC_strip[fStrip[ihit]] - pow(Tmean_strip[fStrip[ihit]],2) );

                //   }
                // }
            }
        }
        // Also add strips for negative signal clustering if fNegSignalStudy is true
        if (fAxis[ihit] == axis && fStripIsNeg[ihit] && fStrip_BUILD_ALL_SAMPLES[ihit] && !fStrip_ENABLE_CM[ihit] && fStrip_CM_GOOD[ihit] && fNegSignalStudy)
        {
            bool newstrip = (striplist_neg.insert(fStrip[ihit])).second;

            if (newstrip)
            { // should always be true:
                hitindex_neg[fStrip[ihit]] = ihit;
                if (axis == SBSGEM::kUaxis)
                {
                    pedrms_strip_neg[fStrip[ihit]] = fPedRMSU[fStrip[ihit]];
                }
                else
                {
                    pedrms_strip_neg[fStrip[ihit]] = fPedRMSV[fStrip[ihit]];
                }
            }
        }
    }

    std::set<UShort_t> localmaxima;
    std::map<UShort_t, bool> islocalmax;
    // std::map<UShort_t,bool> passed_constraint;

    //std::cout<<"xb debug:: total fired strips: "<<striplist.size()<<std::endl;
    for (std::set<UShort_t>::iterator i = striplist.begin(); i != striplist.end(); ++i)
    {
        int strip = *i;
        // int hitidx = hitindex[strip];
        islocalmax[strip] = false;

        // double sumstrip = fADCsums[hitindex[strip]];
        double sumstrip = ADC_strip[strip];
        double sumleft = 0.0;
        double sumright = 0.0;

        if (striplist.find(strip - 1) != striplist.end())
        {
            // sumleft = fADCsums[hitindex[strip-1]]; //if strip - 1 is found in strip list, hitindex is guaranteed to have been initialized above
            sumleft = ADC_strip[strip - 1]; // if strip - 1 is found in strip list, hitindex is guaranteed to have been initialized above
        }
        if (striplist.find(strip + 1) != striplist.end())
        {
            // sumright = fADCsums[hitindex[strip+1]];
            sumright = ADC_strip[strip + 1];
        }

        //std::cout<<"strip adc: "<<sumstrip<<", left strip adc: "<<sumleft<<", right strip adc: "<<sumright<<std::endl;
        //std::cout<<"seed adc sum threshold: "<<fThresholdStripSum<<", seed adc max threshold: "<<fThresholdSample<<std::endl;
        // apply additional thresholds on max. sample and strip sum for local maxima:
        // If a strip is not a local max, the threshold is just 5sigma above the noise
        // from decoding
        if (sumstrip >= sumleft && sumstrip >= sumright &&
            sumstrip >= fThresholdStripSum &&
            ADC_maxsamp[strip] >= fThresholdSample)
        {
            //	fADCmax[hitindex[strip]] >= fThresholdSample ){ //new local max:
            bool goodtime = true;
            if (fUseStripTimingCuts && fabs(Tmean_strip[strip] - fStripMaxTcut_central) > fStripMaxTcut_width)
                goodtime = false;

            if (goodtime)
            {
                islocalmax[strip] = true;
                localmaxima.insert(strip);
            }
        }
    } // end loop over list of strips along this axis:

    //std::cout << "xb debug:: before peak erasing, n local maxima = " << localmaxima.size() << std::endl;

    std::vector<int> peakstoerase;

    // now calculate "prominence" for all peaks and erase "insignificant" peaks:

    for (std::set<UShort_t>::iterator i = localmaxima.begin(); i != localmaxima.end(); ++i)
    {
        int stripmax = *i;

        // double ADCmax = fADCsums[hitindex[stripmax]];
        double ADCmax = ADC_strip[stripmax];
        double prominence = ADCmax;

        int striplo = stripmax, striphi = stripmax;
        double ADCminright = ADCmax, ADCminleft = ADCmax;

        bool higherpeakright = false, higherpeakleft = false;
        int peakright = -1, peakleft = -1;

        while (striplist.find(striphi + 1) != striplist.end())
        {
            striphi++;

            // Double_t ADCtest = fADCsums[hitindex[striphi]];
            Double_t ADCtest = ADC_strip[striphi];

            if (ADCtest < ADCminright && !higherpeakright)
            { // as long as we haven't yet found a higher peak to the right, this is the lowest point between the current maximum and the next higher peak to the right:
                ADCminright = ADCtest;
            }

            if (islocalmax[striphi] && ADCtest > ADCmax)
            { // then this peak is in a contiguous group with another higher peak to the right:
                higherpeakright = true;
                peakright = striphi;
            }
        }

        while (striplist.find(striplo - 1) != striplist.end())
        {
            striplo--;
            // Double_t ADCtest = fADCsums[hitindex[striplo]];
            Double_t ADCtest = ADC_strip[striplo];
            if (ADCtest < ADCminleft && !higherpeakleft)
            { // as long as we haven't yet found a higher peak to the left, this is the lowest point between the current maximum and the next higher peak to the left:
                ADCminleft = ADCtest;
            }

            if (islocalmax[striplo] && ADCtest > ADCmax)
            { // then this peak is in a contiguous group with another higher peak to the left:
                higherpeakleft = true;
                peakleft = striplo;
            }
        }

        //    double sigma_sum = sqrt( double(fN_MPD_TIME_SAMP) )*fZeroSuppressRMS; //~25-50 ADC

        // the above calculation is not consistent with the intended usages of the above variables.
        // We should instead use the pedestal RMS value for the strip in question. The strip RMS values
        // represent the RMS of the AVERAGE of the samples. So the prominence threshold should be expressed in terms of the same thing to be consistent:
        double sigma_sum = double(fN_MPD_TIME_SAMP) * pedrms_strip[stripmax]; // Typically 60-70 ADC. Since pedrms_strip represents the rms of the sum of the ADC samples divided by the number of samples, to get the RMS of the sum, we need only multiply by the number of samples.
        // A 5-sigma threshold on this quantity would typically be about 300-350 ADC.

        bool peak_close = false;
        if (!higherpeakleft)
            ADCminleft = 0.0;
        if (!higherpeakright)
            ADCminright = 0.0;

        if (higherpeakright || higherpeakleft)
        {                                                            // this peak is contiguous with higher peaks on either the left or right or both:
            prominence = ADCmax - std::max(ADCminleft, ADCminright); // subtract the higher of the two valleys to get the prominence

            if (higherpeakleft && std::abs(peakleft - stripmax) <= 2 * maxsep)
                peak_close = true;
            if (higherpeakright && std::abs(peakright - stripmax) <= 2 * maxsep)
                peak_close = true;

            if (peak_close && (prominence < fThresh_2ndMax_nsigma * sigma_sum ||
                               prominence / ADCmax < fThresh_2ndMax_fraction))
            {
                peakstoerase.push_back(stripmax);
            }
        }
    }

    // Erase "insignificant" peaks (those in contiguous grouping with higher peak with prominence below thresholds):
    for (int ipeak : peakstoerase)
    {
        localmaxima.erase(ipeak);
        islocalmax[ipeak] = false;
    }

    //std::cout << "xb debug:: After peak erasing, n local maxima = " << localmaxima.size() << std::endl;
    // for speed/efficiency, resize the cluster array to the theoretical maximum
    // and use operator[] rather than push_back:
    clusters.resize(localmaxima.size());

    // nclust_tot = localmaxima.size();

    // Cluster formation and cluster splitting from remaining local maxima:
    for (auto i = localmaxima.begin(); i != localmaxima.end(); ++i)
    {
        int stripmax = *i;
        int striplo = stripmax;
        int striphi = stripmax;
        // double ADCmax = fADCsums[hitindex[stripmax]];
        double ADCmax = ADC_strip[stripmax];

        bool found_neighbor_low = true;

        // double Tfit =

        // while( striplist.find( striplo-1 ) != striplist.end() &&
        //	   stripmax - striplo < maxsep ){
        while (found_neighbor_low)
        {

            found_neighbor_low = striplist.find(striplo - 1) != striplist.end() && stripmax - striplo < maxsep;

            if (found_neighbor_low && fUseStripTimingCuts && fDeconvolutionFlag == 0)
            {
                // check time difference and correlation coefficient of the candidate strip
                // with the max strip:
                double Tdiff = fTmean[hitindex[striplo - 1]] - fTmean[hitindex[stripmax]];
                if (fabs(Tdiff) > fStripAddTcut_width)
                    found_neighbor_low = false;
            }

            double Ccoeff = CorrCoeff(NsampCorrCoeff, fADCsamples[hitindex[striplo - 1]], fADCsamples[hitindex[stripmax]], FirstSampleCorrCoeff);

            double Ccoeff_deconv = CorrCoeff(fN_MPD_TIME_SAMP, fADCsamples_deconv[hitindex[striplo - 1]], fADCsamples_deconv[hitindex[stripmax]]);
            // if( fDeconvolutionFlag != 0 ){
            // 	Ccoeff = CorrCoeff( fN_MPD_TIME_SAMP, fADCsamples_deconv[hitindex[striplo-1]], fADCsamples_deconv[hitindex[stripmax]] );
            // }

            // if the greater of the two correlation coefficients (shaped samples vs. deconvoluted samples) is too low, don't add this strip:
            if (std::max(Ccoeff, Ccoeff_deconv) < fStripAddCorrCoeffCut)
                found_neighbor_low = false;

            // If either the strip time difference with the max. strip or the Correlation coefficient with the max strip
            // fails the cuts, stop growing the cluster in this direction

            if (found_neighbor_low)
                striplo--;
        }

        // while( striplist.find( striphi+1 ) != striplist.end() &&
        // 	   striphi - stripmax < maxsep ){
        //   striphi++;
        // }

        bool found_neighbor_high = true;

        while (found_neighbor_high)
        {

            found_neighbor_high = striplist.find(striphi + 1) != striplist.end() && striphi - stripmax < maxsep;

            if (found_neighbor_high && fUseStripTimingCuts && fDeconvolutionFlag == 0)
            {
                // check time difference and correlation coefficient of the candidate strip
                // with the max strip:
                double Tdiff = fTmean[hitindex[striphi + 1]] - fTmean[hitindex[stripmax]];
                if (fabs(Tdiff) > fStripAddTcut_width)
                    found_neighbor_high = false;
            }

            double Ccoeff = CorrCoeff(NsampCorrCoeff, fADCsamples[hitindex[striphi + 1]], fADCsamples[hitindex[stripmax]], FirstSampleCorrCoeff);

            double Ccoeff_deconv = CorrCoeff(fN_MPD_TIME_SAMP, fADCsamples_deconv[hitindex[striphi + 1]], fADCsamples_deconv[hitindex[stripmax]]);

            if (std::max(Ccoeff, Ccoeff_deconv) < fStripAddCorrCoeffCut)
                found_neighbor_high = false;

            // If either the strip time difference with the max. strip or the Correlation coefficient with the max strip
            // fails the cuts, stop growing the cluster in this direction

            if (found_neighbor_high)
                striphi++;
        }

        int nstrips = striphi - striplo + 1;

        double sumx = 0.0, sumx2 = 0.0, sumADC = 0.0, sumt = 0.0, sumt2 = 0.0;
        double sumwx = 0.0;
        double sumtdeconv = 0.0;
        double sumADCdeconv = 0.0;
        // double sumADCdeconv_combo = 0.0;

        std::map<int, double> splitfraction;
        std::vector<double> stripADCsum(nstrips);

        double maxpos = (stripmax + 0.5 - 0.5 * Nstrips) * pitch + offset;

        // If peak position falls inside the "track search region" constraint, add a new cluster:
        // if( fabs( maxpos - constraint_center ) <= constraint_width ){
        // Move constraint check to later so we can filter the "total cluster multiplicity" by basic quality criteria:
        // This MIGHT slow down analysis at higher occupancies, but we'll have to see how noticeable it is:

        // create a cluster, but don't add it to the 1D cluster array unless it passes the track search region constraint:
        sbsgemcluster_t clusttemp;
        clusttemp.nstrips = nstrips;
        clusttemp.istriplo = striplo;
        clusttemp.istriphi = striphi;
        clusttemp.istripmax = stripmax;
        clusttemp.ADCsamples.resize(fN_MPD_TIME_SAMP);
        clusttemp.DeconvADCsamples.resize(fN_MPD_TIME_SAMP);
        clusttemp.stripADCsum.clear();
        clusttemp.DeconvADCsum.clear();
        clusttemp.hitindex.clear();
        clusttemp.rawstrip = fStripRaw[hitindex[stripmax]];
        clusttemp.rawMPD = fStripMPD[hitindex[stripmax]];
        clusttemp.rawAPV = fStripADC_ID[hitindex[stripmax]];

        for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
        { // initialize cluster-summed ADC samples to zero:
            clusttemp.ADCsamples[isamp] = 0.0;
            clusttemp.DeconvADCsamples[isamp] = 0.0;
        }

        for (int istrip = striplo; istrip <= striphi; istrip++)
        {
            // int nmax_strip = 1;
            double sumweight = ADCmax / (1.0 + pow((stripmax - istrip) * pitch / fSigma_hitshape, 2));
            double maxweight = sumweight;
            for (int jstrip = istrip - maxsep; jstrip <= istrip + maxsep; jstrip++)
            {
                if (localmaxima.find(jstrip) != localmaxima.end() && jstrip != stripmax)
                {
                    sumweight += ADC_strip[jstrip] / (1.0 + pow((jstrip - istrip) * pitch / fSigma_hitshape, 2));
                }
            }

            splitfraction[istrip] = maxweight / sumweight;

            double hitpos = (istrip + 0.5 - 0.5 * Nstrips) * pitch + offset; // local hit position along direction measured by these strips
            double ADCstrip = ADC_strip[istrip] * splitfraction[istrip];
            // double tstrip = fTmean[hitindex[istrip]];
            double tstrip = Tmean_strip[istrip];

            double tstrip_deconv = fTmeanDeconv[hitindex[istrip]];
            double ADCstrip_deconv = fADCsumsDeconv[hitindex[istrip]] * splitfraction[istrip];

            for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
            {
                clusttemp.ADCsamples[isamp] += fADCsamples[hitindex[istrip]][isamp] * splitfraction[istrip];
                clusttemp.DeconvADCsamples[isamp] += fADCsamples_deconv[hitindex[istrip]][isamp] * splitfraction[istrip];
            }

            clusttemp.stripADCsum.push_back(ADCstrip);
            clusttemp.DeconvADCsum.push_back(ADCstrip_deconv);

            clusttemp.hitindex.push_back(hitindex[istrip]); // do we use this anywhere? Yes, it is good to keep track of this if we want to access raw strip info later on
            sumADC += ADCstrip;

            sumADCdeconv += ADCstrip_deconv;
            // sumADCdeconv_combo += fADCmaxDeconvCombo[hitindex[istrip]]*splitfraction[istrip];

            if (std::abs(istrip - stripmax) <= std::max(UShort_t(1), std::min(maxsepcoord, maxsep)))
            {
                sumx += hitpos * ADCstrip;
                sumx2 += pow(hitpos, 2) * ADCstrip;
                sumwx += ADCstrip;
                // use same strip cuts for cluster timing determination as for position reconstruction: may revisit later:
                sumt += tstrip * ADCstrip;
                sumt2 += pow(tstrip, 2) * ADCstrip;
                sumtdeconv += tstrip_deconv * ADCstrip;
            }
        }
        clusttemp.isampmaxDeconv = 0;
        clusttemp.icombomaxDeconv = 0;
        clusttemp.isampmax = 0; // figure out time sample in which peak of cluster-summed ADC values occurs:
        double maxADC = 0.0;
        double maxADC_deconv = 0.0;
        double maxADCcombo_deconv = 0.0;

        for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
        {
            if (isamp == 0 || clusttemp.ADCsamples[isamp] > maxADC)
            {
                maxADC = clusttemp.ADCsamples[isamp];
                clusttemp.isampmax = isamp;
            }

            if (isamp == 0 || clusttemp.DeconvADCsamples[isamp] > maxADC_deconv)
            {
                maxADC_deconv = clusttemp.DeconvADCsamples[isamp];
                clusttemp.isampmaxDeconv = isamp;
            }

            double combotemp = clusttemp.DeconvADCsamples[isamp];
            if (isamp == 0)
            {
                clusttemp.icombomaxDeconv = 0;
                maxADCcombo_deconv = combotemp;
            }
            else
            {
                combotemp += clusttemp.DeconvADCsamples[isamp - 1];
                if (combotemp > maxADCcombo_deconv)
                {
                    maxADCcombo_deconv = combotemp;
                    clusttemp.icombomaxDeconv = isamp;
                }
            }
            if (isamp + 1 == fN_MPD_TIME_SAMP && clusttemp.DeconvADCsamples[isamp] > maxADCcombo_deconv)
            {
                maxADCcombo_deconv = clusttemp.DeconvADCsamples[isamp];
                clusttemp.icombomaxDeconv = fN_MPD_TIME_SAMP;
            }
        }

        clusttemp.hitpos_mean = sumx / sumwx;
        clusttemp.hitpos_sigma = sqrt(sumx2 / sumwx - pow(clusttemp.hitpos_mean, 2));
        clusttemp.clusterADCsum = sumADC;
        clusttemp.clusterADCsumDeconv = sumADCdeconv;
        clusttemp.clusterADCsumDeconvMaxCombo = maxADCcombo_deconv;
        clusttemp.t_mean = sumt / sumwx;
        clusttemp.t_sigma = sqrt(sumt2 / sumwx - pow(clusttemp.t_mean, 2));

        clusttemp.t_mean_deconv = sumtdeconv / sumwx;

        // initialize "keep" flag for all 1D clusters to true:
        clusttemp.keep = true;

        clusttemp.isneg = false;        // This is used for negative strip analysis
        clusttemp.isnegontrack = false; // This is used for negative strip analysis

        // In the standalone we don't apply an independent threshold on the cluster sum in the context of 1D cluster-finding:
        // if( sumADC >= fThresholdClusterSum ){
        // if( axis == SBSGEM::kVaxis ){
        // 	fVclusters.push_back( clusttemp );
        // 	fNclustV++;
        // } else {
        // 	fUclusters.push_back( clusttemp );
        // 	fNclustU++;
        // }
        if (sumADC >= fThresholdClusterSum && clusttemp.nstrips >= 2)
        { // Increment "total cluster multiplicity"
            nclust_tot++;
        }

        // Hopefully this works correctly:
        if (fabs(clusttemp.hitpos_mean - constraint_center) <= constraint_width)
        {

            // Fit max strip time for hits in constraint region:
            double Tfit = FitStripTime(hitindex[stripmax], 20.0);
            fStripTfit[hitindex[stripmax]] = Tfit;

            // if( fabs( Tfit - 25.0 ) < 10.0 ){ //Experimental, crude hack for testing:

            clusters[nclust] = clusttemp;
            nclust++;
            nclust_pos++;
            //}

            // std::cout << "found cluster, (axis, istripmax, nstrips, ADCsum, hit pos (mm) )=(" << axis << ", " << clusttemp.istripmax << ", "
            // 	  << clusttemp.nstrips << ", " << clusttemp.clusterADCsum
            // 	  << ", " << clusttemp.hitpos_mean*1000.0 << ")" << std::endl;

            //}
        } // Check if peak is inside track search region constraint
    }     // end loop on local maxima

    // Fill cluster multiplicity and "hit rate" histograms:
    if (fMakeEfficiencyPlots && fEfficiencyInitialized && hClusterBasedOccupancyUstrips != nullptr && hClusterBasedOccupancyVstrips != nullptr && hClusterMultiplicityUstrips != nullptr && hClusterMultiplicityVstrips != nullptr)
    {

        // We are using units of kHz/cm^2 for our "rate" plot:
        // Timing window size = cut width:

        // The following is in ns, we want to convert to milliseconds, so need to DIVIDE by 1e6:
        double window = fStripMaxTcut_width; // later we can use a fancier definition:
        if (!fUseStripTimingCuts)
            window = fN_MPD_TIME_SAMP * fSamplePeriod; // If we aren't using strip timing cuts we take the entire six-sample window as the time interval for "occupancy"
        // The following is in m^2, need to multiply by 1e4
        double area = GetXSize() * GetYSize();

        // window * area = ns * m^2 * 1e4 cm^2 /m^2 * 1e-6 ms/ns
        double ratefac = 2.0 * window * area / 100.0; // ms * cm^2

        if (axis == SBSGEM::kUaxis)
        {
            hClusterBasedOccupancyUstrips->Fill(double(nclust_tot) / ratefac);
            hClusterMultiplicityUstrips->Fill(double(nclust_tot));
        }
        else
        {
            hClusterBasedOccupancyVstrips->Fill(double(nclust_tot) / ratefac);
            hClusterMultiplicityVstrips->Fill(double(nclust_tot));
        }
    }

    // clusters.resize(nclust); //just to make sure no pathological behavior later on

    //std::cout << "xb debug::: number of clusters found = " << nclust << std::endl;

    filter_1Dhits(axis);
    // filter_1Dhits(SBSGEM::kVaxis);

    ////// Below is an exact copy of the parts above but clusters negative strips instead. This works by adding -1 factors
    ////// whenever we need the ADC value. Then at the end when saving the cluster information we flip the ADC back
    ////// negative again.
    localmaxima.clear();
    islocalmax.clear();

    // Do clustering again but for negative strips
    for (std::set<UShort_t>::iterator i = striplist_neg.begin(); i != striplist_neg.end(); ++i)
    {
        int strip = *i;
        // int hitidx = hitindex_neg[strip];
        islocalmax[strip] = false;

        double sumstrip = -1 * fADCsums[hitindex_neg[strip]]; // Add -1 factor
        double sumleft = 0.0;
        double sumright = 0.0;

        if (striplist_neg.find(strip - 1) != striplist_neg.end())
        {
            sumleft = -1 * fADCsums[hitindex_neg[strip - 1]]; // if strip - 1 is found in strip list, hitindex_neg is guaranteed to have been initialized above
        }
        if (striplist_neg.find(strip + 1) != striplist_neg.end())
        {
            sumright = -1 * fADCsums[hitindex_neg[strip + 1]];
        }

        // apply additional thresholds on max. sample and strip sum for local maxima:
        // If a strip is not a local max, the threshold is just 5sigma above the noise
        // from decoding
        //-1 factor added in if statuement
        if (sumstrip >= sumleft && sumstrip >= sumright &&
            sumstrip >= fThresholdStripSum &&
            -1 * fADCmax[hitindex_neg[strip]] >= fThresholdSample)
        { // new local max:
            islocalmax[strip] = true;
            localmaxima.insert(strip);
        }
    } // end loop over list of strips along this axis:

    //  cout << "before peak erasing, n local maxima = " << localmaxima.size() << endl;

    peakstoerase.clear();

    // now calculate "prominence" for all peaks and erase "insignificant" peaks:

    for (std::set<UShort_t>::iterator i = localmaxima.begin(); i != localmaxima.end(); ++i)
    {
        int stripmax = *i;

        // Added -1 factor
        double ADCmax = -1 * fADCsums[hitindex_neg[stripmax]];
        double prominence = -1 * ADCmax;

        int striplo = stripmax, striphi = stripmax;
        double ADCminright = ADCmax, ADCminleft = ADCmax;

        bool higherpeakright = false, higherpeakleft = false;
        int peakright = -1, peakleft = -1;

        while (striplist_neg.find(striphi + 1) != striplist_neg.end())
        {
            striphi++;

            Double_t ADCtest = -1 * fADCsums[hitindex_neg[striphi]]; // Added -1 factor

            if (ADCtest < ADCminright && !higherpeakright)
            { // as long as we haven't yet found a higher peak to the right, this is the lowest point between the current maximum and the next higher peak to the right:
                ADCminright = ADCtest;
            }

            if (islocalmax[striphi] && ADCtest > ADCmax)
            { // then this peak is in a contiguous group with another higher peak to the right:
                higherpeakright = true;
                peakright = striphi;
            }
        }

        while (striplist_neg.find(striplo - 1) != striplist_neg.end())
        {
            striplo--;
            Double_t ADCtest = -1 * fADCsums[hitindex_neg[striplo]]; // Added -q factor
            if (ADCtest < ADCminleft && !higherpeakleft)
            { // as long as we haven't yet found a higher peak to the left, this is the lowest point between the current maximum and the next higher peak to the left:
                ADCminleft = ADCtest;
            }

            if (islocalmax[striplo] && ADCtest > ADCmax)
            { // then this peak is in a contiguous group with another higher peak to the left:
                higherpeakleft = true;
                peakleft = striplo;
            }
        }

        //    double sigma_sum = sqrt( double(fN_MPD_TIME_SAMP) )*fZeroSuppressRMS; //~25-50 ADC

        // the above calculation is not consistent with the intended usages of the above variables.
        // We should instead use the pedestal RMS value for the strip in question. The strip RMS values
        // represent the RMS of the AVERAGE of the samples. So the prominence threshold should be expressed in terms of the same thing to be consistent:
        double sigma_sum = double(fN_MPD_TIME_SAMP) * pedrms_strip_neg[stripmax]; // Typically 60-70 ADC. Since pedrms_strip represents the rms of the sum of the ADC samples divided by the number of samples, to get the RMS of the sum, we need only multiply by the number of samples.
        // A 5-sigma threshold on this quantity would typically be about 300-350 ADC.

        bool peak_close = false;
        if (!higherpeakleft)
            ADCminleft = 0.0;
        if (!higherpeakright)
            ADCminright = 0.0;

        if (higherpeakright || higherpeakleft)
        {                                                            // this peak is contiguous with higher peaks on either the left or right or both:
            prominence = ADCmax - std::max(ADCminleft, ADCminright); // subtract the higher of the two valleys to get the prominence

            if (higherpeakleft && std::abs(peakleft - stripmax) <= 2 * maxsep)
                peak_close = true;
            if (higherpeakright && std::abs(peakright - stripmax) <= 2 * maxsep)
                peak_close = true;

            if (peak_close && (prominence < fThresh_2ndMax_nsigma * sigma_sum ||
                               prominence / ADCmax < fThresh_2ndMax_fraction))
            {
                peakstoerase.push_back(stripmax);
            }
        }
    }

    // Erase "insignificant" peaks (those in contiguous grouping with higher peak with prominence below thresholds):
    for (int ipeak : peakstoerase)
    {
        localmaxima.erase(ipeak);
        islocalmax[ipeak] = false;
    }

    // cout << "After peak erasing, n local maxima = " << localmaxima.size() << endl;
    // for speed/efficiency, resize the cluster array to the theoretical maximum
    // and use operator[] rather than push_back:
    // The clusters array will have negative and positive clusters. So we add the
    // maximum negative clusters (localmaxima.size()) with the positive clusters
    // found earlier (nclust).
    clusters.resize(localmaxima.size() + nclust);

    // Cluster formation and cluster splitting from remaining local maxima:
    for (auto i = localmaxima.begin(); i != localmaxima.end(); ++i)
    {
        int stripmax = *i;
        int striplo = stripmax;
        int striphi = stripmax;
        double ADCmax = -1 * fADCsums[hitindex_neg[stripmax]]; // added -1 factor

        bool found_neighbor_low = true;

        // while( striplist_neg.find( striplo-1 ) != striplist_neg.end() &&
        //	   stripmax - striplo < maxsep ){
        while (found_neighbor_low)
        {

            found_neighbor_low = striplist_neg.find(striplo - 1) != striplist_neg.end() && stripmax - striplo < maxsep;

            if (found_neighbor_low && fUseStripTimingCuts)
            {
                // check time difference and correlation coefficient of the candidate strip
                // with the max strip:
                double Tdiff = fTmean[hitindex_neg[striplo - 1]] - fTmean[hitindex_neg[stripmax]];
                if (fabs(Tdiff) > fStripAddTcut_width)
                    found_neighbor_low = false;
                double Ccoeff = CorrCoeff(fN_MPD_TIME_SAMP, fADCsamples[hitindex_neg[striplo - 1]], fADCsamples[hitindex_neg[stripmax]]);
                if (Ccoeff < fStripAddCorrCoeffCut)
                    found_neighbor_low = false;
            }

            // If either the strip time difference with the max. strip or the Correlation coefficient with the max strip
            // fails the cuts, stop growing the cluster in this direction

            if (found_neighbor_low)
                striplo--;
        }

        // while( striplist_neg.find( striphi+1 ) != striplist_neg.end() &&
        // 	   striphi - stripmax < maxsep ){
        //   striphi++;
        // }

        bool found_neighbor_high = true;

        while (found_neighbor_high)
        {

            found_neighbor_high = striplist_neg.find(striphi + 1) != striplist_neg.end() && striphi - stripmax < maxsep;

            if (found_neighbor_high && fUseStripTimingCuts)
            {
                // check time difference and correlation coefficient of the candidate strip
                // with the max strip:
                double Tdiff = fTmean[hitindex_neg[striphi + 1]] - fTmean[hitindex_neg[stripmax]];
                if (fabs(Tdiff) > fStripAddTcut_width)
                    found_neighbor_high = false;
                double Ccoeff = CorrCoeff(fN_MPD_TIME_SAMP, fADCsamples[hitindex_neg[striphi + 1]], fADCsamples[hitindex_neg[stripmax]]);
                if (Ccoeff < fStripAddCorrCoeffCut)
                    found_neighbor_high = false;
            }

            // If either the strip time difference with the max. strip or the Correlation coefficient with the max strip
            // fails the cuts, stop growing the cluster in this direction

            if (found_neighbor_high)
                striphi++;
        }

        int nstrips = striphi - striplo + 1;

        double sumx = 0.0, sumx2 = 0.0, sumADC = 0.0, sumt = 0.0, sumt2 = 0.0;
        double sumwx = 0.0;

        std::map<int, double> splitfraction;
        std::vector<double> stripADCsum(nstrips);

        double maxpos = (stripmax + 0.5 - 0.5 * Nstrips) * pitch + offset;

        // If peak position falls inside the "track search region" constraint, add a new cluster:
        if (fabs(maxpos - constraint_center) <= constraint_width)
        {

            // create a cluster, but don't add it to the 1D cluster array unless it passes the track search region constraint:
            sbsgemcluster_t clusttemp;
            clusttemp.nstrips = nstrips;
            clusttemp.istriplo = striplo;
            clusttemp.istriphi = striphi;
            clusttemp.istripmax = stripmax;
            clusttemp.ADCsamples.resize(fN_MPD_TIME_SAMP);
            clusttemp.stripADCsum.clear();
            clusttemp.hitindex.clear();
            clusttemp.rawstrip = fStripRaw[hitindex_neg[stripmax]];
            clusttemp.rawMPD = fStripMPD[hitindex_neg[stripmax]];
            clusttemp.rawAPV = fStripADC_ID[hitindex_neg[stripmax]];

            for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
            { // initialize cluster-summed ADC samples to zero:
                clusttemp.ADCsamples[isamp] = 0.0;
            }

            for (int istrip = striplo; istrip <= striphi; istrip++)
            {
                // int nmax_strip = 1;
                double sumweight = ADCmax / (1.0 + pow((stripmax - istrip) * pitch / fSigma_hitshape, 2));
                double maxweight = sumweight;

                for (int jstrip = istrip - maxsep; jstrip <= istrip + maxsep; jstrip++)
                {
                    if (localmaxima.find(jstrip) != localmaxima.end() && jstrip != stripmax)
                    {
                        sumweight += -1 * fADCsums[hitindex_neg[jstrip]] / (1.0 + pow((jstrip - istrip) * pitch / fSigma_hitshape, 2)); // Added -1 factor
                    }
                }

                splitfraction[istrip] = maxweight / sumweight;

                double hitpos = (istrip + 0.5 - 0.5 * Nstrips) * pitch + offset; // local hit position along direction measured by these strips
                double ADCstrip = fADCsums[hitindex_neg[istrip]] * splitfraction[istrip];
                double tstrip = fTmean[hitindex_neg[istrip]];

                for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                {
                    // Do not add -1 factor. We are now saving the negative cluster information
                    clusttemp.ADCsamples[isamp] += fADCsamples[hitindex_neg[istrip]][isamp] * splitfraction[istrip];
                }

                clusttemp.stripADCsum.push_back(ADCstrip);

                clusttemp.hitindex.push_back(hitindex_neg[istrip]); // do we use this anywhere? Yes, it is good to keep track of this if we want to access raw strip info later on

                sumADC += ADCstrip;

                // The variables below do not change if the ADC is negative or positive so we leave them alone
                if (std::abs(istrip - stripmax) <= std::max(UShort_t(1), std::min(maxsepcoord, maxsep)))
                {
                    sumx += hitpos * ADCstrip;
                    sumx2 += pow(hitpos, 2) * ADCstrip;
                    sumwx += ADCstrip;
                    // use same strip cuts for cluster timing determination as for position reconstruction: may revisit later:
                    sumt += tstrip * ADCstrip;
                    sumt2 += pow(tstrip, 2) * ADCstrip;
                }
            }

            clusttemp.isampmax = 0; // figure out time sample in which peak of cluster-summed ADC values occurs:
            double minADC = 10000;
            for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
            {
                if (isamp == 0 || clusttemp.ADCsamples[isamp] < minADC)
                {
                    minADC = clusttemp.ADCsamples[isamp];
                    clusttemp.isampmax = isamp;
                }
            }

            clusttemp.hitpos_mean = sumx / sumwx;
            clusttemp.hitpos_sigma = sqrt(sumx2 / sumwx - pow(clusttemp.hitpos_mean, 2));
            clusttemp.clusterADCsum = sumADC;
            clusttemp.t_mean = sumt / sumwx;
            clusttemp.t_sigma = sqrt(sumt2 / sumwx - pow(clusttemp.t_mean, 2));

            // initialize "keep" flag to false for negative strips:
            clusttemp.keep = false;

            clusttemp.isneg = true;         // This is used for negative strip analysis
            clusttemp.isnegontrack = false; // This is used for negative strip analysis

            // In the standalone we don't apply an independent threshold on the cluster sum in the context of 1D cluster-finding:
            // if( sumADC >= fThresholdClusterSum ){
            // if( axis == SBSGEM::kVaxis ){
            // 	fVclusters.push_back( clusttemp );
            // 	fNclustV++;
            // } else {
            // 	fUclusters.push_back( clusttemp );
            // 	fNclustU++;
            // }

            // Hopefully this works correctly:
            clusters[nclust] = clusttemp;
            nclust++;
            nclust_neg++;

            // std::cout << "found cluster, (axis, istripmax, nstrips, ADCsum, hit pos (mm) )=(" << axis << ", " << clusttemp.istripmax << ", "
            // 	  << clusttemp.nstrips << ", " << clusttemp.clusterADCsum
            // 	  << ", " << clusttemp.hitpos_mean*1000.0 << ")" << std::endl;

            //}
        } // Check if peak is inside track search region constraint
    }     // end loop on local maxima

    clusters.resize(nclust); // just to make sure no pathological behavior later on
}

// Experimental, not used for now:
double GEMModule::FitStripTime(int striphitindex, double RMS)
{
    if (striphitindex < 0 || striphitindex > fNstrips_hit)
        return -1000.0;

    // double Ttest_min = -fN_MPD_TIME_SAMP * fSamplePeriod;
    // double Ttest_max = fN_MPD_TIME_SAMP * fSamplePeriod;
    // double Tstep = fSamplePeriod/25.0; //scan t0 in 1 ns steps

    // //fStripTimeFunc
    // // fStripTimeFunc->FixParameter(2,fStripTau);
    // // fStripTimeFunc->SetParameter(1,fTmean[striphitindex]);
    // // fStripTimeFunc->FixParameter(0,fADCmax[striphitindex])

    // //int isampmax = fMaxSamp[striphitindex];
    // double ADCmax = fADCmax[striphitindex];

    // double minchi2 = 0.0;
    // bool first = true;

    // double Tbest = 0.0;

    // double Ttest = Ttest_min;
    // while( Ttest < Ttest_max ){

    //   double chi2 = 0.0;
    //   for( int isamp=0; isamp<fN_MPD_TIME_SAMP; isamp++ ){
    //     double Tsamp = fSamplePeriod * (isamp + 0.5);

    //     double ADCtheory = ADCmax*exp(1.0)/fStripTau * (Tsamp - Ttest) * exp(-(Tsamp-Ttest)/fStripTau );
    //     chi2 += pow( (ADCtheory - fADCsamples[striphitindex][isamp])/RMS, 2 );
    //   }

    //   if ( first || chi2 < minchi2 ){
    //     minchi2 = chi2;
    //     Tbest = Ttest;
    //     first = false;
    //   }
    //   Ttest += Tstep;
    // }
    double xtemp[fN_MPD_TIME_SAMP], ytemp[fN_MPD_TIME_SAMP], extemp[fN_MPD_TIME_SAMP], eytemp[fN_MPD_TIME_SAMP];

    for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
    {
        xtemp[isamp] = fSamplePeriod * (isamp + 0.5);
        ytemp[isamp] = fADCsamples[striphitindex][isamp];
        extemp[isamp] = 0.0;
        eytemp[isamp] = RMS;
    }

    fStripTimeFunc->SetParameter(0, fADCmax[striphitindex]);
    fStripTimeFunc->SetParameter(1, 0.0);
    fStripTimeFunc->SetParameter(2, 50.0);
    fStripTimeFunc->SetParameter(3, 0.0);

    TGraph *gtemp = new TGraphErrors(fN_MPD_TIME_SAMP, xtemp, ytemp, extemp, eytemp);
    gtemp->Fit(fStripTimeFunc, "SQ0");

    // std::cout << "Strip tmean, tfit = " << fTmean[striphitindex] << ", " << Tbest << std::endl;
    //  	    << " +/- " << fStripTimeFunc->GetParError(1) << std::endl;

    // double retval = fStripTimeFunc->GetParameter(1);

    gtemp->Delete();

    return fStripTimeFunc->GetParameter(1);
    // return Tbest;
}

void GEMModule::fill_2D_hit_arrays()
{
    //xb();
    // Clear out the 2D hit array to get rid of any leftover junk from prior events:
    // fHits.clear();
    fN2Dhits = 0;
    //std::cout<<"start .... from gem moduel: "<<fN2Dhits<<std::endl;
 
    // fHits.resize( fNclustU * fNclustV );
    // fHits.clear();
    fHits.resize(std::min(fNclustU * fNclustV, fMAX2DHITS));

    // if( fNclustU * fNclustV > fMAX2DHITS ){
    //    std::cout << "Warning in SBSGEMModule::fill_2D_hit_arrays():
    //  }

    int nsamp_corr = fN_MPD_TIME_SAMP;
    int firstsamp_corr = 0;

    // the following commented lines are experimental:
    //  if( fSuppressFirstLast > 0 ){
    //    nsamp_corr = fN_MPD_TIME_SAMP-2;
    //    firstsamp_corr = 1;
    //  }
    //  if( fSuppressFirstLast < 0 ){
    //    nsamp_corr = fN_MPD_TIME_SAMP-1;
    //    firstsamp_corr = (fSuppressFirstLast == -2 ? 0 : 1);
    //  }

    // This routine is simple: just form all possible 2D hits from combining one "U" cluster with one "V" cluster. Here we assume that find_clusters_1D has already
    // been called, if that is NOT the case, then this routine will just do nothing:
    bool maxhits_exceeded = false;
    //std::cout << "xb debug: " << fNclustU << ": " << fNclustV << std::endl;
    //xb();
    for (UInt_t iu = 0; iu < fNclustU; iu++)
    {
        for (UInt_t iv = 0; iv < fNclustV; iv++)
        {

            if (fUclusters[iu].keep && fVclusters[iv].keep)
            {
                // Initialize sums for computing cluster and strip correlation coefficients:
                sbsgemhit_t hittemp; // declare a temporary "hit" object:

                // copying overhead, might be inefficient:
                // sbsgemcluster_t uclusttemp = fUclusters[iu];
                // sbsgemcluster_t vclusttemp = fVclusters[iv];

                // Initialize "keep" to true:
                hittemp.keep = true;
                hittemp.highquality = false;
                hittemp.ontrack = false;
                hittemp.trackidx = -1;
                hittemp.iuclust = iu;
                hittemp.ivclust = iv;

                hittemp.uhit = fUclusters[iu].hitpos_mean;
                hittemp.vhit = fVclusters[iv].hitpos_mean;

                double pos_maxstripu = (fUclusters[iu].istripmax + 0.5 - 0.5 * fNstripsU) * fUStripPitch + fUStripOffset;
                double pos_maxstripv = (fVclusters[iv].istripmax + 0.5 - 0.5 * fNstripsV) * fVStripPitch + fVStripOffset;

                //"Cluster moments" defined as differences between reconstructed hit position and center of strip with max. signal in the cluster:
                hittemp.umom = (hittemp.uhit - pos_maxstripu) / fUStripPitch;
                hittemp.vmom = (hittemp.vhit - pos_maxstripv) / fVStripPitch;

                TVector2 UVtemp(hittemp.uhit, hittemp.vhit);
                TVector2 XYtemp = UVtoXY(UVtemp);

                hittemp.xhit = XYtemp.X();
                hittemp.yhit = XYtemp.Y();

                // Check if candidate 2D hit is inside the constraint region before doing anything else:
                if (fxcmin <= hittemp.xhit && hittemp.xhit <= fxcmax &&
                    fycmin <= hittemp.yhit && hittemp.yhit <= fycmax &&
                    IsInActiveArea(hittemp.xhit, hittemp.yhit))
                {

                    hittemp.thit = 0.5 * (fUclusters[iu].t_mean + fVclusters[iv].t_mean);
                    hittemp.Ehit = 0.5 * (fUclusters[iu].clusterADCsum + fVclusters[iv].clusterADCsum);

                    hittemp.thitcorr = hittemp.thit; // don't apply any corrections on thit yet

                    // Next up is to calculate "global" hit coordinates (actually coordinates in "tracker-local" system)
                    // DetToTrackCoord is a utility function defined in THaDetectorBase
                    TVector3 hitpos_global = DetToTrackCoord(hittemp.xhit, hittemp.yhit);

                    // Unclear whether it is actually necessary to store these variables, but we also probably want to avoid
                    // repeated calls to THaDetectorBase::DetToTrackCoord, so let's keep these for now:
                    hittemp.xghit = hitpos_global.X();
                    hittemp.yghit = hitpos_global.Y();
                    hittemp.zghit = hitpos_global.Z();

                    hittemp.ADCasym = (fUclusters[iu].clusterADCsum - fVclusters[iv].clusterADCsum) / (2.0 * hittemp.Ehit);

                    hittemp.ADCasymDeconv = (fUclusters[iu].clusterADCsumDeconvMaxCombo - fVclusters[iv].clusterADCsumDeconvMaxCombo) / (fUclusters[iu].clusterADCsumDeconvMaxCombo + fVclusters[iv].clusterADCsumDeconvMaxCombo);

                    hittemp.EhitDeconv = 0.5 * (fUclusters[iu].clusterADCsumDeconvMaxCombo + fVclusters[iv].clusterADCsumDeconvMaxCombo);

                    hittemp.tdiff = fUclusters[iu].t_mean - fVclusters[iv].t_mean;
                    hittemp.tdiffDeconv = fUclusters[iu].t_mean_deconv - fVclusters[iv].t_mean_deconv;
                    hittemp.thitDeconv = 0.5 * (fUclusters[iu].t_mean_deconv + fVclusters[iv].t_mean_deconv);

                    // Calculate correlation coefficients:
                    hittemp.corrcoeff_clust = CorrCoeff(nsamp_corr, fUclusters[iu].ADCsamples, fVclusters[iv].ADCsamples, firstsamp_corr);

                    // compute index of strip with max ADC sum within cluster strip array:
                    UInt_t ustripidx = fUclusters[iu].istripmax - fUclusters[iu].istriplo; //
                    UInt_t vstripidx = fVclusters[iv].istripmax - fVclusters[iv].istriplo; //

                    // compute index of strip with max ADC sum within decoded hit array:
                    UInt_t uhitidx = fUclusters[iu].hitindex[ustripidx];
                    UInt_t vhitidx = fVclusters[iv].hitindex[vstripidx];

                    hittemp.corrcoeff_strip = CorrCoeff(nsamp_corr, fADCsamples[uhitidx], fADCsamples[vhitidx], firstsamp_corr);

                    hittemp.corrcoeff_clust_deconv = CorrCoeff(fN_MPD_TIME_SAMP, fUclusters[iu].DeconvADCsamples, fVclusters[iv].DeconvADCsamples);
                    hittemp.corrcoeff_strip_deconv = CorrCoeff(fN_MPD_TIME_SAMP, fADCsamples_deconv[uhitidx], fADCsamples_deconv[vhitidx]);
                    hittemp.ADCasymDeconv = (fUclusters[iu].clusterADCsumDeconvMaxCombo - fVclusters[iv].clusterADCsumDeconvMaxCombo) /
                                            (fUclusters[iu].clusterADCsumDeconvMaxCombo + fVclusters[iv].clusterADCsumDeconvMaxCombo);
                    hittemp.EhitDeconv = 0.5 * (fUclusters[iu].clusterADCsumDeconvMaxCombo + fVclusters[iv].clusterADCsumDeconvMaxCombo);

                    // A "high-quality" hit is one that passes all the filtering criteria (might add more later):
                    hittemp.highquality = fabs(hittemp.ADCasym) <= fADCasymCut &&
                                          fUclusters[iu].nstrips > 1 && fVclusters[iv].nstrips > 1 &&
                                          fUclusters[iu].clusterADCsum >= fThresholdClusterSum &&
                                          fVclusters[iv].clusterADCsum >= fThresholdClusterSum &&
                                          hittemp.corrcoeff_clust >= fCorrCoeffCut &&
                                          hittemp.corrcoeff_strip >= fCorrCoeffCut &&
                                          fabs(hittemp.tdiff) <= fTimeCutUVdiff &&
                                          fabs(hittemp.ADCasymDeconv) <= fADCasymCut &&
                                          hittemp.corrcoeff_clust_deconv >= fCorrCoeffCut &&
                                          hittemp.corrcoeff_strip_deconv >= fCorrCoeffCut;

                    // cutting on deconvoluted ADC quantities could be dangerous before further study...

                    bool add_hit = true;
                    // we need special handling if we want to use single-strip clusters:
                    if (fUclusters[iu].nstrips == 1 || fVclusters[iv].nstrips == 1)
                    {
                        // If EITHER of these clusters is only single-strip, it must pass more stringent requirements to use as a 2D hit candidate:
                        // To use single-strip clusters, we will REQUIRE good timing, ADC asymmetry, and correlation coefficient cuts:
                        add_hit = false;
                        if (fabs(hittemp.ADCasym) <= fADCasymCut && fabs(hittemp.tdiff) <= fTimeCutUVdiff &&
                            hittemp.corrcoeff_strip >= fCorrCoeffCut && hittemp.corrcoeff_clust >= fCorrCoeffCut && fabs(hittemp.ADCasymDeconv) <= fADCasymCut &&
                            hittemp.corrcoeff_clust_deconv >= fCorrCoeffCut &&
                            hittemp.corrcoeff_strip_deconv >= fCorrCoeffCut)
                        {
                            add_hit = true;
                        }
                    }

                    // Okay, that should be everything. Now add it to the 2D hit array:
                    // fHits.push_back( hittemp );
                    if (add_hit && fN2Dhits < fMAX2DHITS)
                    {
                        fHits[fN2Dhits++] = hittemp; // should be faster than push_back();
                    }
                    else if (add_hit)
                    { //
                        maxhits_exceeded = true;
                    }
                    // fN2Dhits++;
                } // end check that 2D point is inside track search region
            }     // end check that both U and V clusters passed filtering criteria:
        }         // end loop over "V" clusters
    }             // end loop over "U" clusters
    //xb();
    if (maxhits_exceeded)
    {
        std::cout << "Warning in [SBSGEMModule::fill_2D_hit_arrays()]: good 2D hit candidates exceeded user maximum of " << fMAX2DHITS << " for module " << GetName() << ", 2D hit list truncated" << std::endl;
    }
    //std::cout<<"end .... from gem moduel: "<<fN2Dhits<<std::endl;
    fHits.resize(fN2Dhits);

    filter_2Dhits();
}

void GEMModule::filter_1Dhits(SBSGEM::GEMaxis_t axis)
{

    if (fFiltering_flag1D < 0)
        return; // flag < 0 means don't filter 1D clusters at all
    // flag = 0 means use a "soft" filter (only reject if at least one other cluster passed)
    // flag > 0 means use a "hard" filter (reject failing clusters no matter what)
    // First filter on cluster ADC sum:
    int ngood = 0;
    int nclust = (axis == SBSGEM::kUaxis) ? fNclustU : fNclustV;

    std::vector<sbsgemcluster_t> &clusters = (axis == SBSGEM::kUaxis) ? fUclusters : fVclusters;

    bool passed[nclust];

    for (int ipass = 0; ipass < 2; ipass++)
    {
        if (ipass == 0)
            ngood = 0;
        for (int icl = 0; icl < nclust; icl++)
        {

            // On the first pass, determine which clusters passed the criterion and count the number of good clusters:
            if (ipass == 0)
            {
                passed[icl] = clusters[icl].keep && clusters[icl].clusterADCsum >= fThresholdClusterSum;
                if (passed[icl])
                    ngood++;
            }

            // on the second pass, if at least one good cluster was found passing the criterion, we set "keep" for all others to false:
            if (ipass == 1 && !passed[icl] && (ngood > 0 || fFiltering_flag1D > 0))
            {
                clusters[icl].keep = false;
            }
        }
    }

    // Second, filter on cluster size (we may not actually want to filter on cluster size at this stage):
    ngood = 0;

    for (int ipass = 0; ipass < 2; ipass++)
    {
        if (ipass == 0)
            ngood = 0;

        for (int icl = 0; icl < nclust; icl++)
        {
            if (ipass == 0)
            {
                passed[icl] = clusters[icl].keep && clusters[icl].nstrips >= 2;
                if (passed[icl])
                    ngood++;
            }

            if (ipass == 1 && !passed[icl] && (ngood > 0 || fFiltering_flag1D > 0))
            {
                clusters[icl].keep = false;
            }
        }
    }
}

void GEMModule::filter_2Dhits()
{
    // plot 2d raw hits
    for(UInt_t ihit = 0; ihit < fN2Dhits; ihit++)
    {
        double x = fHits[ihit].xhit;
        double y = fHits[ihit].yhit;
        fhrawhitxy -> Fill(x, y);
    }

    // Here we will initially filter only based on time U/V time difference, ADC asymmetry, and perhaps correlation coefficient:

    if (fFiltering_flag2D < 0)
        return;

    // First U/V time difference:
    bool passed[fN2Dhits];
    int ngood = 0;
    for (UInt_t ipass = 0; ipass < 2; ipass++)
    {
        if (ipass == 0)
            ngood = 0;
        for (UInt_t ihit = 0; ihit < fN2Dhits; ihit++)
        {
            if (ipass == 0)
            {
                passed[ihit] = fHits[ihit].keep && fabs(fHits[ihit].tdiff) <= fTimeCutUVdiff;
                if (passed[ihit])
                    ngood++;
            }

            if (ipass == 1 && !passed[ihit] && (ngood > 0 || fFiltering_flag2D > 0))
            {
                fHits[ihit].keep = false;
            }
        }
    }

    // Second: Cluster Correlation Coefficient:
    ngood = 0;
    for (UInt_t ipass = 0; ipass < 2; ipass++)
    {
        if (ipass == 0)
            ngood = 0;
        for (UInt_t ihit = 0; ihit < fN2Dhits; ihit++)
        {
            if (ipass == 0)
            {
                passed[ihit] = fHits[ihit].keep && fHits[ihit].corrcoeff_clust >= fCorrCoeffCut;
                if (passed[ihit])
                    ngood++;
            }

            if (ipass == 1 && !passed[ihit] && (ngood > 0 || fFiltering_flag2D > 0))
            {
                fHits[ihit].keep = false;
            }
        }
    }

    // Third: ADC asymmetry:
    ngood = 0;
    for (UInt_t ipass = 0; ipass < 2; ipass++)
    {
        if (ipass == 0)
            ngood = 0;
        for (UInt_t ihit = 0; ihit < fN2Dhits; ihit++)
        {
            if (ipass == 0)
            {
                passed[ihit] = fHits[ihit].keep && fabs(fHits[ihit].ADCasym) <= fADCasymCut;
                if (passed[ihit])
                    ngood++;
            }

            if (ipass == 1 && !passed[ihit] && (ngood > 0 || fFiltering_flag2D > 0))
            {
                fHits[ihit].keep = false;
            }
        }
    }
}

//_____________________________________________________________________________
Bool_t GEMModule::IsInActiveArea(Double_t x, Double_t y) const
{
    // Check if given (x,y) coordinates are inside this detector's active area
    // (defined by fSize). x and y MUST be given in coordinates of this detector
    // plane, i.e. relative to fOrigin and measured along fXax and fYax.

    return (TMath::Abs(x) <= fSize[0] &&
            TMath::Abs(y) <= fSize[1]);
}

//_____________________________________________________________________________
Bool_t GEMModule::IsInActiveArea(const TVector3 &point) const
{
    // Check if given point 'point' is inside this detector's active area
    // (defined by fSize). 'point' is first projected onto the detector
    // plane (defined by fOrigin, fXax and fYax), and then the projected
    // (x,y) coordinates are checked against fSize.
    // Usually 'point' lies within the detector plane. If it does not,
    // be sure you know what you are doing.

    TVector3 v = TrackToDetCoord(point);
    return IsInActiveArea(v.X(), v.Y());
}

void GEMModule::InitAPVMAP()
{
    APVMAP[SBSGEM::kINFN].resize(fN_APV25_CHAN);
    APVMAP[SBSGEM::kUVA_XY].resize(fN_APV25_CHAN);
    APVMAP[SBSGEM::kUVA_UV].resize(fN_APV25_CHAN);
    APVMAP[SBSGEM::kMC].resize(fN_APV25_CHAN);

    for (UInt_t i = 0; i < fN_APV25_CHAN; i++)
    {
        Int_t strip1 = 32 * (i % 4) + 8 * (i / 4) - 31 * (i / 16);
        Int_t strip2 = strip1 + 1 + strip1 % 4 - 5 * ((strip1 / 4) % 2);
        Int_t strip3 = (strip2 % 2 == 0) ? strip2 / 2 + 32 : ((strip2 < 64) ? (63 - strip2) / 2 : 127 + (65 - strip2) / 2);
        APVMAP[SBSGEM::kINFN][i] = strip1;
        APVMAP[SBSGEM::kUVA_XY][i] = strip2;
        APVMAP[SBSGEM::kUVA_UV][i] = strip3;
        APVMAP[SBSGEM::kMC][i] = i;
    }

    // Print out to test:
    //  //std::cout << "INFN X/Y APV mapping: " << std::endl;
    //  for( UInt_t i=0; i<fN_APV25_CHAN; i++ ){
    //    std::cout << std::setw(8) << APVMAP[SBSGEM::kINFN][i] << ", ";
    //    if( (i+1) % 10 == 0 ) std::cout << endl;
    //  }
    //  std::cout << endl;

    // //Print out to test:
    // std::cout << "UVA X/Y APV mapping: " << std::endl;
    // for( UInt_t i=0; i<fN_APV25_CHAN; i++ ){
    //   std::cout << std::setw(8) << APVMAP[SBSGEM::kUVA_XY][i] << ", ";
    //   if( (i+1) % 10 == 0 ) std::cout << endl;
    // }
    // std::cout << endl;

    // //Print out to test:
    // std::cout << "UVA U/V APV mapping: " << std::endl;
    // for( UInt_t i=0; i<fN_APV25_CHAN; i++ ){
    //   std::cout << std::setw(8) << APVMAP[SBSGEM::kUVA_UV][i] << ", ";
    //   if( (i+1) % 10 == 0 ) std::cout << endl;
    // }
    // std::cout << endl;
}

Int_t GEMModule::GetStripNumber(UInt_t rawstrip, UInt_t pos, UInt_t invert)
{
    Int_t RstripNb = APVMAP[fAPVmapping][rawstrip];
    RstripNb = RstripNb + (127 - 2 * RstripNb) * invert;
    Int_t RstripPos = RstripNb + 128 * pos;

    if (fIsMC)
    {
        return rawstrip + 128 * pos;
    }

    return RstripPos;
}

double GEMModule::GetCommonMode(UInt_t isamp, Int_t flag, const mpdmap_t &apvinfo, UInt_t nhits)
{
    if (isamp > fN_MPD_TIME_SAMP)
        return 0;

    // unsigned int index = apvinfo.index;

    if (flag == 0)
    { // Sorting method: doesn't actually use the apv info:
        // int ngoodhits=0;
        std::vector<double> sortedADCs(nhits);

        if (nhits < fCommonModeNstripRejectLow + fCommonModeNstripRejectHigh + fCommonModeMinStripsInRange)
        {
            Error(Here("SBSGEMModule::GetCommonMode()"), "Sorting-method common-mode calculation requested with nhits %d less than minimum %d required", nhits, fCommonModeNstripRejectLow + fCommonModeNstripRejectHigh + fCommonModeMinStripsInRange);

            exit(-1);
        }

        for (int ihit = 0; ihit < nhits; ihit++)
        {
            int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

            sortedADCs[ihit] = fPedSubADC_APV[iraw];
        }

        std::sort(sortedADCs.begin(), sortedADCs.end());

        //   commonMode[isamp] = 0.0;
        double cm_temp = 0.0;
        int stripcount = 0;

        for (int k = fCommonModeNstripRejectLow; k < nhits - fCommonModeNstripRejectHigh; k++)
        {
            cm_temp += sortedADCs[k];
            stripcount++;
        }

        return cm_temp / double(stripcount);
    }
    else if (flag == 2)
    { // Histogramming method (experimental):

        int iAPV = apvinfo.pos;
        double cm_mean = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeMeanU[iAPV] : fCommonModeMeanV[iAPV];
        double cm_rms = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeRMSU[iAPV] : fCommonModeRMSV[iAPV];

        // for now these are unused. Comment out to suppress compiler warning.
        double DBmean = cm_mean;
        double DBrms = cm_rms;

        // Not sure if we SHOULD update cm_mean and cm_rms in this context because then the logic can become somewhat circular/self-referential:
        if (fMeasureCommonMode && fNeventsRollingAverage_by_APV[apvinfo.index] >= std::min(UInt_t(100), fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack))
        {
            cm_mean = fCommonModeRollingAverage_by_APV[apvinfo.index];
            cm_rms = std::max(0.2 * DBrms, std::min(5.0 * DBrms, fCommonModeRollingRMS_by_APV[apvinfo.index]));
        }

        cm_rms = DBrms;

        // bin width/stepsize = 8 with these settings:
        double stepsize = cm_rms * fCommonModeStepSize_Nsigma; // Default is 0.2 = rms/5
        double binwidth = cm_rms * fCommonModeBinWidth_Nsigma; // Default is +/- 2 sigma, bin width / step size = 20 with these settings

        // this will actually include all ADCs within +/- (ScanRange + BinWidth) sigma of the mean since range is bin center +/- 1*RMS.
        double scan_min = cm_mean - fCommonModeScanRange_Nsigma * cm_rms;
        double scan_max = cm_mean + fCommonModeScanRange_Nsigma * cm_rms;

        int nbins = int((scan_max - scan_min) / stepsize); // 20 * RMS / (RMS/4) = 80

        // std::cout << "histogramming cm: (scan min, scan max, step size, bin width, nbins)=("
        // 	      << scan_min << ", " << scan_max << ", " << stepsize << ", " << binwidth << ", " << nbins << ")" << std::endl;

        // NOTE: The largest number of bins that could contain any given sample is binwidth/stepsize = 20 with default settings:

        if (stepsize == 0)
            return GetCommonMode(isamp, 0, apvinfo);

        if (stepsize == 0)
            std::cout << "SBSGEMModule::GetCommonMode() ERROR Histogramming has zeros" << std::endl;
        // Construct std::vectors and explicitly zero-initialize them:
        std::vector<int> bincounts(nbins, 0);
        std::vector<double> binADCsum(nbins, 0.0);
        std::vector<double> binADCsum2(nbins, 0.0);

        int ibinmax = -1;
        int maxcounts = 0;
        // Now loop on all the strips and fill the histogram:
        // for( int ihit=0; ihit<fN_APV25_CHAN; ihit++ ){
        for (int ihit = 0; ihit < nhits; ihit++)
        {
            double ADC = fPedSubADC_APV[isamp + fN_MPD_TIME_SAMP * ihit];
            // calculate the lowest bin containing this ADC value.
            int nearestbin = std::max(0, std::min(nbins - 1, int(round((ADC - scan_min) / stepsize))));

            int binlow = nearestbin;
            int binhigh = nearestbin + 1;

            while (binlow >= 0 && fabs(ADC - (scan_min + binlow * stepsize)) <= binwidth)
            {
                bincounts[binlow]++;
                binADCsum[binlow] += ADC;
                binADCsum2[binlow] += pow(ADC, 2);

                if (ibinmax < 0 || bincounts[binlow] > maxcounts)
                {
                    ibinmax = binlow;
                    maxcounts = bincounts[binlow];
                }
                binlow--;
            }

            while (binhigh < nbins && fabs(ADC - (scan_min + binhigh * stepsize)) <= binwidth)
            {
                bincounts[binhigh]++;
                binADCsum[binhigh] += ADC;
                binADCsum2[binhigh] += pow(ADC, 2);
                if (ibinmax < 0 || bincounts[binhigh] > maxcounts)
                {
                    ibinmax = binhigh;
                    maxcounts = bincounts[binhigh];
                }
                binhigh++;
            }
        }

        // for( int ibin=0; ibin<nbins; ibin++ ){
        //   if( bincounts[ibin] > 0 ){
        // 	double binAVG = binADCsum[ibin]/double(bincounts[ibin]);
        // 	double binRMS = sqrt( binADCsum2[ibin]/double(bincounts[ibin])-pow(binAVG,2) );
        //   }
        // }

        if (ibinmax >= 0 && maxcounts >= fCommonModeMinStripsInRange)
        {
            return binADCsum[ibinmax] / double(bincounts[ibinmax]);
        }
        else
        { // Fall back on sorting method:
            return GetCommonMode(isamp, 0, apvinfo);
        }
    }
    else if (flag == 3)
    { // Online Danning method with cm min set to 0 used during GMn
        int iAPV = apvinfo.pos;
        double cm_mean = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeMeanU[iAPV] : fCommonModeMeanV[iAPV];
        double cm_rms = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeRMSU[iAPV] : fCommonModeRMSV[iAPV];

        double CM_1 = 0;
        double CM_2 = 0;
        int n_keep = 0;

        for (int ihit = 0; ihit < nhits; ihit++)
        {
            int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

            double ADCtemp = fPedSubADC_APV[iraw];

            if (ADCtemp > 0 && ADCtemp < cm_mean + 5 * cm_rms)
            {
                CM_1 += ADCtemp;
                n_keep++;
            }
        }

        CM_1 /= n_keep;
        n_keep = 0;

        for (int ihit = 0; ihit < nhits; ihit++)
        {
            int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

            double ADCtemp = fPedSubADC_APV[iraw];
            double rmstemp = (apvinfo.axis == SBSGEM::kUaxis) ? fPedRMSU[fStripAPV[iraw]] : fPedRMSV[fStripAPV[iraw]];

            if (ADCtemp > 0 && ADCtemp < CM_1 + 3 * rmstemp)
            {
                CM_2 += ADCtemp;
                n_keep++;
            }
        }

        return CM_2 / n_keep;
    }
    else if (flag == 4)
    { // Online Danning method for GEn
        int iAPV = apvinfo.pos;
        double cm_mean = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeMeanU[iAPV] : fCommonModeMeanV[iAPV];
        double cm_rms = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeRMSU[iAPV] : fCommonModeRMSV[iAPV];

        double cm_temp = 0.0;

        for (int iter = 0; iter < 3; iter++)
        {

            double cm_min = cm_mean - fCommonModeRange_nsigma * cm_rms;
            double cm_max = cm_mean + fCommonModeRange_nsigma * cm_rms;
            double sumADCinrange = 0.0;
            int n_keep = 0;

            for (int ihit = 0; ihit < nhits; ihit++)
            {
                int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

                double ADCtemp = fPedSubADC_APV[iraw];
                double rmstemp = (apvinfo.axis == SBSGEM::kUaxis) ? fPedRMSU[fStripAPV[iraw]] : fPedRMSV[fStripAPV[iraw]];

                if (iter != 0)
                {
                    cm_min = cm_temp - fCommonModeDanningMethod_NsigmaCut * 2.5 * rmstemp;
                    cm_max = cm_temp + fCommonModeDanningMethod_NsigmaCut * 2.5 * rmstemp;
                }

                if (ADCtemp >= cm_min && ADCtemp <= cm_max)
                {
                    n_keep++;
                    sumADCinrange += ADCtemp;
                }
            }

            cm_temp = sumADCinrange / n_keep;
        }

        return cm_temp;
    }
    else
    { //"offline" Danning method (default): requires apv info for cm-mean and cm-rms values:
        int iAPV = apvinfo.pos;
        double cm_mean = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeMeanU[iAPV] : fCommonModeMeanV[iAPV];
        double cm_rms = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeRMSU[iAPV] : fCommonModeRMSV[iAPV];

        // Not sure if we should update cm_mean and cm_rms in this context because then the logic can become somewhat circular/self-referential:
        if (fMeasureCommonMode && fNeventsRollingAverage_by_APV[apvinfo.index] >= std::min(UInt_t(100), fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack))
        {
            cm_mean = fCommonModeRollingAverage_by_APV[apvinfo.index];
            cm_rms = fCommonModeRollingRMS_by_APV[apvinfo.index];
        }

        // TODO: allow to use a different parameter than the one used for
        //  zero-suppression:
        double cm_min = cm_mean - fCommonModeDanningMethod_NsigmaCut * cm_rms;
        double cm_max = cm_mean + fCommonModeDanningMethod_NsigmaCut * cm_rms;

        double cm_temp = 0.0;

        for (int iter = 0; iter < fCommonModeNumIterations; iter++)
        {
            int nstripsinrange = 0;
            double sumADCinrange = 0.0;
            // double sum2ADCinrange=0.0;
            // for( int ihit=0; ihit<fN_APV25_CHAN; ihit++ ){
            for (int ihit = 0; ihit < nhits; ihit++)
            {
                int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

                double ADCtemp = fPedSubADC_APV[iraw];

                // on iterations after the first iteration, reject strips with signals above nsigma * pedrms:
                double rmstemp = (apvinfo.axis == SBSGEM::kUaxis) ? fPedRMSU[fStripAPV[iraw]] : fPedRMSV[fStripAPV[iraw]];

                // if(flag == 4){  //This is used for diagnostic plots where we "pretend" the data is zero suppressed //moving this to another method
                //   double strip_sum = 0;
                //   for(int itsamp=0; itsamp < fN_MPD_TIME_SAMP; itsamp++)
                //     strip_sum += ADCtemp - CM_online;
                //   if(strip_sum/fN_MPD_TIME_SAMP < 3*rmstemp) continue;
                // }

                double mintemp = cm_min;
                double maxtemp = cm_max;

                if (iter > 0)
                {
                    maxtemp = cm_temp + fCommonModeDanningMethod_NsigmaCut * rmstemp * fRMS_ConversionFactor; // 2.45 = sqrt(6), don't want to calculate sqrt every time
                    // mintemp = 0.0;
                    mintemp = cm_temp - fCommonModeDanningMethod_NsigmaCut * rmstemp * fRMS_ConversionFactor;
                }

                if (ADCtemp >= mintemp && ADCtemp <= maxtemp)
                {
                    nstripsinrange++;
                    sumADCinrange += ADCtemp;
                    // sum2ADCinrange += pow(ADCtemp,2);
                }
            }

            // double rmsinrange = cm_rms;

            // TO-DO: don't hard-code the minimum strip count.

            if (nstripsinrange >= fCommonModeMinStripsInRange)
            { // require minimum 10 strips in range:
                cm_temp = sumADCinrange / double(nstripsinrange);
                // rmsinrange = sqrt( sum2ADCinrange/double(nstripsinrange) - pow(commonMode[isamp],2) );
                // cm_max = commonMode[isamp] + fZeroSuppressRMS*std::min( rmsinrange, cm_rms );
            }
            else if (iter == 0)
            { // not enough strips on FIRST iteration, use mean from sorting-method:
                // std::cout << "Warning: fewer than ten strips in range on first iteration for common-mode calculation, crate, slot, MPD, ADC, cm_min, cm_max, n strips in range, cm_mean = "
                // 	  << apvinfo.crate << ", " << apvinfo.slot << ", " << apvinfo.mpd_id << ", " << apvinfo.adc_id << ", "
                // 	  << cm_min << ", " << cm_max << ", " << nstripsinrange << ", "
                // 	  << cm_mean << ", defaulting to sorting method" << std::endl;

                return GetCommonMode(isamp, 0, apvinfo);
            }

            // std::cout << "iteration " << iter << ": isamp, crate, slot, mpd, adc, cm_mean, cm_min, cm_max, nstrips in range, cm calc, cm sorting method = " << isamp << ", "
            // 		<< apvinfo.crate << ", " << apvinfo.slot << ", " << apvinfo.mpd_id << ", " << apvinfo.adc_id << ", "
            // 		<< cm_mean << ", " << cm_min << ", " << cm_max << ", " << nstripsinrange << ", " << cm_temp << ", "
            // 		<< GetCommonMode( isamp, 0, apvinfo ) << std::endl;
        } // loop over iterations for "Danning method" CM calculation

        // std::cout << std::endl;

        return cm_temp;
    }
}

// This routine attempts to calculate any necessary correction to the common-mode in a sample:
double GEMModule::GetCommonModeCorrection(UInt_t isamp, const mpdmap_t &apvinfo, UInt_t &ngoodhits, const UInt_t &nhits, bool fullreadout, Int_t flag)
{

    if (!fCorrectCommonMode)
    {
        return 0.0;
    }
    // In the case of full readout events, here we are trying to simulate what happens with online zero suppression to debug the correction (but not do anything else with the information since the "true" common-mode can be determined for these events):

    // This method should ONLY be called if all 128 channels are read out for a given event. In this case, we can reconstruct the hypothetical result of the online common-mode calculation, calculate our best estimate of the "true" common mode, and see how close any given correction algorithm comes to the "true" common-mode:

    // First, we will need to check the results of zero suppression:

    // Note: it is ASSUMED that we already know the "online" common-mode before we call this routine:

    if (isamp < 0 || isamp >= fN_MPD_TIME_SAMP)
        return 0.0;
    if (nhits < fCorrectCommonModeMinStrips)
        return 0.0;

    int ngood = 0;

    if (!fullreadout)
        ngoodhits = nhits;

    // double sumADCinrange=0.0;

    int iAPV = apvinfo.pos;
    double cm_mean = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeMeanU[iAPV] : fCommonModeMeanV[iAPV];
    double cm_rms = (apvinfo.axis == SBSGEM::kUaxis) ? fCommonModeRMSU[iAPV] : fCommonModeRMSV[iAPV];

    double DBrms = cm_rms;

    if (fMeasureCommonMode && fNeventsRollingAverage_by_APV[apvinfo.index] >= std::max(UInt_t(10), std::min(UInt_t(100), fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack)))
    {

        cm_mean = fCommonModeRollingAverage_by_APV[apvinfo.index];
        cm_rms = fCommonModeRollingRMS_by_APV[apvinfo.index];
    }

    // How much does the online common-mode differ from the expected one?
    double online_bias = cm_mean - fCM_online[isamp];

    std::vector<int> goodhits(nhits, 0);

    // std::cout << "counting number of good hits...";

    // a relevant question here is whether we should put an UPPER limit on the ADC value to attempt a correction?
    // It seems the CM calculations below will take care of imposing any relevant upper limits.

    for (int ihit = 0; ihit < nhits; ihit++)
    {
        int iraw = isamp + fN_MPD_TIME_SAMP * ihit;

        // Subtract the "online" version of the common-mode for this full readout event.
        // Check whether this event would have satisfied online zero suppression:

        bool isgood = true;
        // double ADCtemp = fPedSubADC_APV[iraw];
        if (fullreadout)
        { // then we actually need to sum all samples on this strip to simulate the online zero-suppression:
            // ADCtemp -= fCM_online[isamp];
            double ADCsumtemp = 0.0;
            for (int jsamp = 0; jsamp < fN_MPD_TIME_SAMP; jsamp++)
            {
                ADCsumtemp += fPedSubADC_APV[jsamp + fN_MPD_TIME_SAMP * ihit] - fCM_online[jsamp];
            }

            double RMS = (apvinfo.axis == SBSGEM::kUaxis) ? fPedRMSU[fStripAPV[iraw]] : fPedRMSV[fStripAPV[iraw]];

            // would this strip have passed online zero suppression?
            isgood = (ADCsumtemp >= fCommonModeDanningMethod_NsigmaCut * RMS * double(fN_MPD_TIME_SAMP));
        }

        if (isgood)
        {
            goodhits[ngood] = ihit;
            ngood++;
            // sumADCinrange += fPedSubADC_APV[iraw];
        }
    }
    // std::cout << "done, ngoodhits = " << ngood << std::endl;

    ngoodhits = ngood;

    // Now when we talk about calculating the CORRECTION to the online common-mode, we can still use the Danning, sorting, or histogramming methods
    // We must ask whether it is possible to calculate a correction using the chosen method:

    // In the latest version of the code,
    //  rawADC = whatever the DAQ reported out
    //  pedsubADC = raw ADC - pedestal = same as raw ADC in most circumstances
    //  if CM_ENABLED (i.e., NOT "fullreadout") then ONLINE common-mode has already been subtracted
    //

    double CMcorrection = 0.0;

    // std::cout << "(ngoodhits, flag)=(" << ngoodhits << ", " << flag << std::endl;

    if (ngoodhits >= fCorrectCommonModeMinStrips &&
        (online_bias > fCorrectCommonMode_Nsigma * cm_rms || flag == 0))
    {
        // Attempt to calculate a correction:
        if (fCommonModeFlag == 0)
        {
            // sorting: this method will be significantly biased if we use the same "low strip" rejection as for full-readout events
            if (ngoodhits >= fCommonModeNstripRejectLow + fCommonModeNstripRejectHigh + fCommonModeMinStripsInRange)
            {
                std::vector<double> sortedADCs(ngood);
                for (int ihit = 0; ihit < ngood; ihit++)
                {
                    int iraw = isamp + fN_MPD_TIME_SAMP * goodhits[ihit];
                    double ADCtemp = fPedSubADC_APV[iraw];
                    if (!fullreadout)
                        ADCtemp += fCM_online[isamp]; // Add back in online common-mode if it was subtracted online
                    sortedADCs[ihit] = ADCtemp;
                }

                double cm_temp = 0.0;
                int stripcount = 0;

                std::sort(sortedADCs.begin(), sortedADCs.end());

                for (int k = fCommonModeNstripRejectLow; k < ngoodhits - fCommonModeNstripRejectHigh; k++)
                {
                    cm_temp += sortedADCs[k];
                    stripcount++;
                }
                CMcorrection = fCM_online[isamp] - cm_temp / double(stripcount);
            }
        }
        else if (fCommonModeFlag == 1)
        {

            double cm_min = cm_mean - fCommonModeDanningMethod_NsigmaCut * cm_rms;
            double cm_max = cm_mean + fCommonModeDanningMethod_NsigmaCut * cm_rms;

            double cm_temp = 0.0;
            for (int iter = 0; iter < fCommonModeNumIterations; iter++)
            {

                int nstripsinrange = 0;

                double sumADCinrange = 0.0;

                for (int ihit = 0; ihit < ngood; ihit++)
                {
                    int iraw = isamp + fN_MPD_TIME_SAMP * goodhits[ihit];
                    double ADCtemp = fPedSubADC_APV[iraw];

                    if (!fullreadout)
                        ADCtemp += fCM_online[isamp];

                    double rmstemp = (apvinfo.axis == SBSGEM::kUaxis) ? fPedRMSU[fStripAPV[iraw]] : fPedRMSV[fStripAPV[iraw]];

                    double mintemp = cm_min;
                    double maxtemp = cm_max;

                    if (iter > 0)
                    {
                        maxtemp = cm_temp + fCommonModeDanningMethod_NsigmaCut * rmstemp * fRMS_ConversionFactor;
                        mintemp = cm_temp + fCommonModeDanningMethod_NsigmaCut * rmstemp * fRMS_ConversionFactor;
                    }

                    if (ADCtemp >= mintemp && ADCtemp >= maxtemp)
                    {
                        nstripsinrange++;
                        sumADCinrange += ADCtemp;
                    }
                }

                if (nstripsinrange >= fCommonModeMinStripsInRange)
                {
                    cm_temp = sumADCinrange / double(nstripsinrange);
                    ngoodhits = nstripsinrange;
                }
                else if (iter == 0)
                { // don't attempt correction, just return 0
                    CMcorrection = 0.0;
                }
            }

            CMcorrection = fCM_online[isamp] - cm_temp;
        }
        else if (fCommonModeFlag == 2)
        {

            // cm_rms = std::max(DBrms,std::min(5.*DBrms,cm_rms) );
            cm_rms = DBrms;

            double stepsize = cm_rms * fCommonModeStepSize_Nsigma;
            double binwidth = cm_rms * fCommonModeBinWidth_Nsigma;

            double scan_min = cm_mean - fCommonModeScanRange_Nsigma * cm_rms;
            double scan_max = cm_mean + fCommonModeScanRange_Nsigma * cm_rms;

            int nbins = int((scan_max - scan_min) / stepsize);

            if (stepsize == 0.)
                return 0.0;

            std::vector<int> bincounts(nbins, 0);
            std::vector<double> binADCsum(nbins, 0.0);

            int ibinmax = -1;
            int maxcounts = 0;

            for (int ihit = 0; ihit < ngood; ihit++)
            {
                int iraw = isamp + fN_MPD_TIME_SAMP * goodhits[ihit];
                double ADC = fPedSubADC_APV[iraw];

                if (!fullreadout)
                    ADC += fCM_online[isamp];

                // increment counts for any bin containing this hit:
                for (int ibin = 0; ibin < nbins; ibin++)
                {
                    if (fabs(ADC - (scan_min + ibin * stepsize)) <= binwidth)
                    {
                        bincounts[ibin]++;
                        binADCsum[ibin] += ADC;
                        if (bincounts[ibin] > maxcounts)
                        {
                            ibinmax = ibin;
                            maxcounts = bincounts[ibin];
                        }
                    }
                }
            }

            if (maxcounts >= fCommonModeMinStripsInRange && ibinmax >= 0)
            {
                CMcorrection = fCM_online[isamp] - binADCsum[ibinmax] / double(maxcounts);
                ngoodhits = maxcounts;
            }
        }
    }

    // if( flag == 0 ){ //add extra, "occupancy-based" correction per Sean method:

    // }

    return CMcorrection;
}

// This is a copy of the code for
void GEMModule::UpdateRollingAverage(int iapv, double value, std::vector<std::deque<Double_t>> &ResultContainer, std::vector<Double_t> &RollingAverage, std::vector<Double_t> &RollingRMS, std::vector<UInt_t> &EventCounter)
{

    UInt_t N, Nmax;

    N = EventCounter[iapv];
    Nmax = fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack;

    double sum, sum2;

    if (N < Nmax)
    {
        ResultContainer[iapv][N] = value;
        if (N == 0)
        { // first sample, initialize all sums/averages:
            RollingAverage[iapv] = value;
            RollingRMS[iapv] = 0.0;
            sum = value;
            sum2 = pow(value, 2);
        }
        else
        { // second and subsequent samples: increment sums, recalculate:
            double oldavg = RollingAverage[iapv];
            double oldrms = RollingRMS[iapv];

            sum = N * oldavg + value;
            sum2 = N * (pow(oldrms, 2) + pow(oldavg, 2)) + pow(value, 2);

            double newavg = sum / double(N + 1);
            double newrms = sqrt(sum2 / double(N + 1) - pow(newavg, 2));
            RollingAverage[iapv] = newavg;
            RollingRMS[iapv] = newrms;
        }
        EventCounter[iapv] = N + 1;
    }
    else
    { // we've reached the full size of the look-back window:
        double oldfirstsample = ResultContainer[iapv].front();
        // The net result of the following two operations should be to keep the container size the same:
        ResultContainer[iapv].pop_front();      // remove oldest sample
        ResultContainer[iapv].push_back(value); // insert newest sample

        double oldavg = RollingAverage[iapv];
        double oldrms = RollingRMS[iapv];

        double oldsum = Nmax * oldavg;
        double oldsum2 = Nmax * (pow(oldrms, 2) + pow(oldavg, 2));

        double lastsample = value;

        double newsum = oldsum - oldfirstsample + lastsample;
        double newsum2 = oldsum2 - pow(oldfirstsample, 2) + pow(lastsample, 2);

        double newavg = newsum / double(Nmax);
        double newrms = sqrt(newsum2 / double(Nmax) - pow(newavg, 2));

        RollingAverage[iapv] = newavg;
        RollingRMS[iapv] = newrms;
    }
}

void GEMModule::DefineAxes(Double_t rotation_angle)
{
    // Define detector orientation, assuming a tilt by rotation_angle (in rad)
    // around the y-axis

    fXax.SetXYZ(1.0, 0.0, 0.0);
    fXax.RotateY(rotation_angle);
    fYax.SetXYZ(0.0, 1.0, 0.0);
    fZax = fXax.Cross(fYax);
}

Int_t GEMModule::Begin(int run_number)
{
    // Does nothing
    // Here we can create some histograms that will be written to the ROOT file:
    // This is a natural place to do the hit maps/efficiency maps:
    fZeroSuppress = fZeroSuppress && !fPedestalMode;
    fCODA_BUILD_ALL_SAMPLES = 1; // r->GetDAQInfo("VTP_MPDRO_BUILD_ALL_SAMPLES") == "1";
    fCODA_CM_ENABLED = 0;        // r->GetDAQInfo("VTP_MPDRO_ENABLE_CM") == "1";

    // if (r->GetDAQInfo("VTP_MPDRO_ENABLE_CM").empty())
    // {
    //     fCODA_BUILD_ALL_SAMPLES = -1;
    //     fCODA_CM_ENABLED = -1;
    // }

    // pulled these lines out of the if-block below to avoid code duplication:
    TString appname = "Hall_C_tracker"; //(static_cast<THaDetector *>(GetParent()))->GetApparatus()->GetName();
    appname.ReplaceAll(".", "_");
    appname += "_";
    TString detname = "GEM_Moduel_for_Tracking"; // GetParent()->GetName();
    detname.Prepend(appname);
    detname.ReplaceAll(".", "_");
    detname += "_";
    detname += GetName();
    std::cout<<"Detector Name Check: "<<detname.Data()<<std::endl;

    if (fMakeEventInfoPlots && !fEventInfoPlotsInitialized)
    {
        fEventInfoPlotsInitialized = true;

        hMPD_EventCount_Alignment = new TH1D(TString::Format("h%s_MPD_EvCntAlign", detname.Data()), TString::Format("MPD event count diffs, module %s; MPD event count offsets", detname.Data()), 201, -100.5, 100.5);

        // We should really get the range for the fibers histogram from the MPDmap but we are lazy:

        hMPD_EventCount_Alignment_vs_Fiber = new TH2D(TString::Format("h%s_MPD_EvCntAlign_vs_fiber", detname.Data()), " fiber  MPD event count diff",
                                                      33, -0.5, 32.5, 201, -100.5, 100.5);

        hMPD_FineTimeStamp_vs_Fiber = new TH2D(TString::Format("h%s_MPD_FineTimeStamp_vs_fiber", detname.Data()), " fiber  MPD fine time stamp (ns)",
                                               33, -0.5, 32.5, 256, -0.5, 1023.5);
    }

    if (fMakeEfficiencyPlots && !fEfficiencyInitialized)
    {
        fEfficiencyInitialized = true;

        int nbinsx1D = int(round(1.02 * GetXSize() / fBinSize_efficiency1D));
        int nbinsy1D = int(round(1.02 * GetYSize() / fBinSize_efficiency1D));
        int nbinsx2D = int(round(1.02 * GetXSize() / fBinSize_efficiency2D));
        int nbinsy2D = int(round(1.02 * GetYSize() / fBinSize_efficiency2D));

        fhdidhitx = new TH1D(TString::Format("hdidhitx_%s", detname.Data()), "Local x of hits on good tracks  x (m)", nbinsx1D, -0.51 * GetXSize(), 0.51 * GetXSize());
        fhdidhity = new TH1D(TString::Format("hdidhity_%s", detname.Data()), "Local y of hits on good tracks  y (m)", nbinsy1D, -0.51 * GetYSize(), 0.51 * GetYSize());
        fhdidhitxy = new TH2D(TString::Format("hdidhitxy_%s", detname.Data()), "x vs y of hits on good tracks y (m) x (m)",
                              nbinsy2D, -0.51 * GetYSize(), 0.51 * GetYSize(),
                              nbinsx2D, -0.51 * GetXSize(), 0.51 * GetXSize());

        fhshouldhitx = new TH1D(TString::Format("hshouldhitx_%s", detname.Data()), "x of good track passing through (m) x (m)", nbinsx1D, -0.51 * GetXSize(), 0.51 * GetXSize());
        fhshouldhity = new TH1D(TString::Format("hshouldhity_%s", detname.Data()), "y of good track passing through (m) y (m)", nbinsy1D, -0.51 * GetYSize(), 0.51 * GetYSize());
        fhshouldhitxy = new TH2D(TString::Format("hshouldhitxy_%s", detname.Data()), "x vs y of good track passing through (m) y(m) x(m)",
                                 nbinsy2D, -0.51 * GetYSize(), 0.51 * GetYSize(),
                                 nbinsx2D, -0.51 * GetXSize(), 0.51 * GetXSize());
        fhrawhitxy = new TH2D(TString::Format("hrawhitxy_%s", detname.Data()), "x vs y raw hits y(m) x(m)",
                                 nbinsy2D, -0.51 * GetYSize(), 0.51 * GetYSize(),
                                 nbinsx2D, -0.51 * GetXSize(), 0.51 * GetXSize());

        // We want to plot number of clusters/event/time;
        //  Let's use area units of cm^2, and time units of milliseconds; so that one cluster/cm^2/ms = 1,000 clusters/cm^2/s
        //  Since "expected" rates are in the range of up to 500 kHz/cm^2/s, we might set the limits of the histogram to something like 0-500 (kHz/cm^2)
        hClusterBasedOccupancyUstrips = new TH1D(TString::Format("hClusterBasedOccupancy_%s_U", detname.Data()), TString::Format("U strips Hit rate (kHz/cm^2) "), 250, 0.0, 500.0);
        hClusterBasedOccupancyVstrips = new TH1D(TString::Format("hClusterBasedOccupancy_%s_V", detname.Data()), TString::Format("V strips Hit rate (kHz/cm^2) "), 250, 0.0, 500.0);

        hClusterMultiplicityUstrips = new TH1D(TString::Format("hClusterMultiplicity_%s_U", detname.Data()), TString::Format("U/X strips Total number of clusters per event"), 501, -0.5, 500.5);
        hClusterMultiplicityVstrips = new TH1D(TString::Format("hClusterMultiplicity_%s_V", detname.Data()), TString::Format("V/Y strips Total number of clusters per event"), 501, -0.5, 500.5);

        fEfficiencyInitialized = true;
    }

    if ((fPedestalMode || fMakeCommonModePlots) && !fPedHistosInitialized)
    { // make pedestal histograms:

        // Procedure:
        //  1. Analyze pedestal data with no pedestals supplied from the database.
        //  2. Extract "coarse" strip offsets from raw ADC histograms (i.e., common-mode means)
        //  3. Extract "fine" strip offsets from common-mode-subtracted histograms
        //  3. Add "coarse" strip offsets (common-mode means) back into "common-mode subtracted" ADC

        // U strips:
        hpedrmsU_distribution = new TH1D(TString::Format("hpedrmsU_distribution_%s", detname.Data()), "Pedestal RMS distribution, U strips; Ped. RMS", 200, 0, 100);
        hpedmeanU_distribution = new TH1D(TString::Format("hpedmeanU_distribution_%s", detname.Data()), "Pedestal mean distribution, U strips; Ped. mean", 250, -250, 250);
        hpedrmsU_by_strip = new TH1D(TString::Format("hpedrmsU_by_strip_%s", detname.Data()), "Pedestal rms U by strip; strip; ped. RMS", fNstripsU, -0.5, fNstripsU - 0.5);
        hpedmeanU_by_strip = new TH1D(TString::Format("hpedmeanU_by_strip_%s", detname.Data()), "Pedestal mean U by strip; strip; ped. mean", fNstripsU, -0.5, fNstripsU - 0.5);

        // V strips:
        hpedrmsV_distribution = new TH1D(TString::Format("hpedrmsV_distribution_%s", detname.Data()), "Pedestal RMS distribution, V strips; Ped. RMS", 200, 0, 100);
        hpedmeanV_distribution = new TH1D(TString::Format("hpedmeanV_distribution_%s", detname.Data()), "Pedestal mean distribution, V strips; Ped. mean", 250, -250, 250);
        hpedrmsV_by_strip = new TH1D(TString::Format("hpedrmsV_by_strip_%s", detname.Data()), "Pedestal rms V by strip; strip; Ped. RMS", fNstripsV, -0.5, fNstripsV - 0.5);
        hpedmeanV_by_strip = new TH1D(TString::Format("hpedmeanV_by_strip_%s", detname.Data()), "Pedestal mean V by strip; strip; Ped. mean", fNstripsV, -0.5, fNstripsV - 0.5);

        if (!fPedestalMode)
        { // fill the above histograms with the pedestal info loaded from the database:
            for (UInt_t istrip = 0; istrip < fNstripsU; istrip++)
            {
                hpedmeanU_distribution->Fill(fPedestalU[istrip]);
                hpedrmsU_distribution->Fill(fPedRMSU[istrip]);
                hpedmeanU_by_strip->SetBinContent(istrip + 1, fPedestalU[istrip]);
                hpedrmsU_by_strip->SetBinContent(istrip + 1, fPedRMSU[istrip]);
            }

            for (UInt_t istrip = 0; istrip < fNstripsV; istrip++)
            {
                hpedmeanV_distribution->Fill(fPedestalV[istrip]);
                hpedrmsV_distribution->Fill(fPedRMSV[istrip]);
                hpedmeanV_by_strip->SetBinContent(istrip + 1, fPedestalV[istrip]);
                hpedrmsV_by_strip->SetBinContent(istrip + 1, fPedRMSV[istrip]);
            }
        }

        hrawADCs_by_stripU = new TH2D(TString::Format("hrawADCs_by_stripU_%s", detname.Data()), "Raw ADCs by U strip number, no corrections U/X strip Raw ADC",
                                      fNstripsU, -0.5, fNstripsU - 0.5,
                                      512, -0.5, 4095.5);
        hrawADCs_by_stripV = new TH2D(TString::Format("hrawADCs_by_stripV_%s", detname.Data()), "Raw ADCs by V strip number, no corrections V/Y strip Raw ADC",
                                      fNstripsV, -0.5, fNstripsV - 0.5,
                                      512, -0.5, 4095.5);

        hcommonmode_subtracted_ADCs_by_stripU = new TH2D(TString::Format("hpedestalU_%s", detname.Data()), "ADCs by U strip number, w/common mode correction, no ped. subtraction U/X strip ADC - Common-mode",
                                                         fNstripsU, -0.5, fNstripsU - 0.5,
                                                         500, -500.0, 500.0);
        hcommonmode_subtracted_ADCs_by_stripV = new TH2D(TString::Format("hpedestalV_%s", detname.Data()), "ADCs by V strip number, w/common mode correction, no ped. subtraction V/Y strip ADC - Common-mode",
                                                         fNstripsV, -0.5, fNstripsV - 0.5,
                                                         500, -500.0, 500.0);

        hpedestal_subtracted_ADCs_by_stripU = new TH2D(TString::Format("hADCpedsubU_%s", detname.Data()), "Pedestal and common-mode subtracted ADCs by U strip number U/X strip ADC - Common-mode - pedestal",
                                                       fNstripsU, -0.5, fNstripsU - 0.5,
                                                       500, -500., 4500.);
        hpedestal_subtracted_ADCs_by_stripV = new TH2D(TString::Format("hADCpedsubV_%s", detname.Data()), "Pedestal and common-mode subtracted ADCs by V strip number V/Y strip ADC - Common-mode - pedestal",
                                                       fNstripsV, -0.5, fNstripsV - 0.5,
                                                       500, -500., 4500.);

        hpedestal_subtracted_rawADCs_by_stripU = new TH2D(TString::Format("hrawADCpedsubU_%s", detname.Data()), "ADCs by U strip, ped-subtracted, no common-mode correction U/X strip ADC - pedestal",
                                                          fNstripsU, -0.5, fNstripsU - 0.5,
                                                          500, -500., 4500.);
        hpedestal_subtracted_rawADCs_by_stripV = new TH2D(TString::Format("hrawADCpedsubV_%s", detname.Data()), "ADCs by V strip, ped-subtracted, no common-mode correction V/Y strip ADC - pedestal",
                                                          fNstripsV, -0.5, fNstripsV - 0.5,
                                                          500, -500., 4500.);

        hpedestal_subtracted_rawADCsU = new TH1D(TString::Format("hrawADCpedsubU_allstrips_%s", detname.Data()), "distribution of ped-subtracted U strip ADCs w/o common-mode correction ADC - pedestal",
                                                 1250, -500., 4500.);
        hpedestal_subtracted_rawADCsV = new TH1D(TString::Format("hrawADCpedsubV_allstrips_%s", detname.Data()), "distribution of ped-subtracted V strip ADCs w/o common-mode correction ADC - pedestal",
                                                 1250, -500., 4500.);

        hpedestal_subtracted_ADCsU = new TH1D(TString::Format("hADCpedsubU_allstrips_%s", detname.Data()), "distribution of ped-subtracted U strip ADCs w/common-mode correction ADC - Common-mode - pedestal",
                                              1250, -500., 4500.);
        hpedestal_subtracted_ADCsV = new TH1D(TString::Format("hADCpedsubV_allstrips_%s", detname.Data()), "distribution of ped-subtracted V strip ADCs w/common-mode correction ADC - Common-mode - pedestal",
                                              1250, -500., 4500.);

        UInt_t nAPVs_U = fNstripsU / fN_APV25_CHAN;
        hcommonmode_mean_by_APV_U = new TH2D(TString::Format("hCommonModeMean_by_APV_U_%s", detname.Data()), "distribution of common-mode means for U strip pedestal data APV card Common-mode",
                                             nAPVs_U, -0.5, nAPVs_U - 0.5,
                                             1024, -0.5, 4095.5);
        UInt_t nAPVs_V = fNstripsV / fN_APV25_CHAN;
        hcommonmode_mean_by_APV_V = new TH2D(TString::Format("hCommonModeMean_by_APV_V_%s", detname.Data()), "distribution of common-mode means for V strip pedestal data APV card Common-mode",
                                             nAPVs_V, -0.5, nAPVs_V - 0.5,
                                             1024, -0.5, 4095.5);

        fPedHistosInitialized = true;

        // Uncomment these later if you want them:
        // hrawADCs_by_strip_sampleU = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );
        // hrawADCs_by_strip_sampleV = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );

        // hcommonmode_subtracted_ADCs_by_strip_sampleU = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );
        // hcommonmode_subtracted_ADCs_by_strip_sampleV = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );

        // hpedestal_subtracted_ADCs_by_strip_sampleU = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );
        // hpedestal_subtracted_ADCs_by_strip_sampleV = new TClonesArray( "TH2D", fN_MPD_TIME_SAMP );

        // for( int isamp = 0; isamp<fN_MPD_TIME_SAMP; isamp++ ){
        //   new( (*hrawADCs_by_strip_sampleU)[isamp] ) TH2D( Format( "hrawADCU_%s_sample%d", detname.Data(), isamp ), "Raw U ADCs by strip and sample",
        // 						       fNstripsU, -0.5, fNstripsU-0.5,
        // 						       1024, -0.5, 4095.5 );
        //   new( (*hrawADCs_by_strip_sampleV)[isamp] ) TH2D( Format( "hrawADCV_%s_sample%d", detname.Data(), isamp ), "Raw V ADCs by strip and sample",
        // 						       fNstripsV, -0.5, fNstripsV-0.5,
        // 						       1024, -0.5, 4095.5 );

        //   new( (*hcommonmode_subtracted_ADCs_by_strip_sampleU)[isamp] ) TH2D( Format( "hpedestalU_%s_sample%d", detname.Data(), isamp ), "Pedestals by strip and sample",
        // 									  fNstripsU, -0.5, fNstripsU-0.5,
        // 									  1500, -500.0, 1000.0 );
        //   new( (*hcommonmode_subtracted_ADCs_by_strip_sampleV)[isamp] ) TH2D( Format( "hpedestalV_%s_sample%d", detname.Data(), isamp ), "Pedestals by strip and sample",
        // 									  fNstripsV, -0.5, fNstripsV-0.5,
        // 									  1500, -500.0, 1000.0 );

        //   new( (*hpedestal_subtracted_ADCs_by_strip_sampleU)[isamp] ) TH2D( Format( "hADCpedsubU_%s_sample%d", detname.Data(), isamp ), "Pedestal-subtracted ADCs by strip and sample",
        // 									fNstripsU, -0.5, fNstripsU-0.5,
        // 									1000, -500.0, 500.0 );
        //   new( (*hpedestal_subtracted_ADCs_by_strip_sampleV)[isamp] ) TH2D( Format( "hADCpedsubV_%s_sample%d", detname.Data(), isamp ), "Pedestal-subtracted ADCs by strip and sample",
        // 									fNstripsV, -0.5, fNstripsV-0.5,
        // 									1000, -500.0, 500.0 );

        // }
    }

    // std::cout << "fCommonModePlotsInitialized = " << fCommonModePlotsInitialized << std::endl;
    if (fMakeCommonModePlots && !fCommonModePlotsInitialized)
    {

        // std::cout << "SBSGEMModule::Begin: making common-mode histograms for module " << GetName() << std::endl;
        // U strips:

        fCommonModeDistU = new TH2D(TString::Format("hcommonmodeU_%s", detname.Data()), "U/X strips common-mode (user) APV card Common-mode - Common-mode mean (user)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 500, -200.0, 200.0);

        fCommonModeDistU_Histo = new TH2D(TString::Format("hcommonmodeU_histo_%s", detname.Data()), "U/X strips common-mode (user) APV card Common-mode - Common-mode mean (Histogram)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 500, -200.0, 200.0);

        fCommonModeDistU_Sorting = new TH2D(TString::Format("hcommonmodeU_sorting_%s", detname.Data()), "U/X strips common-mode (Sorting) APV card Common-mode (Sorting) - Common-mode mean", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 500, -200.0, 200.0);

        fCommonModeDistU_Danning = new TH2D(TString::Format("hcommonmodeU_danning_%s", detname.Data()), "U/X strips common-mode (Danning) APV card Common-mode (Danning) - Common-mode mean", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 500, -200.0, 200.0);

        fCommonModeDiffU = new TH2D(TString::Format("hcommonmodeU_diff_%s", detname.Data()), "U/X strips (all events) APV card Common-mode (User) - Common-mode (Danning online)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 200, -100.0, 100.0);

        fCommonModeDiffU_Uncorrected = new TH2D(TString::Format("hcommonmodeU_diff_uncorrected_%s", detname.Data()), "U/X strips (uncorrected events only) APV card CM (User) - CM (Danning online)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 200, -100.0, 100.0);

        fCommonModeCorrectionU = new TH2D(TString::Format("hcommonmodeU_corr_%s", detname.Data()), "U/X strips APV card applied CM correction", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 200, -100.0, 100.0);

        // fCommonModeNotCorrectionU = new TH2D( TString::Format( "hcommonmodeU_notcorr_%s", detname.Data() ), "U/X strips; APV card; CM Sorting - CM Danning without Correction", fNAPVs_U, -0.5, fNAPVs_U-0.5, 250, -100.0, 100.0 );
        fCommonModeResidualBiasU = new TH2D(TString::Format("hcommonmodeU_bias_%s", detname.Data()), "U/X strips APV card residual bias (corr. - true)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 200, -100, 100);

        fCommonModeResidualBiasU_corrected = new TH2D(TString::Format("hcommonmodeU_bias_corrected_%s", detname.Data()), "U/X strips APV card residual bias (corr. - true)", fNAPVs_U, -0.5, fNAPVs_U - 0.5, 200, -100, 100);

        fCommonModeResidualBias_vs_OccupancyU = new TH2D(TString::Format("hcommonmodeU_bias_vs_occupancy_%s", detname.Data()), "U/X strips APV in-range occupancy residual bias (corr. - true)", 100, 0.0, 1.0, 200, -100, 100);

        // V strips:
        fCommonModeDistV = new TH2D(TString::Format("hcommonmodeV_%s", detname.Data()), "V/Y strips common-mode (user) APV card Common-mode - Common-mode mean (user)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 500, -200.0, 200.0);

        fCommonModeDistV_Histo = new TH2D(TString::Format("hcommonmodeV_histo_%s", detname.Data()), "V/Y strips common-mode (user) APV card Common-mode - Common-mode mean (Histogram)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 500, -200.0, 200.0);

        fCommonModeDistV_Sorting = new TH2D(TString::Format("hcommonmodeV_sorting_%s", detname.Data()), "V/Y strips common-mode (Sorting) APV card Common-mode (Sorting) - Common-mode mean", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 500, -200.0, 200.0);

        fCommonModeDistV_Danning = new TH2D(TString::Format("hcommonmodeV_danning_%s", detname.Data()), "V/Y strips common-mode (Danning) APV card Common-mode (Danning) - Common-mode mean", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 500, -200.0, 200.0);

        fCommonModeDiffV = new TH2D(TString::Format("hcommonmodeV_diff_%s", detname.Data()), "V/Y strips (all events) APV card Common-mode (User) - Common-mode (Danning online)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 200, -100.0, 100.0);

        fCommonModeDiffV_Uncorrected = new TH2D(TString::Format("hcommonmodeV_diff_uncorrected_%s", detname.Data()), "V/Y strips (uncorrected events only) APV card CM (User) - CM (Danning online)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 200, -100.0, 100.0);

        fCommonModeCorrectionV = new TH2D(TString::Format("hcommonmodeV_corr_%s", detname.Data()), "V/Y strips APV card applied CM correction", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 200, -100.0, 100.0);

        fCommonModeResidualBiasV = new TH2D(TString::Format("hcommonmodeV_bias_%s", detname.Data()), "V/Y strips APV card residual bias (corr. - true)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 200, -100, 100);

        fCommonModeResidualBiasV_corrected = new TH2D(TString::Format("hcommonmodeV_bias_corrected_%s", detname.Data()), "V/Y strips APV card residual bias (corr. - true)", fNAPVs_V, -0.5, fNAPVs_V - 0.5, 200, -100, 100);

        fCommonModeResidualBias_vs_OccupancyV = new TH2D(TString::Format("hcommonmodeV_bias_vs_occupancy_%s", detname.Data()), "V/Y strips APV in-range occupancy residual bias (corr. - true)", 100, 0.0, 1.0, 200, -100, 100);

        // fCommonModeNotCorrectionV = new TH2D( TString::Format( "hcommonmodeV_notcorr_%s", detname.Data() ), "V/Y strips APV card CM Sorting - CM Danning without Correction", fNAPVs_V, -0.5, fNAPVs_V-0.5, 250, -100.0, 100.0 );

        // fCommonModeDistU->Print();
        // fCommonModeDistU_Sorting->Print();
        // fCommonModeDistU_Danning->Print();
        // fCommonModeDiffU->Print();

        // fCommonModeDistV->Print();
        // fCommonModeDistV_Sorting->Print();
        // fCommonModeDistV_Danning->Print();
        // fCommonModeDiffV->Print();
        fCommonModePlotsInitialized = true;
    }

    // The following histograms we more or less always want:
    if (!fPulseShapeInitialized)
    {
        hADCfrac_vs_timesample_allstrips = new TH2D(TString::Format("h%s_ADCfrac_vs_timesample_all", detname.Data()), "All strips Time sample ADCi/ADCsum", fN_MPD_TIME_SAMP, -0.5, fN_MPD_TIME_SAMP - 0.5, 200, 0.0, 1.0);
        hADCfrac_vs_timesample_goodstrips = new TH2D(TString::Format("h%s_ADCfrac_vs_timesample_good", detname.Data()), "Strips on good tracks Time sample ADCi/ADCsum", fN_MPD_TIME_SAMP, -0.5, fN_MPD_TIME_SAMP - 0.5, 200, 0.0, 1.0);

        hADCfrac_vs_timesample_maxstrip = new TH2D(TString::Format("h%s_ADCfrac_vs_timesample_max", detname.Data()), "Max strip in cluster on track Time sample ADCi/ADCsum", fN_MPD_TIME_SAMP, -0.5, fN_MPD_TIME_SAMP - 0.5, 200, 0.0, 1.0);

        fPulseShapeInitialized = true;
    }

    //  std::cout << "fCommonModePlotsInitialized = " << fCommonModePlotsInitialized << std::endl;

    // if( !fStripTimeFunc ){
    fStripTimeFunc = new TF1(TString::Format("StripPulseShape_%s", detname.Data()),
                             "std::max([3],[3]+[0]*exp(1.0)*(x-[1])/[2]*exp(-(x-[1])/[2]))", 0., fN_MPD_TIME_SAMP * fSamplePeriod);
    //  }

    return 0;
}

Int_t GEMModule::End(int run_number)
{
    TFile *_file_module = new TFile(Form("GEM_Layer_%d_module_%d.root", fLayer, fModule), "recreate");
    // Calculates efficiencies and writes hit maps and efficiency histograms and/or pedestal info to ROOT file:
    if (fMakeEventInfoPlots && fEventInfoPlotsInitialized)
    {
        hMPD_EventCount_Alignment->Write(0, TObject::kOverwrite);
        hMPD_EventCount_Alignment_vs_Fiber->Write(0, TObject::kOverwrite);
        hMPD_FineTimeStamp_vs_Fiber->Write(0, TObject::kOverwrite);
    }

    if (fMakeEfficiencyPlots)
    {
        // Create the track-based efficiency histograms at the end of the run:
        TString appname = "HC_Tracker";
        appname.ReplaceAll(".", "_");
        appname += "_";
        TString detname = GetName();
        detname.Prepend(appname);
        detname.ReplaceAll(".", "_");
        detname += "_";
        detname += GetName();
        //std::cout<<"xb debug: :::"<<GetName()<<std::endl;

        if (fhdidhitx != NULL && fhshouldhitx != NULL)
        { // Create efficiency histograms and write to the ROOT file:
            TH1D *hefficiency_vs_x = new TH1D(*fhdidhitx);
            hefficiency_vs_x->SetName(TString::Format("hefficiency_vs_x_%s", detname.Data()));
            hefficiency_vs_x->SetTitle(TString::Format("Track-based efficiency vs x, module %s x(m) Efficiency", detname.Data()));
            hefficiency_vs_x->Divide(fhshouldhitx);
            hefficiency_vs_x->Write(0, TObject::kOverwrite);
            hefficiency_vs_x->Delete();
        }

        if (fhdidhity != NULL && fhshouldhity != NULL)
        { // Create efficiency histograms and write to the ROOT file:
            TH1D *hefficiency_vs_y = new TH1D(*fhdidhity);
            hefficiency_vs_y->SetName(TString::Format("hefficiency_vs_y_%s", detname.Data()));
            hefficiency_vs_y->SetTitle(TString::Format("Track-based efficiency vs y, module %s y(m) Efficiency", detname.Data()));
            hefficiency_vs_y->Divide(fhshouldhity);
            hefficiency_vs_y->Write(0, TObject::kOverwrite);
            hefficiency_vs_y->Delete();
        }

        if (fhdidhitxy != NULL && fhshouldhitxy != NULL)
        { // Create efficiency histograms and write to the ROOT file:
            TH2D *hefficiency_vs_xy = new TH2D(*fhdidhitxy);
            hefficiency_vs_xy->SetName(TString::Format("hefficiency_vs_xy_%s", detname.Data()));
            hefficiency_vs_xy->SetTitle(TString::Format("Track-based efficiency vs x and y, module %s y(m) x(m)", detname.Data()));
            hefficiency_vs_xy->Divide(fhshouldhitxy);
            hefficiency_vs_xy->Write(0, TObject::kOverwrite);
            hefficiency_vs_xy->Delete();
        }

        if (fhdidhitx != NULL)
            fhdidhitx->Write(fhdidhitx->GetName(), TObject::kOverwrite);
        if (fhdidhity != NULL)
            fhdidhity->Write(fhdidhity->GetName(), TObject::kOverwrite);
        if (fhdidhitxy != NULL)
            fhdidhitxy->Write(fhdidhitxy->GetName(), TObject::kOverwrite);

        if (fhshouldhitx != NULL)
            fhshouldhitx->Write(fhshouldhitx->GetName(), TObject::kOverwrite);
        if (fhshouldhity != NULL)
            fhshouldhity->Write(fhshouldhity->GetName(), TObject::kOverwrite);
        if (fhshouldhitxy != NULL)
            fhshouldhitxy->Write(fhshouldhitxy->GetName(), TObject::kOverwrite);
        if (fhrawhitxy != NULL)
            fhrawhitxy->Write(fhrawhitxy->GetName(), TObject::kOverwrite);

        if (hClusterBasedOccupancyUstrips != nullptr)
            hClusterBasedOccupancyUstrips->Write(hClusterBasedOccupancyUstrips->GetName(), TObject::kOverwrite);
        if (hClusterBasedOccupancyVstrips != nullptr)
            hClusterBasedOccupancyVstrips->Write(hClusterBasedOccupancyVstrips->GetName(), TObject::kOverwrite);
        if (hClusterMultiplicityUstrips != nullptr)
            hClusterMultiplicityUstrips->Write(hClusterMultiplicityUstrips->GetName(), TObject::kOverwrite);
        if (hClusterMultiplicityVstrips != nullptr)
            hClusterMultiplicityVstrips->Write(hClusterMultiplicityVstrips->GetName(), TObject::kOverwrite);
    }

    if (fPedestalMode || fMakeCommonModePlots)
    { // write out pedestal histograms, print out pedestals in the format needed for both database and DAQ:

        // The channel map and pedestal information are specific to a GEM module.
        // But we want ONE database file and ONE DAQ file for the entire tracker.
        // So probably the best way to proceed is to have the
        hpedrmsU_distribution->Write(0, TObject::kOverwrite);
        hpedmeanU_distribution->Write(0, TObject::kOverwrite);
        hpedrmsV_distribution->Write(0, TObject::kOverwrite);
        hpedmeanV_distribution->Write(0, TObject::kOverwrite);

        hpedrmsU_by_strip->Write(0, TObject::kOverwrite);
        hpedmeanU_by_strip->Write(0, TObject::kOverwrite);
        hpedrmsV_by_strip->Write(0, TObject::kOverwrite);
        hpedmeanV_by_strip->Write(0, TObject::kOverwrite);

        hrawADCs_by_stripU->Write(0, TObject::kOverwrite);
        hrawADCs_by_stripV->Write(0, TObject::kOverwrite);
        hcommonmode_subtracted_ADCs_by_stripU->Write(0, TObject::kOverwrite);
        hcommonmode_subtracted_ADCs_by_stripV->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_ADCs_by_stripU->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_ADCs_by_stripV->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_rawADCs_by_stripU->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_rawADCs_by_stripV->Write(0, TObject::kOverwrite);

        hpedestal_subtracted_rawADCsU->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_rawADCsV->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_ADCsU->Write(0, TObject::kOverwrite);
        hpedestal_subtracted_ADCsV->Write(0, TObject::kOverwrite);

        hcommonmode_mean_by_APV_U->Write(0, TObject::kOverwrite);
        hcommonmode_mean_by_APV_V->Write(0, TObject::kOverwrite);

        // hrawADCs_by_strip_sampleU->Write();
        // hrawADCs_by_strip_sampleV->Write();
        // hcommonmode_subtracted_ADCs_by_strip_sampleU->Write();
        // hcommonmode_subtracted_ADCs_by_strip_sampleV->Write();
        // hpedestal_subtracted_ADCs_by_strip_sampleU->Write();
        // hpedestal_subtracted_ADCs_by_strip_sampleV->Write();
    }

    if (fMakeCommonModePlots)
    {
        fCommonModeDistU->Write(0, TObject::kOverwrite);
        fCommonModeDistU_Histo->Write(0, TObject::kOverwrite);
        fCommonModeDistU_Sorting->Write(0, TObject::kOverwrite);
        fCommonModeDistU_Danning->Write(0, TObject::kOverwrite);
        fCommonModeDiffU->Write(0, TObject::kOverwrite);
        fCommonModeDiffU_Uncorrected->Write(0, TObject::kOverwrite);
        fCommonModeCorrectionU->Write(0, TObject::kOverwrite);
        fCommonModeResidualBiasU->Write(0, TObject::kOverwrite);
        fCommonModeResidualBiasU_corrected->Write(0, TObject::kOverwrite);
        fCommonModeResidualBias_vs_OccupancyU->Write(0, TObject::kOverwrite);
        // fCommonModeNotCorrectionU->Write(0,TObject::kOverwrite);

        fCommonModeDistV->Write(0, TObject::kOverwrite);
        fCommonModeDistV_Histo->Write(0, TObject::kOverwrite);
        fCommonModeDistV_Sorting->Write(0, TObject::kOverwrite);
        fCommonModeDistV_Danning->Write(0, TObject::kOverwrite);
        fCommonModeDiffV->Write(0, TObject::kOverwrite);
        fCommonModeDiffV_Uncorrected->Write(0, TObject::kOverwrite);
        fCommonModeCorrectionV->Write(0, TObject::kOverwrite);
        fCommonModeResidualBiasV->Write(0, TObject::kOverwrite);
        fCommonModeResidualBiasV_corrected->Write(0, TObject::kOverwrite);
        fCommonModeResidualBias_vs_OccupancyV->Write(0, TObject::kOverwrite);
        // fCommonModeNotCorrectionV->Write(0,TObject::kOverwrite);
    }

    if (fPulseShapeInitialized)
    {
        hADCfrac_vs_timesample_allstrips->Write(0, TObject::kOverwrite);
        hADCfrac_vs_timesample_goodstrips->Write(0, TObject::kOverwrite);
        hADCfrac_vs_timesample_maxstrip->Write(0, TObject::kOverwrite);
    }

    // if( fStripTimeFunc )

    // delete fStripTimeFunc;

    _file_module -> Close();

    return 0;
}

void GEMModule::Decode(const EventWrapper &evdata)
{
    Clear();
    // std::cout << "GEMModule::Decode() to be implemented..." << std::endl;
    // std::cout << "[SBSGEMModule::Decode " << fName << "]" << std::endl;

    // initialize generic "strip" counter to zero:
    fNstrips_hit = 0;
    fNstrips_hit_neg = 0;
    fNstrips_hit_pos = 0;
    // initialize "U" and "V" strip counters to zero:
    fNstrips_hitU = 0;
    fNstrips_hitV = 0;
    fNstrips_hitU_neg = 0;
    fNstrips_hitV_neg = 0;

    // UInt_t MAXNSAMP_PER_APV = fN_APV25_CHAN * fN_MPD_TIME_SAMP;

    // std::cout << "MAXNSAMP_PER_APV = " << MAXNSAMP_PER_APV << std::endl;

    // we could save some time on these allocations by making these data members of SBSGEMModule: these are probably expensive:

    // to avoid rewriting the other code below, declare references to the fixed-size arrays:
    std::vector<UInt_t> &Strip = fStripAPV;
    std::vector<UInt_t> &rawStrip = fRawStripAPV;
    std::vector<Int_t> &rawADC = fRawADC_APV;
    std::vector<Double_t> &pedsubADC = fPedSubADC_APV; // ped-subtracted, not necessarily common-mode subtracted
    std::vector<Double_t> &commonModeSubtractedADC = fCommonModeSubtractedADC_APV;

    // Now, how are we going to implement any kind of common-mode "sag" correction?
    //  1. It has to be optional
    //  2. It will probably require some new optional user-adjustable database parameters to control the behavior.
    //  3. We will want to keep track of the mean of the "sorting-method" common-mode calculation for the full-readout events; specifically a
    //     rolling average over the previous, say, 100 full readout events? Then the error of this calculation will be the RMS over 10.
    //     we can make the size of the "look-back" window user-adjustable. For a trigger rate of 3 kHz, 10,000 /100 = 100 = ~3 s of data taking
    //  4. For online zero-suppressed events, we want to check if the calculated common-mode is less than some number of standard deviations below the common-mode rolling mean, and if there is a sufficient number of strips with raw ADC within +/- some number of standard deviations of the rolling common-mode mean, then we re-calculate the common-mode and correct the ADC values accordingly before we pass them to the cluster-finding routines.

    // Do we need to loop on all APV cards? maybe not,
    int apvcounter = 0;

    bool firstevcnt = true;
    UInt_t FirstEvCnt = 0;
    //for (auto &i : fMPDmap)
    //    std::cout << i;
    //getchar();


    for (std::vector<mpdmap_t>::iterator it = fMPDmap.begin(); it != fMPDmap.end(); ++it)
    {
        // loop over all decode map entries associated with this module (each decode map entry is one APV card)
        Int_t effChan = it->mpd_id << 4 | it->adc_id; // left-shift mpd id by 4 bits and take the bitwise OR with ADC_id to uniquely identify the APV card.
        // mpd_id is not necessarily equal to slot, but that seems to be the convention in many cases
        //  Find channel for this crate/slot

        // TO-DO: rewrite this "time stamp" part of the code more efficiently, by somehow pulling it outside the loop over APVs, since the time stamp is
        // really per-MPD, not per-APV. This is unnecessarily repeated analysis for up to 16 times per MPD.
        // Note that we don't use the information in the analysis in any way so far.
        // on the other hand, the decode map is organized by APV card, so it is difficult to imagine how we would restructure this part of the code
        // since otherwise we don't know which crate/slot to look in.
        // First get time stamp info:
        UInt_t nhits_timestamp_low = evdata.GetNumHits(it->crate, it->slot, fChan_TimeStamp_low);
        UInt_t nhits_timestamp_high = evdata.GetNumHits(it->crate, it->slot, fChan_TimeStamp_high);
        UInt_t nhits_event_count = evdata.GetNumHits(it->crate, it->slot, fChan_MPD_EventCount);

        // std::cout << "nhits_timestamp_low = " << nhits_timestamp_low << std::endl;

        if (nhits_timestamp_low > 0 && nhits_timestamp_high == nhits_timestamp_low && nhits_event_count == nhits_timestamp_low)
        {
            for (unsigned int ihit = 0; ihit < nhits_timestamp_low; ihit++)
            {
                unsigned int fiber = evdata.GetRawData(it->crate, it->slot, fChan_TimeStamp_low, ihit);
                if (fiber == it->mpd_id)
                { // this is the channel we want:
                    UInt_t Tlow = evdata.GetData(it->crate, it->slot, fChan_TimeStamp_low, ihit);
                    UInt_t Thigh = evdata.GetData(it->crate, it->slot, fChan_TimeStamp_high, ihit);
                    UInt_t EvCnt = evdata.GetData(it->crate, it->slot, fChan_MPD_EventCount, ihit);

                    // std::cout << "Tlow, Thigh, EvCnt = " << Tlow << ", " << Thigh << ", " << EvCnt << std::endl;

                    if (firstevcnt)
                    {
                        FirstEvCnt = EvCnt;
                        firstevcnt = false;
                    }

                    fEventCount_by_APV[apvcounter] = EvCnt;

                    // Fine time stamp is in the first 8 bits of Tlow;
                    fTfine_by_APV[apvcounter] = Tlow & 0x000000FF;

                    if (fMakeEventInfoPlots && fEventInfoPlotsInitialized)
                    {
                        hMPD_EventCount_Alignment->Fill(EvCnt - FirstEvCnt);
                        hMPD_EventCount_Alignment_vs_Fiber->Fill(fiber, EvCnt - FirstEvCnt);

                        hMPD_FineTimeStamp_vs_Fiber->Fill(fiber, fTfine_by_APV[apvcounter] * 4.0);
                    }

                    Long64_t Tcoarse = Thigh << 16 | (Tlow << 8);
                    double Tc = double(Tcoarse);

                    if (EvCnt == 0)
                        fT0_by_APV[apvcounter] = Tc;

                    // T ref is the coarse time stamp of the reference APV (the first one, in this case)
                    if (apvcounter == 0)
                        fTref_coarse = Tc - fT0_by_APV[apvcounter];

                    // This SHOULD make fTcoarse_by_APV the Tcoarse RELATIVE to the
                    //  "reference" APV
                    fTcoarse_by_APV[apvcounter] = Tc - fT0_by_APV[apvcounter] - fTref_coarse;

                    // We probably don't want to hard-code 24 ns and 4 ns here for the units of
                    // Tcoarse and Tfine, but this should be fine for initial checkout of decoding:
                    fTimeStamp_ns_by_APV[apvcounter] = 24.0 * fTcoarse_by_APV[apvcounter] + 4.0 * (fTfine_by_APV[apvcounter] % 6);

                    // std::cout << "fiber, apvcounter, EvCnt, Tcoarse, Tfine, time stamp ns = " << fiber << ", " <<  apvcounter << ", "
                    // 	    << fEventCount_by_APV[apvcounter] << ", "
                    // 	    << Tcoarse << ", " << fTfine_by_APV[apvcounter] << ", "
                    // 	    << fTimeStamp_ns_by_APV[apvcounter] << std::endl;

                    break;
                }
            }
        }

        // Get common-mode flags, if applicable:
        // Default to the values from the database (or the default values):

        // Bool_t CM_ENABLED = fCommonModeFlag != 0 && fCommonModeFlag != 1 && !fPedestalMode; // WTF???
        Bool_t CM_ENABLED = false; // regarding the above WTF???
        Bool_t BUILD_ALL_SAMPLES = !fOnlineZeroSuppression;
        Bool_t CM_OUT_OF_RANGE = false;

        // Initialize default values based on the run "DAQ info":
        if (fCODA_BUILD_ALL_SAMPLES != -1)
        {
            BUILD_ALL_SAMPLES = fCODA_BUILD_ALL_SAMPLES;
            fPedSubFlag = (fCODA_BUILD_ALL_SAMPLES == 0);
        }
        if (fCODA_CM_ENABLED != -1)
            CM_ENABLED = fCODA_CM_ENABLED;

        UInt_t cm_flags = 4 * CM_OUT_OF_RANGE + 2 * CM_ENABLED + BUILD_ALL_SAMPLES;
        UInt_t nhits_cm_flag = evdata.GetNumHits(it->crate, it->slot, fChan_CM_flags);

        // bool cm_flags_found = false;

        if (nhits_cm_flag > 0)
        {
            // If applicable, find the common-mode/zero-suppression settings loaded from the raw data for this APV:
            // In principle in the SSP/VTP event format, there should be exactly one "hit" per APV in this "channel":
            for (unsigned int ihit = 0; ihit < nhits_cm_flag; ihit++)
            {
                int chan_temp = evdata.GetRawData(it->crate, it->slot, fChan_CM_flags, ihit);
                if (chan_temp == effChan)
                { // assume that this is only filled once per MPD per event, and exit the loop when we find this MPD:
                    // std::cout << "Before decoding cm flags, CM_ENABLED, BUILD_ALL_SAMPLES = " << CM_ENABLED << ", "
                    // 	    << BUILD_ALL_SAMPLES << std::endl;
                    cm_flags = evdata.GetData(it->crate, it->slot, fChan_CM_flags, ihit);
                    // cm_flags_found = true;
                    break;
                }
            }
        }

        // The proper logic of common-mode calculation/subtraction and zero suppression is as follows:
        //  1. If CM_ENABLED is true, we never calculate the common-mode ourselves, it has already been subtracted from the data:
        //  2. If BUILD_ALL_SAMPLES is false, then online zero suppression is enabled. We can, in addition, apply our own higher thresholds if we want:
        //  3. If CM_ENABLED is true, the pedestal has also been subtracted, so we don't subtract it again.
        //  4. If CM_ENABLED is false, we need to subtract the pedestals (maybe) AND calculate and subtract the common-mode:
        //  5. If BUILD_ALL_SAMPLES is false then CM_ENABLED had better be true!
        //  6. If CM_OUT_OF_RANGE is true then BUILD_ALL_SAMPLES must be true and CM_ENABLED
        //     must be false!

        CM_OUT_OF_RANGE = cm_flags / 4;
        CM_ENABLED = cm_flags / 2;
        BUILD_ALL_SAMPLES = cm_flags % 2;

        // if( cm_flags_found ){
        // std::cout << "cm flag defaults overridden by raw data, effChan = " << effChan << ", CM_ENABLED = " << CM_ENABLED << ", BUILD_ALL_SAMPLES = " << BUILD_ALL_SAMPLES << std::endl;
        // }

        // cout<<BUILD_ALL_SAMPLES<<" "<<CM_ENABLED<<endl;
        // fOnlineZeroSuppression = !BUILD_ALL_SAMPLES;
        if (!BUILD_ALL_SAMPLES && !CM_ENABLED)
        { // This should never happen: skip this APV card
            continue;
        }
        if (CM_OUT_OF_RANGE && !BUILD_ALL_SAMPLES)
        { // this should also never happen: skip this APV card:
            continue;
        }

        if (CM_OUT_OF_RANGE && CM_ENABLED)
        { // force CM_ENABLED to false if CM_OUT_OF_RANGE is true;
            // This should have already been done online:
            CM_ENABLED = false;
        }

        // Int_t nchan = evdata.GetNumChan( it->crate, it->slot ); //this could be made faster

        SBSGEM::GEMaxis_t axis = it->axis == 0 ? SBSGEM::kUaxis : SBSGEM::kVaxis;

        // printf("nchan = %d\n", nchan );

        // std::cout << "crate, slot, nchan = " << it->crate << ", " << it->slot << ", " << nchan << std::endl;

        // this is looping on all the
        // for( Int_t ichan = 0; ichan < nchan; ++ichan ) { //this is looping over all the "channels" (APV cards) in the crate and slot containing this decode map entry/APV card:
        // Int_t chan = evdata.GetNextChan( it->crate, it->slot, ichan ); //"chan" here refers to one APV card
        // std::cout << it->crate << " " << it->slot << " mpd_id ??? " << it->mpd_id << " " << chan << " " << effChan << std::endl;

        // if( chan != effChan ) continue; //

        if (fIsMC)
        {
            CM_ENABLED = true;
            BUILD_ALL_SAMPLES = false;
        }

        // Let's see if we can actually decode the MPD debug headers:
        UInt_t nhits_MPD_debug = 0;

        // Let's store here in temporary arrays the calculated common-mode values decoded from the MPD debug headers for events with online zero suppression:

        UInt_t CMcalc[fN_MPD_TIME_SAMP];
        Int_t CMcalc_signed[fN_MPD_TIME_SAMP];

        if (CM_ENABLED)
        { // try to decode MPD debug headers and see if the results make any sense:
            nhits_MPD_debug = evdata.GetNumHits(it->crate, it->slot, fChan_MPD_Debug);

            if (nhits_MPD_debug > 0)
            { // we expect to get three words per APV card:
                UInt_t wcount = 0;

                UInt_t MPDdebugwords[3];

                for (unsigned int ihit = 0; ihit < nhits_MPD_debug; ihit++)
                {
                    UInt_t chan_temp = evdata.GetRawData(it->crate, it->slot, fChan_MPD_Debug, ihit);
                    UInt_t word_temp = evdata.GetData(it->crate, it->slot, fChan_MPD_Debug, ihit);
                    if (chan_temp == effChan && wcount < 3)
                    {
                        MPDdebugwords[wcount++] = word_temp;
                    }
                    if (wcount == 3)
                        break; // if we found all 3 MPD debug words for this channel, exit the loop
                }
                if (wcount == 3)
                { // Then let's decode the debug headers:

                    for (unsigned int iw = 0; iw < 3; iw++)
                    {
                        CMcalc[2 * iw] = (MPDdebugwords[iw] & 0xFFF) | ((MPDdebugwords[iw] & 0x1000) ? 0xFFFFF000 : 0x0);
                        CMcalc[2 * iw + 1] = ((MPDdebugwords[iw] >> 13) & 0xFFF) | (((MPDdebugwords[iw] >> 13) & 0x1000) ? 0xFFFFF000 : 0x0);
                        CMcalc_signed[2 * iw] = Int_t(CMcalc[2 * iw]);
                        CMcalc_signed[2 * iw + 1] = Int_t(CMcalc[2 * iw + 1]);

                        fCM_online[2 * iw] = double(CMcalc_signed[2 * iw]);
                        fCM_online[2 * iw + 1] = double(CMcalc_signed[2 * iw + 1]);
                    }

                    //
                    // for( int isamp=0; isamp<fN_MPD_TIME_SAMP; isamp++ ){
                    //   std::cout << "MPD debug words for APV in (crate,slot,effChan)=(" << it->crate << ", " << it->slot << ", " << effChan << "): time sample " << isamp
                    // 	      << ", online calculated common-mode = " << CMcalc_signed[isamp] << std::endl;
                    // }
                }
            }
        } // End check if CM_ENABLED

        Int_t nsamp = evdata.GetNumHits(it->crate, it->slot, effChan);
        //std::cout << __func__ << "<><><><><><>GEMModule::XB DEBUG::" << nsamp << std::endl;
        //std::cout<<"crate: "<<it->crate<<", slot: "<<it->slot<<", effchan: "<<effChan<<std::endl;
        //std::cout<<"mpd_id = "<<(effChan >> 4 )<<" adc: "<<(effChan &0xf)<<std::endl;
        //if(nsamp == 774) {
        //    evdata.print();
        //    getchar();
        //}

        if (nsamp > 0)
        {
            //      assert(nsamp%fN_MPD_TIME_SAMP==0); //this is making sure that the number of samples is equal to an integer multiple of the number of time samples per strip
            Int_t nstrips = nsamp / fN_MPD_TIME_SAMP; // number of strips fired on this APV card (should be exactly 128 if online zero suppression is NOT used):
            //if(nstrips != 128) {
            //    std::cout<<"nsamp: "<<nsamp<<", fN_MPD_TIME_SAMP: "<<(double)fN_MPD_TIME_SAMP<<std::endl;
            //    std::cout<<"nstrips: "<<nstrips<<std::endl;
            //    getchar();
            //}

            bool fullreadout = !CM_ENABLED && BUILD_ALL_SAMPLES && nstrips == fN_APV25_CHAN;
            // std::cout << "MPD ID, ADC channel, number of strips fired = " << it->mpd_id << ", "
            //           << it->adc_id << ", " << nstrips << std::endl;

            double commonMode[fN_MPD_TIME_SAMP];

            double CommonModeCorrection[fN_MPD_TIME_SAMP]; // possible correction to apply, initialize to zero:

            for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
            {
                commonMode[isamp] = 0.0;
                CommonModeCorrection[isamp] = 0.0;
            }

            // Let's throw in the towel and always do a first loop over the data, despite a small loss of efficiency,
            // to populate the local arrays; otherwise this code is getting too confusing and bug-prone:
            // First loop over the hits: populate strip, raw strip, raw ADC, ped sub ADC and common-mode-subtracted aDC:
            for (int iraw = 0; iraw < nsamp; iraw++)
            { 
                // NOTE: iraw = isamp + fN_MPD_TIME_SAMP * istrip
                int strip = evdata.GetRawData(it->crate, it->slot, effChan, iraw);
                UInt_t decoded_rawADC = evdata.GetData(it->crate, it->slot, effChan, iraw);

                int isamp = iraw % fN_MPD_TIME_SAMP;

                Int_t ADC = Int_t(decoded_rawADC);

                rawStrip[iraw] = strip;
                Strip[iraw] = GetStripNumber(strip, it->pos, it->invert);

                rawADC[iraw] = ADC;

                double ped = (axis == SBSGEM::kUaxis) ? fPedestalU[Strip[iraw]] : fPedestalV[Strip[iraw]];

                // If pedestal subtraction was done online, don't do it again:
                // In pedestal mode, the DAQ should NOT have subtracted the pedestals,
                // but even if it did, it shouldn't affect the pedestal analysis to first order whether we subtract the pedestals or not:
                if (fPedSubFlag != 0 && !fPedestalMode && !fIsMC)
                    ped = 0.0;
                if (CM_ENABLED && !fIsMC)
                {
                    ped = 0.0; // If this is true then the pedestal was DEFINITELY always calculated online:
                               // rawADC[iraw] += fCM_online[isamp] (not yet sure if we want to add back the online-CM) ;
                }

                pedsubADC[iraw] = double(ADC) - ped;
                commonModeSubtractedADC[iraw] = double(ADC) - ped;

                // the calculation of common mode in pedestal mode analysis differs from the
                //  offline or online zero suppression analysis; here we use a simple average of all 128 channels:
                if (fPedestalMode)
                {
                    // do simple common-mode calculation involving the simple average of all 128 (ped-subtracted) ADC
                    // values

                    if (fSubtractPedBeforeCommonMode)
                    {
                        commonMode[isamp] += pedsubADC[iraw] / double(fN_APV25_CHAN);
                    }
                    else
                    {
                        commonMode[isamp] += rawADC[iraw] / double(fN_APV25_CHAN);
                    }
                }
            }

            if (fullreadout)
            {
                // then we need to calculate the common-mode:
                // declare temporary array to hold common mode values for this APV card and, if necessary, calculate them:

                // std::cout << "Common-mode calculation: " << std::endl;

                if (fMakeCommonModePlots || !fPedestalMode)
                {
                    // calculate both ways:
                    // vector<double> CM_danning_online_temp(fN_MPD_TIME_SAMP,0);

                    for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                    {
                        // moved common-mode calculation to its own function:

                        // Now: a question is should we modify the behavior/do added checks if
                        // CM_OUT_OF_RANGE is set?

                        if (fMakeCommonModePlots)
                        {
                            // double cm_danning = GetCommonMode( isamp, 1, *it );
                            // experimental: Test histogramming method:
                            double cm_danning = GetCommonMode(isamp, 1, *it);
                            double cm_histo = GetCommonMode(isamp, 2, *it);
                            // if( !CM_OUT_OF_RANGE ) { // this is a hack so I only get debug printouts for good (full readout) events
                            // cm_histo = GetCommonMode( isamp, 2, *it );
                            //  }	else {
                            // cm_histo = cm_danning;
                            // }
                            double cm_sorting = GetCommonMode(isamp, 0, *it);
                            double cm_danning_online = GetCommonMode(isamp, fCommonModeOnlFlag, *it);
                            // double cm_danning_offline = GetCommonMode( isamp, 4, *it, cm_danning_online ); //Artificially zero suppress this calculation to check the CM correction algorithm

                            fCM_online[isamp] = cm_danning_online;

                            // If we are in this if-block, this is a full-readout event

                            // std::cout << "cm danning, sorting = " << cm_danning << ", " << cm_sorting << std::endl;

                            if (!fPedestalMode)
                            {
                                switch (fCommonModeFlag)
                                {
                                case 2:
                                    commonMode[isamp] = cm_histo;
                                    break;
                                case 1:
                                default:
                                    commonMode[isamp] = cm_danning;
                                    break;
                                case 0:
                                    commonMode[isamp] = cm_sorting;
                                    break;
                                }
                            }

                            double cm_mean;

                            UInt_t iAPV = it->pos;

                            // std::cout << "Filling common-mode histograms..." << std::endl;

                            // std::cout << "iAPV, nAPVsU, nAPVsV, axis = " << iAPV << ", " << fNAPVs_U << ", "
                            // 		<< fNAPVs_V << ", " << axis << std::endl;

                            if (!CM_OUT_OF_RANGE)
                            {
                                if (axis == SBSGEM::kUaxis)
                                {
                                    cm_mean = fCommonModeMeanU[iAPV];

                                    fCommonModeDistU->Fill(iAPV, commonMode[isamp] - cm_mean);
                                    fCommonModeDistU_Histo->Fill(iAPV, cm_histo - cm_mean);
                                    fCommonModeDistU_Sorting->Fill(iAPV, cm_sorting - cm_mean);
                                    fCommonModeDistU_Danning->Fill(iAPV, cm_danning - cm_mean);
                                    fCommonModeDiffU->Fill(iAPV, commonMode[isamp] - cm_danning_online);

                                    // Moving simulated common-mode corrections for full-readout events to a separate dedicated method
                                    // if(CommonModeCorrection[isamp] != 0.0) fCommonModeCorrectionU->Fill( iAPV, cm_sorting - (cm_danning_online - CommonModeCorrection[isamp]));
                                    // else fCommonModeNotCorrectionU->Fill( iAPV, cm_sorting - (cm_danning_online - CommonModeCorrection[isamp]));
                                }

                                else
                                {
                                    cm_mean = fCommonModeMeanV[iAPV];

                                    fCommonModeDistV->Fill(iAPV, commonMode[isamp] - cm_mean);
                                    fCommonModeDistV_Histo->Fill(iAPV, cm_histo - cm_mean);
                                    fCommonModeDistV_Sorting->Fill(iAPV, cm_sorting - cm_mean);
                                    fCommonModeDistV_Danning->Fill(iAPV, cm_danning - cm_mean);
                                    fCommonModeDiffV->Fill(iAPV, commonMode[isamp] - cm_danning_online);
                                    // Moving simulated common-mode corrections for full-readout events to a separate dedicated method
                                    // if(CommonModeCorrection[isamp] != 0.0) fCommonModeCorrectionV->Fill( iAPV, cm_sorting - (cm_danning_online - CommonModeCorrection[isamp]));
                                    // else fCommonModeNotCorrectionV->Fill( iAPV, cm_sorting - (cm_danning_online - CommonModeCorrection[isamp]));
                                }
                            }
                            // std::cout << "Done..." << std::endl;
                        }
                        else if (!fPedestalMode)
                        { // if not doing diagnostic plots, just calculate whichever way the user wanted:
                            commonMode[isamp] = GetCommonMode(isamp, fCommonModeFlag, *it);

                            if (fCorrectCommonMode)
                            {
                                // always calculate online CM if doing corrections:
                                fCM_online[isamp] = GetCommonMode(isamp, fCommonModeOnlFlag, *it);
                            }
                        }
                        // std::cout << "effChan, isamp, Common-mode = " << effChan << ", " << isamp << ", " << commonMode[isamp] << std::endl;

                        // Now handle rolling average common-mode calculation:

                        // UpdateRollingCommonModeAverage(apvcounter,commonMode[isamp]);

                        if (!CM_OUT_OF_RANGE)
                        {
                            UpdateRollingAverage(apvcounter, commonMode[isamp],
                                                 fCommonModeResultContainer_by_APV,
                                                 fCommonModeRollingAverage_by_APV,
                                                 fCommonModeRollingRMS_by_APV,
                                                 fNeventsRollingAverage_by_APV);
                        }
                    } // loop over time samples

                    if (fCorrectCommonMode)
                    { // For full readout events we are mainly interested in monitoring the "bias" of the ONLINE calculation,
                        // NOT correcting the offline calculation
                        for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                        {
                            // Are we passing sufficient information for this purpose? Let's see:
                            UInt_t ngoodhits = 0;

                            // std::cout << "Attempting common-mode correction for full-readout event sample " << isamp << "...";

                            double Correction = GetCommonModeCorrection(isamp, *it, ngoodhits, fN_APV25_CHAN, true);

                            double bias = fCM_online[isamp] - Correction - commonMode[isamp];

                            if (Correction != 0. && !CM_OUT_OF_RANGE)
                            {
                                UpdateRollingAverage(apvcounter, bias,
                                                     fCMbiasResultContainer_by_APV,
                                                     fCommonModeOnlineBiasRollingAverage_by_APV,
                                                     fCommonModeOnlineBiasRollingRMS_by_APV,
                                                     fNeventsOnlineBias_by_APV);
                            }

                            // Correction += 2.0*bias*(1.0-double(ngoodhits)/double(fN_APV25_CHAN));

                            // double Correction = 0.0;

                            // std::cout << " done." << std::endl;
                            // We are subtracting the result of the CM correction from the data.

                            UInt_t iAPV = it->pos;

                            if (fMakeCommonModePlots)
                            {
                                if (Correction != 0.)
                                {

                                    double CMbiasDB = (it->axis == SBSGEM::kUaxis) ? fCMbiasU[iAPV] : fCMbiasV[iAPV];
                                    double CMbias = CMbiasDB;

                                    if (fNeventsOnlineBias_by_APV[apvcounter] >= std::min(UInt_t(100), std::max(UInt_t(10), fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack)))
                                    {
                                        CMbias = fCommonModeOnlineBiasRollingAverage_by_APV[apvcounter];
                                    }

                                    if (it->axis == SBSGEM::kUaxis)
                                    {
                                        fCommonModeCorrectionU->Fill(iAPV, -Correction);
                                        fCommonModeResidualBiasU->Fill(iAPV, fCM_online[isamp] - Correction - commonMode[isamp]);
                                        fCommonModeResidualBias_vs_OccupancyU->Fill(double(ngoodhits) / double(fN_APV25_CHAN), fCM_online[isamp] - Correction - commonMode[isamp]);
                                        fCommonModeResidualBiasU_corrected->Fill(iAPV, fCM_online[isamp] - Correction - commonMode[isamp] - 2.0 * CMbias * (1.0 - double(ngoodhits) / double(fN_APV25_CHAN)));
                                    }
                                    else
                                    {
                                        fCommonModeCorrectionV->Fill(iAPV, -Correction);
                                        fCommonModeResidualBiasV->Fill(iAPV, fCM_online[isamp] - Correction - commonMode[isamp]);
                                        fCommonModeResidualBias_vs_OccupancyV->Fill(double(ngoodhits) / double(fN_APV25_CHAN), fCM_online[isamp] - Correction - commonMode[isamp]);
                                        fCommonModeResidualBiasV_corrected->Fill(iAPV, fCM_online[isamp] - Correction - commonMode[isamp] - 2.0 * CMbias * (1.0 - double(ngoodhits) / double(fN_APV25_CHAN)));
                                    }
                                }
                                else
                                {
                                    if (it->axis == SBSGEM::kUaxis)
                                    {
                                        fCommonModeDiffU_Uncorrected->Fill(iAPV, commonMode[isamp] - fCM_online[isamp]);
                                    }
                                    else
                                    {
                                        fCommonModeDiffV_Uncorrected->Fill(iAPV, commonMode[isamp] - fCM_online[isamp]);
                                    }
                                }
                            }
                        }
                    }
                } // check if conditions are satisfied to require offline common-mode calculation
            } // End check !CM_ENABLED && BUILD_ALL_SAMPLES

            if (CM_ENABLED && fCorrectCommonMode)
            {
                // Under certain conditions we want to attempt to correct the ADC values for all strips on an APV card using either
                // the rolling average over a certain number of previous events, or the CM mean from the database.
                // There are two conditions that must be satisfied to attempt correcting the ADC values for an event:
                // 1) The online calculated CM must be more than some number of std. deviations below the "expected" CM according to the rolling average
                // 2) The number of strips with ADC values within some number of std. deviations of the "expected" CM must exceed some threshold to allow us to
                //    to obtain a new estimate of the "true" common-mode for that event
                // If both 1) and 2) are satisfied, then we will attempt a new common-mode calculation using the strips that passed zero suppression using the online common-mode calculation.

                for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                {

                    UInt_t ngood = 0;
                    UInt_t nhitstemp = UInt_t(nstrips);

                    // std::cout << "Attempting common-mode correction for online zero-suppressed event sample " << isamp << "...";

                    CommonModeCorrection[isamp] = GetCommonModeCorrection(isamp, *it, ngood, nhitstemp);

                    if (CommonModeCorrection[isamp] != 0.0)
                    { // if we are applying a correction, correct it for bias:
                        UInt_t iAPV = it->pos;

                        double CMbiasDB = (it->axis == SBSGEM::kUaxis) ? fCMbiasU[iAPV] : fCMbiasV[iAPV];

                        double CMbias = CMbiasDB;

                        if (fNeventsOnlineBias_by_APV[apvcounter] >= std::min(UInt_t(100), std::max(UInt_t(10), fN_MPD_TIME_SAMP * fNeventsCommonModeLookBack)))
                        {
                            CMbias = fCommonModeOnlineBiasRollingAverage_by_APV[apvcounter];
                        }

                        // bias is DEFINED as Online common-mode MINUS correction MINUS "true" common-mode:
                        //"correction" is DEFINED as Online common-mode MINUS "corrected common-mode" and is to be ADDED to the ADC values:

                        // bias = online CM - (online CM - corrected CM) - true CM = corrected CM - true CM
                        // --> true CM = corrected CM - bias
                        // corrected ADC = ADC + online CM - true CM = uncorrected ADC + [online CM - (corrected CM - bias)]
                        // = uncorrected ADC + [correction + bias]
                        // [...] = correction to be ADDED to ADC
                        // --> corrected correction = correction + bias
                        CommonModeCorrection[isamp] += 2.0 * CMbias * (1.0 - double(ngood) / double(fN_APV25_CHAN));

                        //"TRUE" common-mode is equal to
                    }
                    // std::cout << " done." << std::endl;
                }
            }

            // std::cout << "finished common mode " << std::endl;
            //  Last loop over all the strips and samples in the data and populate/calculate global variables that are passed to track-finding:
            // Int_t ihit = 0;

            for (Int_t istrip = 0; istrip < nstrips; ++istrip)
            {
                // Temporary vector to hold ped-subtracted ADC samples for this strip:
                std::vector<double> ADCtemp(fN_MPD_TIME_SAMP);
                std::vector<int> rawADCtemp(fN_MPD_TIME_SAMP);

                // sums over time samples
                double ADCsum_temp = 0.0;
                double maxADC = 0.0;
                double minADC = 10000.0; // Negative pulse
                Int_t iSampMax = -1;
                Int_t iSampMin = -1; // Negative pulse

                // crude timing calculations:
                double Tsum = 0.0;
                double T2sum = 0.0;

                // grab decoded strip number directly:
                int strip = Strip[fN_MPD_TIME_SAMP * istrip];

                // Pedestal has already been subtracted by the time we get herre, but let's grab anyway in case it's needed:

                //"pedtemp" is only used to fill pedestal histograms as of now:
                double pedtemp = (axis == SBSGEM::kUaxis) ? fPedestalU[strip] : fPedestalV[strip];

                if (fPedSubFlag != 0 && !fIsMC && !fPedestalMode)
                    pedtemp = 0.0;

                double rmstemp = (axis == SBSGEM::kUaxis) ? fPedRMSU[strip] : fPedRMSV[strip];
                double gaintemp = (axis == SBSGEM::kUaxis) ? fUgain[strip / fN_APV25_CHAN] : fVgain[strip / fN_APV25_CHAN]; // should probably not hard-code 128 here

                // std::cout << "pedestal temp, rms temp, nsigma cut, threshold, zero suppress, pedestal mode = " << pedtemp << ", " << rmstemp << ", " << fZeroSuppressRMS
                //   	  << ", " << fZeroSuppressRMS * rmstemp << ", " << fZeroSuppress << ", " << fPedestalMode << std::endl;

                
                // Now loop over the time samples:
                for (Int_t adc_samp = 0; adc_samp < fN_MPD_TIME_SAMP; adc_samp++)
                {
                    int iraw = adc_samp + fN_MPD_TIME_SAMP * istrip;

                    // If applicable, subtract common-mode here:

                    // We need to subtract the common-mode if it was calculated offline:
                    if (!CM_ENABLED && BUILD_ALL_SAMPLES && nstrips == fN_APV25_CHAN)
                    {

                        //std::cout << "isamp, commonMode = " << adc_samp << ", " << commonMode[adc_samp]
                        //          << std::endl;
                        commonModeSubtractedADC[iraw] = pedsubADC[iraw] - commonMode[adc_samp];
                    }

                    // We need to apply a common-mode correction if we calculated one:
                    if (fCorrectCommonMode)
                        commonModeSubtractedADC[iraw] += CommonModeCorrection[adc_samp];

                    // Int_t ihit = adc_samp + fN_MPD_TIME_SAMP * istrip; //index in the "hit" array for this APV card:
                    // assert(ihit<nsamp);

                    // Int_t rawADC = evdata.GetData(it->crate, it->slot, chan, ihit);
                    Int_t RawADC = rawADC[iraw]; // this value has no corrections applied:

                    //std::cout << adc_samp << " " << istrip << " " << RawADC << " " << std::endl;

                    // rawADCtemp.push_back( RawADC );
                    rawADCtemp[adc_samp] = RawADC; // raw only

                    // The following value already has pedestal and common-mode subtracted (if applicable):
                    double ADCvalue = commonModeSubtractedADC[iraw]; // zero-suppress BEFORE we apply gain correction

                    // if( fPedestalMode ){ //If we are analyzing pedestal data, DON'T substract the pedestal
                    //   ADCvalue = commonModeSubtractedADC[adc_samp][istrip];
                    // }
                    // pedestal-subtracted ADC values:
                    // ADCtemp.push_back( ADCvalue );
                    ADCtemp[adc_samp] = ADCvalue; // common-mode AND pedestal subtracted
                    // fadc[adc_samp][fNch] =  evdata.GetData(it->crate, it->slot,
                    // 					 chan, isamp++) - fPedestal[strip];

                    ADCsum_temp += ADCvalue;
                    // cout << ADCvalue << " "<< endl;
                    if (iSampMax < 0 || ADCvalue > maxADC)
                    {
                        maxADC = ADCvalue;
                        iSampMax = adc_samp;
                    }

                    ///// Used for negative pulse study
                    if (iSampMin < 0 || ADCvalue < minADC)
                    {
                        minADC = ADCvalue;
                        iSampMin = adc_samp;
                    }

                    // for crude strip timing, just take simple time bins at the center of each sample (we'll worry about trigger time words later):
                    double Tsamp = fSamplePeriod * (adc_samp + 0.5);

                    Tsum += Tsamp * ADCvalue;
                    T2sum += pow(Tsamp, 2) * ADCvalue;

                    // assert( ((UInt_t) fNch) < fMPDmap.size()*fN_APV25_CHAN );
                    // assert( fNstrips_hit < fMPDmap.size()*fN_APV25_CHAN );
                }
                //	assert(strip>=0); // Make sure we don't end up with negative strip numbers!
                // Zero suppression based on third time sample only?
                // Maybe better to do based on max ADC sample:
                if(fNstrips_hit >= fMPDmap.size()*fN_APV25_CHAN) {
                    std::cout<<"XB Warning: more than "<<fMPDmap.size()*fN_APV25_CHAN<<" strips per chamber fired, skip current detector: "<<GetName()<<std::endl;
                    continue;
                }

                // if( !CM_ENABLED && BUILD_ALL_SAMPLES ){
                //   std::cout << " before zero suppression: common-mode and pedestal-subtracted ADC sum: effChan, istrip, ADC sum = "
                // 	    << effChan << ", " << istrip << ", " << ADCsum_temp << std::endl;
                // }

                // cout << ADCsum_temp << " >? " << fThresholdStripSum << "; "
                //    << ADCsum_temp/double(fN_MPD_TIME_SAMP) << " >? " << fZeroSuppressRMS*rmstemp << "; "
                //    << maxADC << " >? " << fZeroSuppressRMS*rmstemp
                //    << "; " << fThresholdSample << endl;

                // fill pedestal diagnostic histograms if and only if we are in pedestal mode or plot common mode
                //  AND the CM_ENABLED is not set, meaning we did cm and ped subtraction offline

                if ((fPedestalMode || fMakeCommonModePlots) && !CM_ENABLED)
                {
                    int iAPV = strip / fN_APV25_CHAN;

                    if (axis == SBSGEM::kUaxis)
                    {
                        for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                        {

                            hrawADCs_by_stripU->Fill(strip, rawADCtemp[isamp]);
                            hpedestal_subtracted_ADCs_by_stripU->Fill(strip, ADCtemp[isamp]);                        // common-mode AND ped-subtracted
                            hcommonmode_subtracted_ADCs_by_stripU->Fill(strip, ADCtemp[isamp] + pedtemp);            // common-mode subtraction only, no ped:
                            hpedestal_subtracted_rawADCs_by_stripU->Fill(strip, ADCtemp[isamp] + commonMode[isamp]); // pedestal subtraction only, no common-mode

                            hpedestal_subtracted_rawADCsU->Fill(ADCtemp[isamp] + commonMode[isamp]); // 1D distribution of ped-subtracted ADCs w/o common-mode subtraction
                            hpedestal_subtracted_ADCsU->Fill(ADCtemp[isamp]);                        // 1D distribution of ped-and-common-mode subtracted ADCs

                            hcommonmode_mean_by_APV_U->Fill(iAPV, commonMode[isamp]);
                        }
                    }
                    else
                    {
                        for (int isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                        {
                            // std::cout << "V axis: isamp, strip, rawADC, ADC, ped, commonmode = " << isamp << ", " << strip << ", "
                            // 	  << rawADCtemp[isamp] << ", " << ADCtemp[isamp] << ", " << pedtemp << ", " << commonMode[isamp] << std::endl;
                            hrawADCs_by_stripV->Fill(strip, rawADCtemp[isamp]);
                            hpedestal_subtracted_ADCs_by_stripV->Fill(strip, ADCtemp[isamp]);
                            hcommonmode_subtracted_ADCs_by_stripV->Fill(strip, ADCtemp[isamp] + pedtemp);
                            hpedestal_subtracted_rawADCs_by_stripV->Fill(strip, ADCtemp[isamp] + commonMode[isamp]);

                            hpedestal_subtracted_rawADCsV->Fill(ADCtemp[isamp] + commonMode[isamp]);
                            hpedestal_subtracted_ADCsV->Fill(ADCtemp[isamp]);

                            hcommonmode_mean_by_APV_V->Fill(iAPV, commonMode[isamp]);
                        }
                    }

                    // std::cout << "finished pedestal histograms..." << std::endl;
                }

                // the ROOTgui multicrate uses a threshold on the AVERAGE ADC sample (not the MAX). To be consistent
                //  with how the "Hit" root files are produced, let's use the same threshold;
                //  this amounts to using a higher effective threshold than cutting on the max ADC sample would have been:
                // fThresholdStripSum is in many respects redundant with fZeroSuppressRMS
                if (!fZeroSuppress || (ADCsum_temp / double(fN_MPD_TIME_SAMP) > fZeroSuppressRMS * rmstemp) )
                { 
                    // Default threshold is 5-sigma!
                    // Increment hit count and populate decoded data structures:
                    // cout<<ADCsum_temp/double(fN_MPD_TIME_SAMP)<<endl;
                    // threshold on the average ADC

                    // Slight reorganization: compute Tmean and Tsigma before applying gain correction:
                    //(since these sums were computed using the uncorrected ADC samples)
                    double Tmean_temp = Tsum / ADCsum_temp;
                    double Tsigma_temp = sqrt(T2sum / ADCsum_temp - pow(Tmean_temp, 2));

                    // NOW apply gain correction:
                    // Don't apply any gain correction if we are doing pedestal mode analysis:
                    if (!fPedestalMode)
                    { // only apply gain correction if we aren't in pedestal-mode:
                        for (Int_t isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                        {
                            ADCtemp[isamp] *= gaintemp;
                        }
                        maxADC *= gaintemp;
                        ADCsum_temp *= gaintemp;
                    }

                    //  ADCsum_temp /= gaintemp;

                    // fStrip.push_back( strip );
                    // fAxis.push_back( axis );

                    // std::cout << "strip, axis = " << strip << ", " << axis << std::endl;

                    fStrip[fNstrips_hit] = strip;
                    fAxis[fNstrips_hit] = axis;
                    fStripRaw[fNstrips_hit] = rawStrip[fN_MPD_TIME_SAMP * istrip];

                    fKeepStrip[fNstrips_hit] = true;
                    // fStripKeep[fNstrips_hit] = 1;
                    //	  fMaxSamp.push_back( iSampMax );
                    fMaxSamp[fNstrips_hit] = iSampMax;

                    // if( fSuppressFirstLast && (iSampMax == 0 || iSampMax+1 == fN_MPD_TIME_SAMP ) ){
                    //  fSuppressFirstLast:
                    //  0 = allow peaking in first or last sample
                    //  1 = suppress peaking in first and last sample
                    //  -1 = suppress peaking in first sample only (or other negative number)
                    //  -2 = suppress peaking in last sample only:
                    if (fDeconvolutionFlag == 0)
                    { // if "deconvolution flag" is non-zero, then set "keep strip" based on deconvoluted variables
                        if (fSuppressFirstLast != 0)
                        {
                            bool peakfirst = iSampMax == 0;
                            bool peaklast = iSampMax + 1 == fN_MPD_TIME_SAMP;

                            if (peakfirst)
                            {
                                if (fSuppressFirstLast > 0 || fSuppressFirstLast != -2)
                                {
                                    fKeepStrip[fNstrips_hit] = false;
                                }
                            }
                            else if (peaklast)
                            {
                                if (fSuppressFirstLast > 0 || fSuppressFirstLast == -2)
                                {
                                    fKeepStrip[fNstrips_hit] = false;
                                }
                            }
                        }
                    }

                    double ADCsum_deconv = 0.0;
                    double maxdeconv = 0.0;
                    int imaxdeconv = 0;

                    double Tsum_deconv = 0.0;

                    double maxcombo = 0.0;
                    std::vector<double> combostemp(fN_MPD_TIME_SAMP + 1, 0.0);
                    int imaxcombo = 0;

                    for (Int_t isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                    {
                        //std::cout<<"fNstrips_hit = "<<fNstrips_hit<<", isamp = "<<isamp<<std::endl;
                        //if(fNstrips_hit >= 512 ) {std::cout<<"more than 512 strips, something wrong."<<std::endl;getchar();}
                        fADCsamples[fNstrips_hit][isamp] = ADCtemp[isamp];
                        fRawADCsamples[fNstrips_hit][isamp] = rawADCtemp[isamp];

                        double Adeconvtemp = fDeconv_weights[0] * ADCtemp[isamp];
                        if (isamp > 0)
                            Adeconvtemp += fDeconv_weights[1] * ADCtemp[isamp - 1];
                        if (isamp > 1)
                            Adeconvtemp += fDeconv_weights[2] * ADCtemp[isamp - 2];

                        fADCsamples_deconv[fNstrips_hit][isamp] = Adeconvtemp;

                        ADCsum_deconv += Adeconvtemp;

                        if (isamp == 0)
                        {
                            combostemp[isamp] = Adeconvtemp;
                            maxcombo = combostemp[isamp];
                            imaxcombo = isamp;
                        }
                        else
                        {
                            combostemp[isamp] = Adeconvtemp + fADCsamples_deconv[fNstrips_hit][isamp - 1];
                            if (combostemp[isamp] > maxcombo)
                            {
                                maxcombo = combostemp[isamp];
                                imaxcombo = isamp;
                            }
                        }
                        if (isamp == 5)
                        {
                            combostemp[fN_MPD_TIME_SAMP] = Adeconvtemp;
                            if (combostemp[fN_MPD_TIME_SAMP] > maxcombo)
                            {
                                maxcombo = combostemp[isamp];
                                imaxcombo = fN_MPD_TIME_SAMP;
                            }
                        }

                        if (isamp == 0 || Adeconvtemp > maxdeconv)
                        {
                            imaxdeconv = isamp;
                            maxdeconv = Adeconvtemp;
                        }

                        Tsum_deconv += fSamplePeriod * (isamp + 0.5) * Adeconvtemp;

                        fADCsamples1D[isamp + fN_MPD_TIME_SAMP * fNstrips_hit] = ADCtemp[isamp];
                        fRawADCsamples1D[isamp + fN_MPD_TIME_SAMP * fNstrips_hit] = rawADCtemp[isamp];
                        fADCsamplesDeconv1D[isamp + fN_MPD_TIME_SAMP * fNstrips_hit] = fADCsamples_deconv[fNstrips_hit][isamp];

                        if (fKeepStrip[fNstrips_hit] && hADCfrac_vs_timesample_allstrips != NULL)
                        {
                            hADCfrac_vs_timesample_allstrips->Fill(isamp, ADCtemp[isamp] / ADCsum_temp);
                        }
                    }

                    fADCsumsDeconv[fNstrips_hit] = ADCsum_deconv;
                    // fStripTrackIndex.push_back( -1 ); //This could be modified later based on tracking results
                    fStripTrackIndex[fNstrips_hit] = -1;
                    fStripOnTrack[fNstrips_hit] = 0;

                    // This block is only used for the negative signal studies
                    fStripIsNeg[fNstrips_hit] = 0;
                    fStripIsNegU[fNstrips_hit] = 0;
                    fStripIsNegV[fNstrips_hit] = 0;
                    fStripIsNegOnTrack[fNstrips_hit] = 0;
                    fStripIsNegOnTrackU[fNstrips_hit] = 0;
                    fStripIsNegOnTrackV[fNstrips_hit] = 0;

                    // These are used for saving numbers to a text file for event displays
                    fStripEvent[fNstrips_hit] = evdata.GetEvNum();
                    fStripMPD[fNstrips_hit] = it->mpd_id;
                    fStripADC_ID[fNstrips_hit] = it->adc_id;
                    /// This block above is used for negative signal studies

                    //	  fKeepStrip.push_back( true ); //keep all strips by default

                    //	  fADCmax.push_back( maxADC );
                    fADCmax[fNstrips_hit] = maxADC;
                    fADCmaxDeconv[fNstrips_hit] = maxdeconv;
                    fADCmaxDeconvCombo[fNstrips_hit] = maxcombo;

                    fMaxSampDeconv[fNstrips_hit] = imaxdeconv;
                    fMaxSampDeconvCombo[fNstrips_hit] = imaxcombo;

                    if (fDeconvolutionFlag != 0)
                    {
                        if (fSuppressFirstLast != 0 && imaxcombo == 0 || imaxcombo == fN_MPD_TIME_SAMP)
                        {
                            fKeepStrip[fNstrips_hit] = false;
                        }
                        if (maxcombo <= fZeroSuppressRMS * rmstemp)
                        {
                            fKeepStrip[fNstrips_hit] = false;
                        }
                    }

                    fTmeanDeconv[fNstrips_hit] = Tsum_deconv / ADCsum_deconv;

                    //	  fTmean.push_back( Tsum/ADCsum_temp );
                    fTmean[fNstrips_hit] = Tmean_temp;
                    //  fTsigma.push_back( sqrt( T2sum/ADCsum_temp - pow( fTmean.back(), 2 ) ) );
                    fTsigma[fNstrips_hit] = Tsigma_temp;
                    // fTcorr.push_back( fTmean.back() ); //don't apply any corrections for now

                    fStripTSchi2[fNstrips_hit] = StripTSchi2(fNstrips_hit);

                    if (fUseTSchi2cut && fStripTSchi2[fNstrips_hit] > fStripTSchi2Cut)
                    {
                        fKeepStrip[fNstrips_hit] = false;
                    }

                    fStripTdiff[fNstrips_hit] = -1000.;     // This will become meaningful only at the clustering stage
                    fStripCorrCoeff[fNstrips_hit] = -1000.; // This will become meaningful only at the clustering stage
                    fTcorr[fNstrips_hit] = fTmean[fNstrips_hit];

                    // fStripTfit[fNstrips_hit] = FitStripTime( fNstrips_hit, rmstemp*2.45 );
                    fStripTfit[fNstrips_hit] = fTmean[fNstrips_hit];

                    fStrip_ENABLE_CM[fNstrips_hit] = CM_ENABLED;
                    fStrip_CM_GOOD[fNstrips_hit] = !CM_OUT_OF_RANGE;
                    fStrip_BUILD_ALL_SAMPLES[fNstrips_hit] = BUILD_ALL_SAMPLES;

                    fADCsums[fNstrips_hit] = ADCsum_temp;

                    fStripADCavg[fNstrips_hit] = ADCsum_temp / double(fN_MPD_TIME_SAMP);

                    UInt_t isU = (axis == SBSGEM::kUaxis) ? 1 : 0;
                    UInt_t isV = (axis == SBSGEM::kVaxis) ? 1 : 0;

                    fStripIsU[fNstrips_hit] = isU;
                    fStripIsV[fNstrips_hit] = isV;

                    fStripUonTrack[fNstrips_hit] = 0;
                    fStripVonTrack[fNstrips_hit] = 0;

                    fNstrips_hitU += isU;
                    fNstrips_hitV += isV;

                    if (fKeepStrip[fNstrips_hit])
                    {
                        fNstrips_keep++;
                        fNstrips_keepU += isU;
                        fNstrips_keepV += isV;
                        if (fADCmax[fNstrips_hit] >= fThresholdSample && fADCsums[fNstrips_hit] >= fThresholdStripSum)
                        {
                            fNstrips_keep_lmax++;
                            fNstrips_keep_lmaxU += isU;
                            fNstrips_keep_lmaxV += isV;
                        }
                    }

                    fNstrips_hit++;
                    fNstrips_hit_pos++;
                } // check if passed zero suppression cuts

                /////// Negative pulse study, This is an exact copy of the loop above but instead stores the negative ADC info. "Keep" is set
                /////// to false regardless so these strips will not be used for any of the normal clustering and tracking algorithms. They
                /////// are differentiated from positive strips by fStripIsNeg.
                if (ADCsum_temp / double(fN_MPD_TIME_SAMP) < -1.0 * fZeroSuppressRMS * rmstemp && BUILD_ALL_SAMPLES && !CM_ENABLED && !CM_OUT_OF_RANGE && fNegSignalStudy)
                {
                    // Increment hit count and populate decoded data structures:

                    // threshold on the average ADC

                    // Slight reorganization: compute Tmean and Tsigma before applying gain correction:
                    //(since these sums were computed using the uncorrected ADC samples)
                    double Tmean_temp = Tsum / ADCsum_temp;
                    double Tsigma_temp = sqrt(-1.0 * T2sum / ADCsum_temp - pow(Tmean_temp, 2));

                    // NOW apply gain correction:
                    // Don't apply any gain correction if we are doing pedestal mode analysis:
                    if (!fPedestalMode)
                    { // only apply gain correction if we aren't in pedestal-mode:
                        for (Int_t isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                        {
                            ADCtemp[isamp] *= gaintemp;
                        }
                        minADC *= gaintemp; // Use min ADC instead of max for negative signals
                        ADCsum_temp *= gaintemp;
                    }

                    fStrip[fNstrips_hit] = strip;
                    fAxis[fNstrips_hit] = axis;
                    fStripRaw[fNstrips_hit] = rawStrip[fN_MPD_TIME_SAMP * istrip];

                    fMaxSamp[fNstrips_hit] = iSampMin;

                    // if( fSuppressFirstLast && (iSampMax == 0 || iSampMax+1 == fN_MPD_TIME_SAMP ) ){
                    //  fSuppressFirstLast:
                    //  0 = allow peaking in first or last sample
                    //  1 = suppress peaking in first and last sample
                    //  -1 = suppress peaking in first sample only (or other negative number)
                    //  -2 = suppress peaking in last sample only:
                    if (fSuppressFirstLast != 0)
                    {
                        bool peakfirst = iSampMin == 0;
                        bool peaklast = iSampMin + 1 == fN_MPD_TIME_SAMP;

                        if (peakfirst)
                        {
                            if (fSuppressFirstLast > 0 || fSuppressFirstLast != -2)
                            {
                                fKeepStrip[fNstrips_hit] = false;
                            }
                        }
                        else if (peaklast)
                        {
                            if (fSuppressFirstLast > 0 || fSuppressFirstLast == -2)
                            {
                                fKeepStrip[fNstrips_hit] = false;
                            }
                        }
                    }

                    for (Int_t isamp = 0; isamp < fN_MPD_TIME_SAMP; isamp++)
                    {
                        fADCsamples[fNstrips_hit][isamp] = ADCtemp[isamp];
                        fRawADCsamples[fNstrips_hit][isamp] = rawADCtemp[isamp];

                        // fADCsamples1D.push_back( ADCtemp[isamp] );
                        // fRawADCsamples1D.push_back( rawADCtemp[isamp] );
                        fADCsamples1D[isamp + fN_MPD_TIME_SAMP * fNstrips_hit] = ADCtemp[isamp];
                        fRawADCsamples1D[isamp + fN_MPD_TIME_SAMP * fNstrips_hit] = rawADCtemp[isamp];

                        if (fKeepStrip[fNstrips_hit] && hADCfrac_vs_timesample_allstrips != NULL)
                        {
                            hADCfrac_vs_timesample_allstrips->Fill(isamp, ADCtemp[isamp] / ADCsum_temp);
                        }
                    }
                    // fStripTrackIndex.push_back( -1 ); //This could be modified later based on tracking results
                    fStripTrackIndex[fNstrips_hit] = -1;
                    fStripOnTrack[fNstrips_hit] = 0;

                    // Variables used for negative signal studies
                    fStripIsNeg[fNstrips_hit] = 1;
                    fStripIsNegU[fNstrips_hit] = (axis == SBSGEM::kUaxis) ? 1 : 0;
                    fStripIsNegV[fNstrips_hit] = (axis == SBSGEM::kVaxis) ? 1 : 0;
                    fStripIsNegOnTrack[fNstrips_hit] = 0;
                    fStripIsNegOnTrackU[fNstrips_hit] = 0;
                    fStripIsNegOnTrackV[fNstrips_hit] = 0;

                    // These are used for saving numbers to a text file for event displays
                    fStripEvent[fNstrips_hit] = evdata.GetEvNum();
                    fStripMPD[fNstrips_hit] = it->mpd_id;
                    fStripADC_ID[fNstrips_hit] = it->adc_id;

                    fADCmax[fNstrips_hit] = minADC; // Use minADC instead for negative strips
                    fTmean[fNstrips_hit] = Tmean_temp;
                    fTsigma[fNstrips_hit] = Tsigma_temp;

                    fStripTSchi2[fNstrips_hit] = StripTSchi2(fNstrips_hit);

                    if (fUseTSchi2cut && fStripTSchi2[fNstrips_hit] > fStripTSchi2Cut)
                    {
                        fKeepStrip[fNstrips_hit] = false;
                    }

                    fStripTdiff[fNstrips_hit] = -1000.;     // This will become meaningful only at the clustering stage
                    fStripCorrCoeff[fNstrips_hit] = -1000.; // This will become meaningful only at the clustering stage
                    fTcorr[fNstrips_hit] = fTmean[fNstrips_hit];

                    fStripTfit[fNstrips_hit] = fTmean[fNstrips_hit];

                    fStrip_ENABLE_CM[fNstrips_hit] = CM_ENABLED;
                    fStrip_CM_GOOD[fNstrips_hit] = !CM_OUT_OF_RANGE;
                    fStrip_BUILD_ALL_SAMPLES[fNstrips_hit] = BUILD_ALL_SAMPLES;

                    fADCsums[fNstrips_hit] = ADCsum_temp;

                    fStripADCavg[fNstrips_hit] = ADCsum_temp / double(fN_MPD_TIME_SAMP);

                    UInt_t isU = (axis == SBSGEM::kUaxis) ? 1 : 0;
                    UInt_t isV = (axis == SBSGEM::kVaxis) ? 1 : 0;

                    fStripIsU[fNstrips_hit] = isU;
                    fStripIsV[fNstrips_hit] = isV;

                    fStripUonTrack[fNstrips_hit] = 0;
                    fStripVonTrack[fNstrips_hit] = 0;

                    fNstrips_hitU_neg += isU;
                    fNstrips_hitV_neg += isV;

                    if (fKeepStrip[fNstrips_hit])
                    {
                        fNstrips_keep++;
                        fNstrips_keepU += isU;
                        fNstrips_keepV += isV;
                        if (fADCmax[fNstrips_hit] >= fThresholdSample && fADCsums[fNstrips_hit] >= fThresholdStripSum)
                        {
                            fNstrips_keep_lmax++;
                            fNstrips_keep_lmaxU += isU;
                            fNstrips_keep_lmaxV += isV;
                        }
                    }

                    fNstrips_hit++;
                    fNstrips_hit_neg++;
                } // end loop over negative zero suppression for full readout events
            } // end loop over strips on this APV card
        } // end if( nsamp > 0 )

        apvcounter++;
    } // end loop on decode map entries for this module


    fNdecoded_ADCsamples = fNstrips_hit * fN_MPD_TIME_SAMP;

    fIsDecoded = true;
}
