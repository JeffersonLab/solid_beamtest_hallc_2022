#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <exception>
#include "TTree.h"
#include "TFile.h"
#include "EvStruct.h"
#include "EvChannel.h"
#include "Fadc250Decoder.h"
#include "WfAnalyzer.h"
#include "SSPDecoder.h"
#include "ConfigArgs.h"
#include "read_modules.h"
#include "TiDecoder.h"
#include "EPICSystem.h"
#include "epics_tree_struct.h"
#include <chrono>
#include "ReadDatabase.h"


//In this file, disable the debug version of tracking, it is too slow
#undef USE_GEM_TRACKING

#ifdef USE_GEM_TRACKING
#include "GEMModule.h"
#include "HCTracking.h"
#include "EventWrapper.h"
#define PROGRESS_COUNT 10
#else
#include "GEMSystem.h"
#include "gem_tree_struct.h"
#include "APVStripMapping.h"
#include "MPDSSPRawEventDecoder.h"
#define PROGRESS_COUNT 1000
#endif

void write_raw_data(const std::string &dpath, const std::string &opath, const std::string &mpath, int nev,
                    int nskip=0, int res=3, double thres=10, int npeds=5, double flat=1.0, int usefixedped=0);

int GetRunNumber(std::string str);


// event types
enum EvType {
    CODA_PRST = 0xffd1,
    CODA_GO = 0xffd2,
    CODA_END = 0xffd4,
    CODA_PHY1 = 0xff50,
    CODA_PHY2 = 0xff70,
    CODA_EPICS = 0x83,
};

int main(int argc, char*argv[])
{
    ConfigArgs arg_parser;
    arg_parser.AddHelps({"-h", "--help"});
    arg_parser.AddPositional("raw_data", "raw data in evio format");
    arg_parser.AddPositional("root_file", "output data file in root format");
    arg_parser.AddArg<int>("-n", "nev", "number of events to process (< 0 means all)", -1);
    arg_parser.AddArg<int>("-k", "nskip", "number of events to skip", 0);
    arg_parser.AddArgs<std::string>({"-m", "--module"}, "module", "json file for module configuration",
                                    "database/modules/mapmt_module.json");
    arg_parser.AddArg<int>("-r", "res", "resolution for waveform analysis", 3);
    arg_parser.AddArg<double>("-t", "thres", "peak threshold for waveform analysis", 10.0);
    arg_parser.AddArg<int>("-p", "npeds", "sample window width for pedestal searching", 8);
    arg_parser.AddArg<double>("-f", "flat", "flatness requirement for pedestal searching", 1.0);
    arg_parser.AddArg<int>("-x", "usefixedped", "whether or not to use fixed FADC pedestals", 0);

    auto args = arg_parser.ParseArgs(argc, argv);

    // show arguments
    for(auto &it : args) {
        std::cout << it.first << ": " << it.second.String() << std::endl;
    }

    write_raw_data(args["raw_data"].String(),
                   args["root_file"].String(),
                   args["module"].String(),
                   args["nev"].Int(),
                   args["nskip"].Int(),
                   args["res"].Int(),
                   args["thres"].Double(),
                   args["npeds"].Int(),
                   args["flat"].Double(),
                   args["usefixedped"].Int());
    return 0;
}

// create an event tree according to modules
TTree *create_tree(std::vector<Module> &modules, const std::string tname = "EvTree",
                   const std::string &ttitle = "SoLID Ecal HallC BeamTest Events")
{
    auto tree = new TTree(tname.c_str(), ttitle.c_str());
    for (auto &m : modules) {
        switch (m.type) {
        case kFADC250:
            {
                // 16 channels
                auto event = new fdec::Fadc250Event(0, 16);
                m.event = static_cast<void*>(event);
                for (auto &ch : m.channels) {
                    auto n = ch.name;
                    tree->Branch(n.c_str(), &event->channels[ch.id], 32000, 0);
                }
            }
            break;
        case kSSP:
            {
#ifdef USE_GEM_TRACKING
#else
                auto event = new GEMTreeStruct();
                m.event = static_cast<void*>(event);
                tree->Branch("event_number",                    &event->event_number,    "event_number/I");
                std::string sname = "GEM";
                tree->Branch((sname + "_nCluster").c_str(),     &event->nCluster,        "GEM_nCluster/I");
                tree->Branch((sname + "_planeID").c_str(),      &event->Plane[0],        "GEM_planeID[GEM_nCluster]/I");
                tree->Branch((sname + "_prodID").c_str(),       &event->Prod[0],         "GEM_prodID[GEM_nCluster]/I");
                tree->Branch((sname + "_moduleID").c_str(),     &event->Module[0],       "GEM_moduleID[GEM_nCluster]/I");
                tree->Branch((sname + "_axis").c_str(),         &event->Axis[0],         "GEM_axis[GEM_nCluster]/I");
                tree->Branch((sname + "_size").c_str(),         &event->Size[0],         "GEM_size[GEM_nCluster]/I");
                tree->Branch((sname + "_adc").c_str(),          &event->Adc[0],          "GEM_adc[GEM_nCluster]/F");
                tree->Branch((sname + "_pos").c_str(),          &event->Pos[0],          "GEM_pos[GEM_nCluster]/F");

                tree->Branch((sname + "_stripNo").c_str(),      &event->StripNo);
                tree->Branch((sname + "_stripAdc").c_str(),     &event->StripAdc);
                tree->Branch((sname + "_stripTs0").c_str(),     &event->StripTs0);
                tree->Branch((sname + "_stripTs1").c_str(),     &event->StripTs1);
                tree->Branch((sname + "_stripTs2").c_str(),     &event->StripTs2);
                tree->Branch((sname + "_stripTs3").c_str(),     &event->StripTs3);
                tree->Branch((sname + "_stripTs4").c_str(),     &event->StripTs4);
                tree->Branch((sname + "_stripTs5").c_str(),     &event->StripTs5);

                tree->Branch((sname + "_nAPV").c_str(),         &event->nAPV,            "GEM_nAPV/I");
                tree->Branch((sname + "_apv_crate_id").c_str(), &event->apv_crate_id[0], "GEM_apv_crate_id[GEM_nAPV]/I");
                tree->Branch((sname + "_apv_mpd_id").c_str(),   &event->apv_mpd_id[0],   "GEM_apv_mpd_id[GEM_nAPV]/I");
                tree->Branch((sname + "_apv_adc_ch").c_str(),   &event->apv_adc_ch[0],   "GEM_apv_adc_ch[GEM_nAPV]/I");
#endif
            }
            break;
        case kTI:
            {
                std::cout<<" TI bank encountered..."<<m.type<<std::endl;
            }
            break;
        default:
            std::cout << "Unsupported module type " << m.type << std::endl;
            break;
        }
    }
    return tree;
}

#ifndef USE_GEM_TRACKING
// do gem clustering
void extract_gem_cluster(GEMSystem *gem_sys, MPDSSPRawEventDecoder *gem_decoder, GEMTreeStruct &gem_data, int evtNum)
{
    auto &decoded_data = gem_decoder -> GetAPV();
    auto &decoded_data_flags = gem_decoder -> GetAPVDataFlags();

    EventData event_data;
    for(auto &i: decoded_data){
        gem_sys -> FillRawDataMPD(i.first, i.second, decoded_data_flags.at(i.first), event_data);
    }

    gem_sys -> Reconstruct(event_data);

    // gem system fill gem_data
    gem_data.Clear();

    auto getChamberBasedStripNo = [](int strip, int type, int N_APVS_PER_PLANE, int detLayerPositionIndex)
        -> int
    {
        // no conversion for Y plane
        if(type == 1)
            return strip;

        // conversion for X plane
        int c_strip = strip - N_APVS_PER_PLANE * 128 * detLayerPositionIndex;
        if(strip < 0)
        {
            std::cout<<"Error: strip conversion failed, returned without conversion."
                <<std::endl;
            return strip;
        }
        return c_strip;
    };

    gem_data.nAPV = apv_strip_mapping::Mapping::Instance() -> GetTotalNumberOfAPVs();
    std::vector<GEMDetector*> detectors = gem_sys -> GetDetectorList();
    int icluster = 0, apv_index = 0;
    for(auto &i: detectors)
    {
        std::vector<GEMPlane*> planes = i -> GetPlaneList();
        for(auto &pln: planes) {
            const std::vector<StripCluster> &clusters = pln -> GetStripClusters();
            int napvs_per_plane = pln -> GetCapacity();
            for(auto &c: clusters)
            {
                if(icluster >= MAXCLUSTERS)
                    break;

                gem_data.Plane[icluster] = i -> GetLayerID();
                gem_data.Prod[icluster] = i -> GetDetID();
                gem_data.Module[icluster] = i -> GetDetLayerPositionIndex();
                gem_data.Axis[icluster] = static_cast<int>(pln -> GetType());
                gem_data.Size[icluster] = c.hits.size();
                gem_data.Adc[icluster] = c.peak_charge;
                gem_data.Pos[icluster] = c.position;

                // strips in this cluster
                const std::vector<StripHit> &hits = c.hits;
                for(size_t nS = 0; nS < hits.size() && nS < MAXCLUSTERSIZE; ++nS)
                {
                    // layer based strip no
                    //StripNo[nCluster][nS] = hits[nS].strip;

                    // chamber based strip no
                    int tmp_strip_no = getChamberBasedStripNo(hits[nS].strip, gem_data.Axis[icluster],
                            napvs_per_plane, gem_data.Module[icluster]);
                    gem_data.StripNo.push_back(tmp_strip_no);

                    gem_data.StripAdc.push_back(hits[nS].charge);

                    gem_data.StripTs0.push_back(hits[nS].ts_adc[0]);
                    gem_data.StripTs1.push_back(hits[nS].ts_adc[1]);
                    gem_data.StripTs2.push_back(hits[nS].ts_adc[2]);
                    gem_data.StripTs3.push_back(hits[nS].ts_adc[3]);
                    gem_data.StripTs4.push_back(hits[nS].ts_adc[4]);
                    gem_data.StripTs5.push_back(hits[nS].ts_adc[5]);
                }
                icluster++;
            }

            // apvs
            std::vector<GEMAPV*> apv_list = pln -> GetAPVList();
            for(auto &apv: apv_list)
            {
                gem_data.apv_crate_id[apv_index] = apv->GetAddress().crate_id;
                gem_data.apv_mpd_id[apv_index] = apv->GetAddress().mpd_id;
                gem_data.apv_adc_ch[apv_index] = apv->GetAddress().adc_ch;
            }
        }
    }
    gem_data.nCluster = icluster;
    gem_data.event_number = evtNum;
}
#endif

// create an epics tree
TTree* create_epics_tree(EPICSystem *epic_sys, const std::string tname = "EpicsTree", const std::string name = "epics tree")
{
    auto tree = new TTree(tname.c_str(), name.c_str());

    epics_struct::EPICStruct *epic_dat = new epics_struct::EPICStruct();

    auto & e_map = epic_sys -> GetEpicsMap();
    for(auto &i: e_map) {
        (epic_dat->data)[i.first] = -999999.9;
    }

    epics_struct::__g_epic_data = static_cast<epics_struct::EPICStruct*>(epic_dat);

    std::string sname = "Epics_";
    tree -> Branch((sname + "event_number").c_str(), &epics_struct::__g_epic_data->event_number, "eventNo/I");
    tree -> Branch((sname + "timestamp").c_str(), &epics_struct::__g_epic_data->timestamp);
    for(auto &entry: epics_struct::__g_epic_data->data)
    {
        std::string branch_name = sname + entry.first;

        // branch name does not allow "." and ":", which is in CODA epics name
        size_t dot_pos = branch_name.find(".");
        while(dot_pos != std::string::npos) {
            branch_name = branch_name.replace(dot_pos, 1, "_");
            dot_pos = branch_name.find(".");
        }

        dot_pos = branch_name.find(":");
        while(dot_pos != std::string::npos){
            branch_name = branch_name.replace(dot_pos, 1, "_");
            dot_pos = branch_name.find(":");
        }

        tree -> Branch(branch_name.c_str(), &entry.second, Form("%s/F", branch_name.c_str()));
    }
    return tree;
}

// analyze event and fill epics tree
void fill_epics_event(const uint32_t *buf, EPICSystem* epic_sys, const int event_number, TTree *T)
{
    // skip top level bank header information
    auto evh = evc::BankHeader(&buf[0] + 2);
    size_t len = evc::BankHeader::size();

    // skip epics bank header information
    epic_sys -> FillRawData( (const char*)(&buf[0] + 4) );

    // fill epics event data

    epics_struct::__g_epic_data -> event_number = event_number;
    epics_struct::__g_epic_data -> timestamp = epic_sys -> GetCurrentTimeStamp();

    for(auto &i: epics_struct::__g_epic_data -> data)
    {
        i.second = epic_sys -> GetEpicsValueByName(i.first);
    }

    T -> Fill();
}

// read raw data in evio format, and extract information
void write_raw_data(const std::string &dpath, const std::string &opath, const std::string &mpath, int nev,
                    int nskip, int res, double thres, int npeds, double flat, int usefixedped)
{
    // read modules
    auto modules = read_modules(mpath);

    if (modules.empty()) {
        std::cout << "Cannot find any modules in configuration file \"" << mpath << "\"" << std::endl;
        return;
    }

    ReadDatabase("database/FADCPedestal.txt");
    int run = GetRunNumber(dpath);

    db::dbBlock *dbFADCPed = db::FindBlock("FADCPedestal",run);
    if(dbFADCPed) dbFADCPed->Print();
    if(!dbFADCPed && usefixedped) {
        cout<<"Warning: could not find 'FADCPedestal' for run "<<run<<" in the database file database/FADCPedestal.txt"<<endl;
        cout<<"\t The decoder will extract the pedestal event by event."<<endl;
        usefixedped=0;
    }

    // raw data
    evc::EvChannel evchan;
    if (evchan.Open(dpath) != evc::status::success) {
        std::cout << "Cannot open evchannel at " << dpath << std::endl;
        return;
    }

    // get banks
    std::vector<uint32_t> dbanks;
    for (auto &m : modules) {
        if (std::find(dbanks.begin(), dbanks.end(), m.bank) == dbanks.end()) {
            dbanks.push_back(m.bank);
        }
    }
    // waveform analyzer
    fdec::Analyzer analyzer(res, thres, npeds, flat);

#ifdef USE_GEM_TRACKING
    EventWrapper evio_event_wrapper;
#else
    // gem analyzer
    GEMSystem gem_system;
    gem_system.Configure("config/gem.conf");
    gem_system.ReadPedestalFile();
#endif

    // decoders
    fdec::Fadc250Decoder fdecoder;
    ssp::SSPDecoder sdecoder;

#ifdef USE_GEM_TRACKING
    HCTracking *tracking = new HCTracking();
    tracking->LoadConfig(TDatime("2022-11-03 16:35:00"));
    tracking->CompleteInitialization();
    tracking -> Begin(9999);
#else
    MPDSSPRawEventDecoder gem_decoder;
#endif

    // epics system
    EPICSystem epic_sys("config/epics_map.txt");
    
    // output
    auto *hfile = new TFile(opath.c_str(), "RECREATE", "SoLID HallC Beamtest");
    auto tree = create_tree(modules);
#ifdef USE_GEM_TRACKING
    tracking -> InitTrackingResultTree(tree);
#endif

    int TriggerType = -1;
    uint64_t TriggerTime = 0;
    tree -> Branch("trigger_type", &TriggerType, "trigger_type/I");
    tree -> Branch("trigger_time", &TriggerTime, "trigger_time/l");
    

    auto epics_tree = create_epics_tree(&epic_sys);

    int count = 0;
    if(nskip>0) nev += nskip;
    auto &ref = modules.front();
    while ((evchan.Read() == evc::status::success) && (nev-- != 0)) {
        if(count>0 && count<nskip)  {count++;continue;} //keep the first prestart event for absolute trigger time
        if(nskip>0 && count==nskip) {
            std::cout << "First " << nskip <<" events were skipped." << std::endl;
        }
        if((count % PROGRESS_COUNT) == 0) {
            std::cout << "Processed events - " << count << "\r" << std::flush;
        }

        switch(evchan.GetEvHeader().tag) {
        // only want physics events
        case CODA_PHY1:
        case CODA_PHY2:
            break;
        case CODA_EPICS:
            {
                fill_epics_event(evchan.GetRawBuffer(), &epic_sys, count, epics_tree);
            }
            continue;
        case CODA_PRST:
        case CODA_GO:
        case CODA_END:
        default:
            continue;
        }

        evchan.ScanBanks(dbanks);
        // get block level
        int blvl = evchan.GetEvBuffer(ref.crate, ref.bank, ref.slot).size();
        uint16_t event_type = evchan.GetEventType();
        TriggerType = (int)(event_type);

        const uint32_t *dbuf;
        size_t buflen;
        for (int ii = 0; ii < blvl; ++ii) {
            int imod = -1;
            // parse module data
            for (auto &mod : modules) {
                // get data buffer
                try {
                    dbuf = evchan.GetEvBuffer(mod.crate, mod.bank, mod.slot, ii, buflen);
                } catch (std::exception &e) {
                    std::cout << "warning: " << e.what() << "\n";
                    continue;
                }
                // decode by module type
                switch (mod.type) {
                case kFADC250:
                    {
                        auto event = static_cast<fdec::Fadc250Event*>(mod.event);
                        fdecoder.DecodeEvent(*event, dbuf, buflen);
                        imod++;
                        size_t idx=16*imod;
                        for (auto &ch : event->channels) {
                            double fixedPed = 0.0, fixedPedErr = 0.0;
                            if(usefixedped) {
                                if(idx >= dbFADCPed->Data.size()) {
                                    std::cout<<"Error! Number of channels in FADC Pedestal file is less than that in the raw data, I quit ... \n";
                                    exit(-1);
                                }
                                fixedPed = dbFADCPed->Data[idx];
                                fixedPedErr=1.5; //dbFADCPed->Data[idx+32];
                            }

                            //std::cout<<"event = "<<count<<",  imod = "<<imod<<",  FADCPed["<<idx<<"] = "<<fixedPed<<", fixedPedErr="<<fixedPedErr<<std::endl;
                            idx++;
                            analyzer.Analyze(ch,fixedPed,fixedPedErr);
                        }
                    }
                    break;
                case kSSP:
                    {
#ifdef USE_GEM_TRACKING
                        auto raw_vec = evchan.GetRawBufferVec();
                        evio_event_wrapper.LoadEvent(raw_vec.data(), raw_vec.size());
                        tracking->Decode(evio_event_wrapper);
                        tracking -> find_tracks();
#else
                        std::vector<int> ivec{mod.bank, mod.crate};
                        gem_decoder.Decode(dbuf, buflen, ivec);
                        auto event = static_cast<GEMTreeStruct*>(mod.event);
                        extract_gem_cluster(&gem_system, &gem_decoder, *event, count);
#endif
                    }
                    break;
                default:
                    std::cout << "Unsupported module type " << mod.type << std::endl;
                    break;
                }
            }
            tree->Fill();
            count ++;
        }

    }
    std::cout << "Processed events - " << count << std::endl;

    evchan.Close();
#ifdef USE_GEM_TRACKING
    tracking -> End(9999);
    //tracking -> CalcEfficiency();
#endif
    hfile->Write();
    hfile->Close();
}

int GetRunNumber(std::string str)
{
    //extract the run number
    //std::string str = gFile->GetName();  //"../ROOTFILE/beamtest_hallc_3032.evio.xx

    std::string fileN="", file="";
    std::size_t found = str.find_last_of("/\\");
    std::string file0 = str.substr(found+1);     //take pure file name: "beamtest_hallc_3032.evio.xx"

    found = file0.find_last_of(".evio");
    std::string file1 = file0.substr(0,found);   //remove ".evio,xx": "beamtest_hallc_3032"

    //remove all characters from a to z and A to Z
    for(size_t i = 0; i<file1.length(); i++) {
        if (!(file1[i] >= 'a' && file1[i]<='z') && !(file1[i] >= 'A' && file1[i]<='Z')) {
            file.append(1, file1[i]);
        }
    }

    found = file.find_last_of("_");
    std::string file2 = file.substr(0,found);   //remove whatever after the last underscore: "beamtest_hallc_3032"
    fileN = file.substr(found+1);

    if(fileN.length()<4 && file2.length()>=4) {
        found = file2.find_last_of("_");
        std::string file3 = file2.substr(0,found);   //"beamtest_hallc_3032"
        fileN = file2.substr(found+1);    //"3032"
    }

    //std::cout<<" file0 = "<<file0<<"\n file1 = "<<file1<<"\n file = "<<file<<"\n file2 = "<<file2<<"\n fileN = "<<fileN<<std::endl;

    int pRunNumber = 0;
    if(fileN.length()>0) pRunNumber = atoi(fileN.c_str());
    std::cout<<"file = \""<<file0<<"\"  --> RunNumber = "<<pRunNumber<<std::endl;
    return pRunNumber;
}
