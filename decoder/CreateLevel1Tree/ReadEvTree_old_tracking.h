//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jun 30 11:41:55 2022 by ROOT version 6.22/02
//////////////////////////////////////////////////////////

#ifndef ReadEvTree_h
#define ReadEvTree_h

#include <TROOT.h>
#include <TSystem.h>
#include <TChain.h>
#include <TFile.h>

#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>

// Header file for the classes stored in the TTree if any.
#include "../include/Fadc250Data.h"

#define MAX_CLUSTER_SIZE 100     //maximum number of stripes in one cluster
#define MAX_CLUSTERS_PER_PLANE 2000

class ReadEvTree {
public :
	TTree          *fChain;   //!pointer to the analyzed TTree or TChain
	Int_t           fCurrent; //!current Tree number in a TChain
	Int_t           fRunNumber;

	// Fixed size dimensions of array or collections stored in the TTree if any.

	// Declaration of leaf types
	Int_t           event_number;
	Int_t           GEM_nCluster;
	Int_t           GEM_planeID[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_prodID[MAX_CLUSTERS_PER_PLANE];    //[GEM_nCluster]
	Int_t           GEM_moduleID[MAX_CLUSTERS_PER_PLANE];  //[GEM_nCluster]
	Int_t           GEM_axis[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_size[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Float_t         GEM_adc[MAX_CLUSTERS_PER_PLANE];    //[GEM_nCluster]
	Float_t         GEM_pos[MAX_CLUSTERS_PER_PLANE];    //[GEM_nCluster]
	Int_t           GEM_stripNo[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];   //[GEM_nCluster]
	Float_t         GEM_stripAdc[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];  //[GEM_nCluster]
	/*
	//vector<int>     *GEM_stripNo;
	//vector<float>   *GEM_stripAdc;
	vector<float>   *GEM_stripTs0;
	vector<float>   *GEM_stripTs1;
	vector<float>   *GEM_stripTs2;
	vector<float>   *GEM_stripTs3;
	vector<float>   *GEM_stripTs4;
	vector<float>   *GEM_stripTs5;
	*/
	Int_t           GEM_nAPV;
	Int_t           GEM_apv_crate_id[16]; //[GEM_nAPV]
	Int_t           GEM_apv_mpd_id[16];   //[GEM_nAPV]
	Int_t           GEM_apv_adc_ch[16];   //[GEM_nAPV]
	
	
	Int_t           fNtracks_found;
	vector<int>     *fNhitsOnTrack;
	vector<double>  *fXtrack;
	vector<double>  *fYtrack;
	vector<double>  *fXptrack;
	vector<double>  *fYptrack;
	vector<double>  *fChi2Track;
	Int_t           fBestTrackIndex;
	Int_t           fNgoodhits;
	vector<int>     *fHitTrackIndex;
	vector<int>     *fHitModule;
	vector<int>     *fHitLayer;
	vector<int>     *fHitNstripsU;
	vector<int>     *fHitUstripMax;
	vector<int>     *fHitUstripLo;
	vector<int>     *fHitUstripHi;
	vector<int>     *fHitNstripsV;
	vector<int>     *fHitVstripMax;
	vector<int>     *fHitVstripLo;
	vector<int>     *fHitVstripHi;
	vector<double>  *fHitUlocal;
	vector<double>  *fHitVlocal;
	vector<double>  *fHitXlocal;
	vector<double>  *fHitYlocal;
	vector<double>  *fHitXglobal;
	vector<double>  *fHitYglobal;
	vector<double>  *fHitZglobal;
	vector<double>  *fHitUmoment;
	vector<double>  *fHitVmoment;
	vector<double>  *fHitUsigma;
	vector<double>  *fHitVsigma;
	vector<double>  *fHitResidU;
	vector<double>  *fHitResidV;
	vector<double>  *fHitEResidU;
	vector<double>  *fHitEResidV;
	vector<double>  *fHitUADC;
	vector<double>  *fHitVADC;
	vector<double>  *fHitUADCclust_deconv;
	vector<double>  *fHitVADCclust_deconv;
	vector<double>  *fHitUADCclust_maxsamp_deconv;
	vector<double>  *fHitVADCclust_maxsamp_deconv;
	vector<double>  *fHitUADCclust_maxcombo_deconv;
	vector<double>  *fHitVADCclust_maxcombo_deconv;
	vector<double>  *fHitUADCmaxstrip;
	vector<double>  *fHitVADCmaxstrip;
	vector<double>  *fHitUADCmaxstrip_deconv;
	vector<double>  *fHitVADCmaxstrip_deconv;
	vector<double>  *fHitUADCmaxsample;
	vector<double>  *fHitVADCmaxsample;
	vector<double>  *fHitUADCmaxsample_deconv;
	vector<double>  *fHitVADCmaxsample_deconv;
	vector<double>  *fHitUADCmaxcombo_deconv;
	vector<double>  *fHitVADCmaxcombo_deconv;
	vector<double>  *fHitUADCmaxclustsample;
	vector<double>  *fHitVADCmaxclustsample;
	vector<double>  *fHitADCasym;
	vector<double>  *fHitADCavg;
	vector<double>  *fHitADCasym_deconv;
	vector<double>  *fHitADCavg_deconv;
	vector<double>  *fHitUTime;
	vector<double>  *fHitVTime;
	vector<double>  *fHitUTimeDeconv;
	vector<double>  *fHitVTimeDeconv;
	vector<double>  *fHitUTimeMaxStrip;
	vector<double>  *fHitVTimeMaxStrip;
	vector<double>  *fHitUTimeMaxStripFit;
	vector<double>  *fHitVTimeMaxStripFit;
	vector<double>  *fHitUTimeMaxStripDeconv;
	vector<double>  *fHitVTimeMaxStripDeconv;
	vector<double>  *fHitDeltaTDeconv;
	vector<double>  *fHitTavgDeconv;
	vector<double>  *fHitIsampMaxUclust;
	vector<double>  *fHitIsampMaxVclust;
	vector<double>  *fHitIsampMaxUstrip;
	vector<double>  *fHitIsampMaxVstrip;
	vector<double>  *fHitIsampMaxUstripDeconv;
	vector<double>  *fHitIsampMaxVstripDeconv;
	vector<double>  *fHitIcomboMaxUstripDeconv;
	vector<double>  *fHitIcomboMaxVstripDeconv;
	vector<double>  *fHitIsampMaxUclustDeconv;
	vector<double>  *fHitIsampMaxVclustDeconv;
	vector<double>  *fHitIcomboMaxUclustDeconv;
	vector<double>  *fHitIcomboMaxVclustDeconv;
	vector<double>  *fHitCorrCoeffClust;
	vector<double>  *fHitCorrCoeffMaxStrip;
	vector<double>  *fHitCorrCoeffClustDeconv;
	vector<double>  *fHitCorrCoeffMaxStripDeconv;
	vector<double>  *fHitADCfrac0_MaxUstrip;
	vector<double>  *fHitADCfrac1_MaxUstrip;
	vector<double>  *fHitADCfrac2_MaxUstrip;
	vector<double>  *fHitADCfrac3_MaxUstrip;
	vector<double>  *fHitADCfrac4_MaxUstrip;
	vector<double>  *fHitADCfrac5_MaxUstrip;
	vector<double>  *fHitADCfrac0_MaxVstrip;
	vector<double>  *fHitADCfrac1_MaxVstrip;
	vector<double>  *fHitADCfrac2_MaxVstrip;
	vector<double>  *fHitADCfrac3_MaxVstrip;
	vector<double>  *fHitADCfrac4_MaxVstrip;
	vector<double>  *fHitADCfrac5_MaxVstrip;
	vector<double>  *fHitDeconvADC0_MaxUstrip;
	vector<double>  *fHitDeconvADC1_MaxUstrip;
	vector<double>  *fHitDeconvADC2_MaxUstrip;
	vector<double>  *fHitDeconvADC3_MaxUstrip;
	vector<double>  *fHitDeconvADC4_MaxUstrip;
	vector<double>  *fHitDeconvADC5_MaxUstrip;
	vector<double>  *fHitDeconvADC0_MaxVstrip;
	vector<double>  *fHitDeconvADC1_MaxVstrip;
	vector<double>  *fHitDeconvADC2_MaxVstrip;
	vector<double>  *fHitDeconvADC3_MaxVstrip;
	vector<double>  *fHitDeconvADC4_MaxVstrip;
	vector<double>  *fHitDeconvADC5_MaxVstrip;
	
	fdec::Fadc250Data *CerA0;
	fdec::Fadc250Data *CerA1;
	fdec::Fadc250Data *CerA2;
	fdec::Fadc250Data *CerA3;
	fdec::Fadc250Data *CerB0;
	fdec::Fadc250Data *CerB1;
	fdec::Fadc250Data *CerB2;
	fdec::Fadc250Data *CerB3;
	fdec::Fadc250Data *CerC0;
	fdec::Fadc250Data *CerC1;
	fdec::Fadc250Data *CerC2;
	fdec::Fadc250Data *CerC3;
	fdec::Fadc250Data *CerD0;
	fdec::Fadc250Data *CerD1;
	fdec::Fadc250Data *CerD2;
	fdec::Fadc250Data *CerD3;

	fdec::Fadc250Data *SC_D;
	fdec::Fadc250Data *SC_A;
	fdec::Fadc250Data *SC_B;
	fdec::Fadc250Data *CerSum;
	fdec::Fadc250Data *LASPD_t;
	fdec::Fadc250Data *LASPD_b;
	fdec::Fadc250Data *SC_C;
	fdec::Fadc250Data *PreSh_l;
	fdec::Fadc250Data *PreSh_r;
	fdec::Fadc250Data *PreSh_t;
	fdec::Fadc250Data *Shower_l;
	fdec::Fadc250Data *Shower_r;
	fdec::Fadc250Data *Shower_t;
	fdec::Fadc250Data *PreShSum;
	fdec::Fadc250Data *ShowerSum;
	fdec::Fadc250Data *Trig;
	Int_t           trigger_type;
	ULong64_t       trigger_time;

	// List of branches
	TBranch        *b_event_number;   //!
	TBranch        *b_GEM_nCluster;   //!
	TBranch        *b_GEM_planeID;   //!
	TBranch        *b_GEM_prodID;   //!
	TBranch        *b_GEM_moduleID;   //!
	TBranch        *b_GEM_axis;   //!
	TBranch        *b_GEM_size;   //!
	TBranch        *b_GEM_adc;   //!
	TBranch        *b_GEM_pos;   //!
	TBranch        *b_GEM_stripNo;   //!
	TBranch        *b_GEM_stripAdc;   //!
	/*
	TBranch        *b_GEM_stripNo;   //!
	TBranch        *b_GEM_stripAdc;   //!
	TBranch        *b_GEM_stripTs0;   //!
	TBranch        *b_GEM_stripTs1;   //!
	TBranch        *b_GEM_stripTs2;   //!
	TBranch        *b_GEM_stripTs3;   //!
	TBranch        *b_GEM_stripTs4;   //!
	TBranch        *b_GEM_stripTs5;   //!
	*/
	TBranch        *b_GEM_nAPV;   //!
	TBranch        *b_GEM_apv_crate_id;   //!
	TBranch        *b_GEM_apv_mpd_id;   //!
	TBranch        *b_GEM_apv_adc_ch;   //!
	TBranch        *b_CerA0;   //!
	TBranch        *b_CerA1;   //!
	TBranch        *b_CerA2;   //!
	TBranch        *b_CerA3;   //!
	TBranch        *b_CerB0;   //!
	TBranch        *b_CerB1;   //!
	TBranch        *b_CerB2;   //!
	TBranch        *b_CerB3;   //!
	TBranch        *b_CerC0;   //!
	TBranch        *b_CerC1;   //!
	TBranch        *b_CerC2;   //!
	TBranch        *b_CerC3;   //!
	TBranch        *b_CerD0;   //!
	TBranch        *b_CerD1;   //!
	TBranch        *b_CerD2;   //!
	TBranch        *b_CerD3;   //!

	TBranch        *b_SC_D;   //!
	TBranch        *b_SC_A;   //!
	TBranch        *b_SC_B;   //!
	TBranch        *b_CerSum;   //!
	TBranch        *b_LASPD_t;   //!
	TBranch        *b_LASPD_b;   //!
	TBranch        *b_SC_C;   //!
	TBranch        *b_PreSh_l;   //!
	TBranch        *b_PreSh_r;   //!
	TBranch        *b_PreSh_t;   //!
	TBranch        *b_Shower_l;   //!
	TBranch        *b_Shower_r;   //!
	TBranch        *b_Shower_t;   //!
	TBranch        *b_PreShSum;   //!
	TBranch        *b_ShowerSum;   //!
	TBranch        *b_Trig;   //!
	TBranch        *b_fNtracks_found;   //!
	TBranch        *b_fNhitsOnTrack;   //!
	TBranch        *b_fXtrack;   //!
	TBranch        *b_fYtrack;   //!
	TBranch        *b_fXptrack;   //!
	TBranch        *b_fYptrack;   //!
	TBranch        *b_fChi2Track;   //!
	TBranch        *b_fBestTrackIndex;   //!
	TBranch        *b_fNgoodhits;   //!
	TBranch        *b_fHitTrackIndex;   //!
	TBranch        *b_fHitModule;   //!
	TBranch        *b_fHitLayer;   //!
	TBranch        *b_fHitNstripsU;   //!
	TBranch        *b_fHitUstripMax;   //!
	TBranch        *b_fHitUstripLo;   //!
	TBranch        *b_fHitUstripHi;   //!
	TBranch        *b_fHitNstripsV;   //!
	TBranch        *b_fHitVstripMax;   //!
	TBranch        *b_fHitVstripLo;   //!
	TBranch        *b_fHitVstripHi;   //!
	TBranch        *b_fHitUlocal;   //!
	TBranch        *b_fHitVlocal;   //!
	TBranch        *b_fHitXlocal;   //!
	TBranch        *b_fHitYlocal;   //!
	TBranch        *b_fHitXglobal;   //!
	TBranch        *b_fHitYglobal;   //!
	TBranch        *b_fHitZglobal;   //!
	TBranch        *b_fHitUmoment;   //!
	TBranch        *b_fHitVmoment;   //!
	TBranch        *b_fHitUsigma;   //!
	TBranch        *b_fHitVsigma;   //!
	TBranch        *b_fHitResidU;   //!
	TBranch        *b_fHitResidV;   //!
	TBranch        *b_fHitEResidU;   //!
	TBranch        *b_fHitEResidV;   //!
	TBranch        *b_fHitUADC;   //!
	TBranch        *b_fHitVADC;   //!
	TBranch        *b_fHitUADCclust_deconv;   //!
	TBranch        *b_fHitVADCclust_deconv;   //!
	TBranch        *b_fHitUADCclust_maxsamp_deconv;   //!
	TBranch        *b_fHitVADCclust_maxsamp_deconv;   //!
	TBranch        *b_fHitUADCclust_maxcombo_deconv;   //!
	TBranch        *b_fHitVADCclust_maxcombo_deconv;   //!
	TBranch        *b_fHitUADCmaxstrip;   //!
	TBranch        *b_fHitVADCmaxstrip;   //!
	TBranch        *b_fHitUADCmaxstrip_deconv;   //!
	TBranch        *b_fHitVADCmaxstrip_deconv;   //!
	TBranch        *b_fHitUADCmaxsample;   //!
	TBranch        *b_fHitVADCmaxsample;   //!
	TBranch        *b_fHitUADCmaxsample_deconv;   //!
	TBranch        *b_fHitVADCmaxsample_deconv;   //!
	TBranch        *b_fHitUADCmaxcombo_deconv;   //!
	TBranch        *b_fHitVADCmaxcombo_deconv;   //!
	TBranch        *b_fHitUADCmaxclustsample;   //!
	TBranch        *b_fHitVADCmaxclustsample;   //!
	TBranch        *b_fHitADCasym;   //!
	TBranch        *b_fHitADCavg;   //!
	TBranch        *b_fHitADCasym_deconv;   //!
	TBranch        *b_fHitADCavg_deconv;   //!
	TBranch        *b_fHitUTime;   //!
	TBranch        *b_fHitVTime;   //!
	TBranch        *b_fHitUTimeDeconv;   //!
	TBranch        *b_fHitVTimeDeconv;   //!
	TBranch        *b_fHitUTimeMaxStrip;   //!
	TBranch        *b_fHitVTimeMaxStrip;   //!
	TBranch        *b_fHitUTimeMaxStripFit;   //!
	TBranch        *b_fHitVTimeMaxStripFit;   //!
	TBranch        *b_fHitUTimeMaxStripDeconv;   //!
	TBranch        *b_fHitVTimeMaxStripDeconv;   //!
	TBranch        *b_fHitDeltaTDeconv;   //!
	TBranch        *b_fHitTavgDeconv;   //!
	TBranch        *b_fHitIsampMaxUclust;   //!
	TBranch        *b_fHitIsampMaxVclust;   //!
	TBranch        *b_fHitIsampMaxUstrip;   //!
	TBranch        *b_fHitIsampMaxVstrip;   //!
	TBranch        *b_fHitIsampMaxUstripDeconv;   //!
	TBranch        *b_fHitIsampMaxVstripDeconv;   //!
	TBranch        *b_fHitIcomboMaxUstripDeconv;   //!
	TBranch        *b_fHitIcomboMaxVstripDeconv;   //!
	TBranch        *b_fHitIsampMaxUclustDeconv;   //!
	TBranch        *b_fHitIsampMaxVclustDeconv;   //!
	TBranch        *b_fHitIcomboMaxUclustDeconv;   //!
	TBranch        *b_fHitIcomboMaxVclustDeconv;   //!
	TBranch        *b_fHitCorrCoeffClust;   //!
	TBranch        *b_fHitCorrCoeffMaxStrip;   //!
	TBranch        *b_fHitCorrCoeffClustDeconv;   //!
	TBranch        *b_fHitCorrCoeffMaxStripDeconv;   //!
	TBranch        *b_fHitADCfrac0_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac1_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac2_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac3_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac4_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac5_MaxUstrip;   //!
	TBranch        *b_fHitADCfrac0_MaxVstrip;   //!
	TBranch        *b_fHitADCfrac1_MaxVstrip;   //!
	TBranch        *b_fHitADCfrac2_MaxVstrip;   //!
	TBranch        *b_fHitADCfrac3_MaxVstrip;   //!
	TBranch        *b_fHitADCfrac4_MaxVstrip;   //!
	TBranch        *b_fHitADCfrac5_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC0_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC1_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC2_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC3_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC4_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC5_MaxUstrip;   //!
	TBranch        *b_fHitDeconvADC0_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC1_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC2_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC3_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC4_MaxVstrip;   //!
	TBranch        *b_fHitDeconvADC5_MaxVstrip;   //!
	TBranch        *b_trigger_type;   //!
	TBranch        *b_trigger_time;   //!


	const char *Slot7Name[16]= {
		"CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3",
		"CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };

	const char *Slot8Name[16]= {
		"SC_D", "SC_A", "SC_B", "CerSum", "LASPD_t", "LASPD_b", "SC_C", "PreSh_l",
		"PreSh_r", "PreSh_t", "Shower_l", "Shower_r", "Shower_t", "PreShSum", "ShowerSum", "Trig" };


	fdec::Fadc250Data *Slot7Data[16];
	fdec::Fadc250Data *Slot8Data[16];


	ReadEvTree(TTree *tree=0);
	ReadEvTree(const char *filename);

	virtual ~ReadEvTree();
	virtual Int_t    Cut(Long64_t entry);
	virtual Int_t    GetEntry(Long64_t entry);
	virtual Long64_t LoadTree(Long64_t entry);
	virtual void     Init(TTree *tree);
	virtual void     Loop(Long64_t istart=0, Long64_t iend=-1);
	virtual Bool_t   Notify();
	virtual void     Show(Long64_t entry = -1);

	virtual bool     ExtractPedestal(Long64_t istart=0, Long64_t iend=100000);
	virtual void     PlotSpectrum(Long64_t istart=0, Long64_t iend=-1);
	virtual void     PlotEvent(Long64_t entry);
	virtual int      GetRunNumber(const char* filename="");
};

#endif

#ifdef ReadEvTree_cxx
ReadEvTree::ReadEvTree(TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.

	if (tree == 0) {
		if(gROOT->FindObject("EvTree")) tree = (TTree*)gROOT->FindObject("EvTree");
	}
	if (tree == 0) {
		TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("../ROOTFILE/beamtest_hallc_4126_0.root");
		if (!f || !f->IsOpen()) {
			f = new TFile("../ROOTFILE/beamtest_hallc_4126_0.root");
		}
		f->GetObject("EvTree",tree);
	}

	Init(tree);
}

ReadEvTree::ReadEvTree(const char *filename)
{
	fChain=0;

	TFile *f = new TFile(filename);
	TTree *tree = 0;
	f->GetObject("EvTree",tree);

	Init(tree);
}

ReadEvTree::~ReadEvTree()
{
	if (!fChain) return;
	delete fChain->GetCurrentFile();
}

int ReadEvTree::GetRunNumber(const char* infile)
{
	//extract the run number
	std::string str = infile;
	if(strlen(infile)<5)  str = gFile->GetName();  //"../ROOTFILE/beamtest_hallc_3032_1.root"

	std::string fileN="", file="";
	std::size_t found = str.find_last_of("/\\");
	std::string file0 = str.substr(found+1);     //take pure file name: "beamtest_hallc_3032_1.root"

	found = file0.find_last_of(".");
	std::string file1 = file0.substr(0,found);   //remove ".root": "beamtest_hallc_3032_1"
	
	found = file1.find("_debug");
	if(found != std::string::npos) file1 = file1.substr(0,found);   //remove "_debug*" from "beamtest_hallc_3032_1_debug*"

	//remove all characters from ato z and A to Z
	for(size_t i = 0; i<file1.length(); i++) {
		if (!(file1[i] >= 'a' && file1[i]<='z') && !(file1[i] >= 'A' && file1[i]<='Z')) {
			file.append(1, file1[i]);
		}
	}

	found = file.find_last_of("_");
	std::string file2 = file.substr(0,found);   //remove whatever after the last underscore: "beamtest_hallc_3032"
	fileN = file.substr(found+1);

	if(file2.length()>=4) {
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

Int_t ReadEvTree::GetEntry(Long64_t entry)
{
	// Read contents of entry.
	if (!fChain) return 0;

	// Reset object pointer
	for(int i=0;i<16;i++) {
		if(Slot7Data[i]) Slot7Data[i]->Clear();
		if(Slot8Data[i]) Slot8Data[i]->Clear();
	}
	event_number=trigger_type=-1;
	return fChain->GetEntry(entry);
}

Long64_t ReadEvTree::LoadTree(Long64_t entry)
{
	// Set the environment to read one entry
	if (!fChain) return -5;
	Long64_t centry = fChain->LoadTree(entry);
	if (centry < 0) return centry;
	if (fChain->GetTreeNumber() != fCurrent) {
		fCurrent = fChain->GetTreeNumber();
		Notify();
	}
	return centry;
}

void ReadEvTree::Init(TTree *tree)
{
	// The Init() function is called when the selector needs to initialize
	// a new tree or chain. Typically here the branch addresses and branch
	// pointers of the tree will be set.
	// It is normally not necessary to make changes to the generated
	// code, but the routine can be extended by the user if needed.
	// Init() will be called many times when running on PROOF
	// (once per file to be processed).

	fRunNumber = 0;

	// Set object pointer
	/*
	GEM_stripNo = 0;
	GEM_stripAdc = 0;
	GEM_stripTs0 = 0;
	GEM_stripTs1 = 0;
	GEM_stripTs2 = 0;
	GEM_stripTs3 = 0;
	GEM_stripTs4 = 0;
	GEM_stripTs5 = 0;
	*/
	CerA0 = 0;
	CerA1 = 0;
	CerA2 = 0;
	CerA3 = 0;
	CerB0 = 0;
	CerB1 = 0;
	CerB2 = 0;
	CerB3 = 0;
	CerC0 = 0;
	CerC1 = 0;
	CerC2 = 0;
	CerC3 = 0;
	CerD0 = 0;
	CerD1 = 0;
	CerD2 = 0;
	CerD3 = 0;

	SC_D = 0;
	SC_A = 0;
	SC_B = 0;
	CerSum = 0;
	LASPD_t = 0;
	LASPD_b = 0;
	SC_C = 0;
	PreSh_r = 0;
	PreSh_l = 0;
	PreSh_t = 0;
	Shower_r = 0;
	Shower_l = 0;
	Shower_t = 0;
	PreShSum = 0;
	ShowerSum = 0;
	Trig = 0;
	
	fNhitsOnTrack = 0;
	fXtrack = 0;
	fYtrack = 0;
	fXptrack = 0;
	fYptrack = 0;
	fChi2Track = 0;
	fHitTrackIndex = 0;
	fHitModule = 0;
	fHitLayer = 0;
	fHitNstripsU = 0;
	fHitUstripMax = 0;
	fHitUstripLo = 0;
	fHitUstripHi = 0;
	fHitNstripsV = 0;
	fHitVstripMax = 0;
	fHitVstripLo = 0;
	fHitVstripHi = 0;
	fHitUlocal = 0;
	fHitVlocal = 0;
	fHitXlocal = 0;
	fHitYlocal = 0;
	fHitXglobal = 0;
	fHitYglobal = 0;
	fHitZglobal = 0;
	fHitUmoment = 0;
	fHitVmoment = 0;
	fHitUsigma = 0;
	fHitVsigma = 0;
	fHitResidU = 0;
	fHitResidV = 0;
	fHitEResidU = 0;
	fHitEResidV = 0;
	fHitUADC = 0;
	fHitVADC = 0;
	fHitUADCclust_deconv = 0;
	fHitVADCclust_deconv = 0;
	fHitUADCclust_maxsamp_deconv = 0;
	fHitVADCclust_maxsamp_deconv = 0;
	fHitUADCclust_maxcombo_deconv = 0;
	fHitVADCclust_maxcombo_deconv = 0;
	fHitUADCmaxstrip = 0;
	fHitVADCmaxstrip = 0;
	fHitUADCmaxstrip_deconv = 0;
	fHitVADCmaxstrip_deconv = 0;
	fHitUADCmaxsample = 0;
	fHitVADCmaxsample = 0;
	fHitUADCmaxsample_deconv = 0;
	fHitVADCmaxsample_deconv = 0;
	fHitUADCmaxcombo_deconv = 0;
	fHitVADCmaxcombo_deconv = 0;
	fHitUADCmaxclustsample = 0;
	fHitVADCmaxclustsample = 0;
	fHitADCasym = 0;
	fHitADCavg = 0;
	fHitADCasym_deconv = 0;
	fHitADCavg_deconv = 0;
	fHitUTime = 0;
	fHitVTime = 0;
	fHitUTimeDeconv = 0;
	fHitVTimeDeconv = 0;
	fHitUTimeMaxStrip = 0;
	fHitVTimeMaxStrip = 0;
	fHitUTimeMaxStripFit = 0;
	fHitVTimeMaxStripFit = 0;
	fHitUTimeMaxStripDeconv = 0;
	fHitVTimeMaxStripDeconv = 0;
	fHitDeltaTDeconv = 0;
	fHitTavgDeconv = 0;
	fHitIsampMaxUclust = 0;
	fHitIsampMaxVclust = 0;
	fHitIsampMaxUstrip = 0;
	fHitIsampMaxVstrip = 0;
	fHitIsampMaxUstripDeconv = 0;
	fHitIsampMaxVstripDeconv = 0;
	fHitIcomboMaxUstripDeconv = 0;
	fHitIcomboMaxVstripDeconv = 0;
	fHitIsampMaxUclustDeconv = 0;
	fHitIsampMaxVclustDeconv = 0;
	fHitIcomboMaxUclustDeconv = 0;
	fHitIcomboMaxVclustDeconv = 0;
	fHitCorrCoeffClust = 0;
	fHitCorrCoeffMaxStrip = 0;
	fHitCorrCoeffClustDeconv = 0;
	fHitCorrCoeffMaxStripDeconv = 0;
	fHitADCfrac0_MaxUstrip = 0;
	fHitADCfrac1_MaxUstrip = 0;
	fHitADCfrac2_MaxUstrip = 0;
	fHitADCfrac3_MaxUstrip = 0;
	fHitADCfrac4_MaxUstrip = 0;
	fHitADCfrac5_MaxUstrip = 0;
	fHitADCfrac0_MaxVstrip = 0;
	fHitADCfrac1_MaxVstrip = 0;
	fHitADCfrac2_MaxVstrip = 0;
	fHitADCfrac3_MaxVstrip = 0;
	fHitADCfrac4_MaxVstrip = 0;
	fHitADCfrac5_MaxVstrip = 0;
	fHitDeconvADC0_MaxUstrip = 0;
	fHitDeconvADC1_MaxUstrip = 0;
	fHitDeconvADC2_MaxUstrip = 0;
	fHitDeconvADC3_MaxUstrip = 0;
	fHitDeconvADC4_MaxUstrip = 0;
	fHitDeconvADC5_MaxUstrip = 0;
	fHitDeconvADC0_MaxVstrip = 0;
	fHitDeconvADC1_MaxVstrip = 0;
	fHitDeconvADC2_MaxVstrip = 0;
	fHitDeconvADC3_MaxVstrip = 0;
	fHitDeconvADC4_MaxVstrip = 0;
	fHitDeconvADC5_MaxVstrip = 0;
	// Set branch addresses and branch pointers
	if (!tree) return;
	fChain = tree;
	fCurrent = -1;
	fChain->SetMakeClass(1);
	if (fChain->FindBranch("GEM_nCluster")) {
		fChain->SetBranchAddress("event_number", &event_number, &b_event_number);
		fChain->SetBranchAddress("GEM_nCluster", &GEM_nCluster, &b_GEM_nCluster);
		fChain->SetBranchAddress("GEM_planeID", GEM_planeID, &b_GEM_planeID);
		fChain->SetBranchAddress("GEM_prodID", GEM_prodID, &b_GEM_prodID);
		fChain->SetBranchAddress("GEM_moduleID", GEM_moduleID, &b_GEM_moduleID);
		fChain->SetBranchAddress("GEM_axis", GEM_axis, &b_GEM_axis);
		fChain->SetBranchAddress("GEM_size", GEM_size, &b_GEM_size);
		fChain->SetBranchAddress("GEM_adc", GEM_adc, &b_GEM_adc);
		fChain->SetBranchAddress("GEM_pos", GEM_pos, &b_GEM_pos);
		fChain->SetBranchAddress("GEM_stripNo", GEM_stripNo, &b_GEM_stripNo);
		fChain->SetBranchAddress("GEM_stripAdc", GEM_stripAdc, &b_GEM_stripAdc);
		/*
		if (fChain->FindBranch("GEM_stripTs0")) {
			fChain->SetBranchAddress("GEM_stripTs0", &GEM_stripTs0, &b_GEM_stripTs0);
			fChain->SetBranchAddress("GEM_stripTs1", &GEM_stripTs1, &b_GEM_stripTs1);
			fChain->SetBranchAddress("GEM_stripTs2", &GEM_stripTs2, &b_GEM_stripTs2);
			fChain->SetBranchAddress("GEM_stripTs3", &GEM_stripTs3, &b_GEM_stripTs3);
			fChain->SetBranchAddress("GEM_stripTs4", &GEM_stripTs4, &b_GEM_stripTs4);
			fChain->SetBranchAddress("GEM_stripTs5", &GEM_stripTs5, &b_GEM_stripTs5);
		}
		*/
		fChain->SetBranchAddress("GEM_nAPV", &GEM_nAPV, &b_GEM_nAPV);
		fChain->SetBranchAddress("GEM_apv_crate_id", GEM_apv_crate_id, &b_GEM_apv_crate_id);
		fChain->SetBranchAddress("GEM_apv_mpd_id", GEM_apv_mpd_id, &b_GEM_apv_mpd_id);
		fChain->SetBranchAddress("GEM_apv_adc_ch", GEM_apv_adc_ch, &b_GEM_apv_adc_ch);
	}

	fChain->SetBranchAddress("CerA0", &CerA0, &b_CerA0);
	fChain->SetBranchAddress("CerA1", &CerA1, &b_CerA1);
	fChain->SetBranchAddress("CerA2", &CerA2, &b_CerA2);
	fChain->SetBranchAddress("CerA3", &CerA3, &b_CerA3);
	fChain->SetBranchAddress("CerB0", &CerB0, &b_CerB0);
	fChain->SetBranchAddress("CerB1", &CerB1, &b_CerB1);
	fChain->SetBranchAddress("CerB2", &CerB2, &b_CerB2);
	fChain->SetBranchAddress("CerB3", &CerB3, &b_CerB3);
	fChain->SetBranchAddress("CerC0", &CerC0, &b_CerC0);
	fChain->SetBranchAddress("CerC1", &CerC1, &b_CerC1);
	fChain->SetBranchAddress("CerC2", &CerC2, &b_CerC2);
	fChain->SetBranchAddress("CerC3", &CerC3, &b_CerC3);
	fChain->SetBranchAddress("CerD0", &CerD0, &b_CerD0);
	fChain->SetBranchAddress("CerD1", &CerD1, &b_CerD1);
	fChain->SetBranchAddress("CerD2", &CerD2, &b_CerD2);
	fChain->SetBranchAddress("CerD3", &CerD3, &b_CerD3);

	fChain->SetBranchAddress("SC_D", &SC_D, &b_SC_D);
	fChain->SetBranchAddress("SC_A", &SC_A, &b_SC_A);
	fChain->SetBranchAddress("SC_B", &SC_B, &b_SC_B);
	fChain->SetBranchAddress("CerSum", &CerSum, &b_CerSum);
	fChain->SetBranchAddress("LASPD_t", &LASPD_t, &b_LASPD_t);
	fChain->SetBranchAddress("LASPD_b", &LASPD_b, &b_LASPD_b);
	fChain->SetBranchAddress("SC_C", &SC_C, &b_SC_C);
	fChain->SetBranchAddress("PreSh_l", &PreSh_l, &b_PreSh_l);
	fChain->SetBranchAddress("PreSh_r", &PreSh_r, &b_PreSh_r);
	fChain->SetBranchAddress("PreSh_t", &PreSh_t, &b_PreSh_t);
	fChain->SetBranchAddress("Shower_l", &Shower_l, &b_Shower_l);
	fChain->SetBranchAddress("Shower_r", &Shower_r, &b_Shower_r);
	fChain->SetBranchAddress("Shower_t", &Shower_t, &b_Shower_t);
	fChain->SetBranchAddress("PreShSum", &PreShSum, &b_PreShSum);
	fChain->SetBranchAddress("ShowerSum", &ShowerSum, &b_ShowerSum);
	fChain->SetBranchAddress("Trig", &Trig, &b_Trig);
	fChain->SetBranchAddress("fNtracks_found", &fNtracks_found, &b_fNtracks_found);
	fChain->SetBranchAddress("fNhitsOnTrack", &fNhitsOnTrack, &b_fNhitsOnTrack);
	fChain->SetBranchAddress("fXtrack", &fXtrack, &b_fXtrack);
	fChain->SetBranchAddress("fYtrack", &fYtrack, &b_fYtrack);
	fChain->SetBranchAddress("fXptrack", &fXptrack, &b_fXptrack);
	fChain->SetBranchAddress("fYptrack", &fYptrack, &b_fYptrack);
	fChain->SetBranchAddress("fChi2Track", &fChi2Track, &b_fChi2Track);
	fChain->SetBranchAddress("fBestTrackIndex", &fBestTrackIndex, &b_fBestTrackIndex);
	fChain->SetBranchAddress("fNgoodhits", &fNgoodhits, &b_fNgoodhits);
	fChain->SetBranchAddress("fHitTrackIndex", &fHitTrackIndex, &b_fHitTrackIndex);
	fChain->SetBranchAddress("fHitModule", &fHitModule, &b_fHitModule);
	fChain->SetBranchAddress("fHitLayer", &fHitLayer, &b_fHitLayer);
	fChain->SetBranchAddress("fHitNstripsU", &fHitNstripsU, &b_fHitNstripsU);
	fChain->SetBranchAddress("fHitUstripMax", &fHitUstripMax, &b_fHitUstripMax);
	fChain->SetBranchAddress("fHitUstripLo", &fHitUstripLo, &b_fHitUstripLo);
	fChain->SetBranchAddress("fHitUstripHi", &fHitUstripHi, &b_fHitUstripHi);
	fChain->SetBranchAddress("fHitNstripsV", &fHitNstripsV, &b_fHitNstripsV);
	fChain->SetBranchAddress("fHitVstripMax", &fHitVstripMax, &b_fHitVstripMax);
	fChain->SetBranchAddress("fHitVstripLo", &fHitVstripLo, &b_fHitVstripLo);
	fChain->SetBranchAddress("fHitVstripHi", &fHitVstripHi, &b_fHitVstripHi);
	fChain->SetBranchAddress("fHitUlocal", &fHitUlocal, &b_fHitUlocal);
	fChain->SetBranchAddress("fHitVlocal", &fHitVlocal, &b_fHitVlocal);
	fChain->SetBranchAddress("fHitXlocal", &fHitXlocal, &b_fHitXlocal);
	fChain->SetBranchAddress("fHitYlocal", &fHitYlocal, &b_fHitYlocal);
	fChain->SetBranchAddress("fHitXglobal", &fHitXglobal, &b_fHitXglobal);
	fChain->SetBranchAddress("fHitYglobal", &fHitYglobal, &b_fHitYglobal);
	fChain->SetBranchAddress("fHitZglobal", &fHitZglobal, &b_fHitZglobal);
	fChain->SetBranchAddress("fHitUmoment", &fHitUmoment, &b_fHitUmoment);
	fChain->SetBranchAddress("fHitVmoment", &fHitVmoment, &b_fHitVmoment);
	fChain->SetBranchAddress("fHitUsigma", &fHitUsigma, &b_fHitUsigma);
	fChain->SetBranchAddress("fHitVsigma", &fHitVsigma, &b_fHitVsigma);
	fChain->SetBranchAddress("fHitResidU", &fHitResidU, &b_fHitResidU);
	fChain->SetBranchAddress("fHitResidV", &fHitResidV, &b_fHitResidV);
	fChain->SetBranchAddress("fHitEResidU", &fHitEResidU, &b_fHitEResidU);
	fChain->SetBranchAddress("fHitEResidV", &fHitEResidV, &b_fHitEResidV);
	fChain->SetBranchAddress("fHitUADC", &fHitUADC, &b_fHitUADC);
	fChain->SetBranchAddress("fHitVADC", &fHitVADC, &b_fHitVADC);
	fChain->SetBranchAddress("fHitUADCclust_deconv", &fHitUADCclust_deconv, &b_fHitUADCclust_deconv);
	fChain->SetBranchAddress("fHitVADCclust_deconv", &fHitVADCclust_deconv, &b_fHitVADCclust_deconv);
	fChain->SetBranchAddress("fHitUADCclust_maxsamp_deconv", &fHitUADCclust_maxsamp_deconv, &b_fHitUADCclust_maxsamp_deconv);
	fChain->SetBranchAddress("fHitVADCclust_maxsamp_deconv", &fHitVADCclust_maxsamp_deconv, &b_fHitVADCclust_maxsamp_deconv);
	fChain->SetBranchAddress("fHitUADCclust_maxcombo_deconv", &fHitUADCclust_maxcombo_deconv, &b_fHitUADCclust_maxcombo_deconv);
	fChain->SetBranchAddress("fHitVADCclust_maxcombo_deconv", &fHitVADCclust_maxcombo_deconv, &b_fHitVADCclust_maxcombo_deconv);
	fChain->SetBranchAddress("fHitUADCmaxstrip", &fHitUADCmaxstrip, &b_fHitUADCmaxstrip);
	fChain->SetBranchAddress("fHitVADCmaxstrip", &fHitVADCmaxstrip, &b_fHitVADCmaxstrip);
	fChain->SetBranchAddress("fHitUADCmaxstrip_deconv", &fHitUADCmaxstrip_deconv, &b_fHitUADCmaxstrip_deconv);
	fChain->SetBranchAddress("fHitVADCmaxstrip_deconv", &fHitVADCmaxstrip_deconv, &b_fHitVADCmaxstrip_deconv);
	fChain->SetBranchAddress("fHitUADCmaxsample", &fHitUADCmaxsample, &b_fHitUADCmaxsample);
	fChain->SetBranchAddress("fHitVADCmaxsample", &fHitVADCmaxsample, &b_fHitVADCmaxsample);
	fChain->SetBranchAddress("fHitUADCmaxsample_deconv", &fHitUADCmaxsample_deconv, &b_fHitUADCmaxsample_deconv);
	fChain->SetBranchAddress("fHitVADCmaxsample_deconv", &fHitVADCmaxsample_deconv, &b_fHitVADCmaxsample_deconv);
	fChain->SetBranchAddress("fHitUADCmaxcombo_deconv", &fHitUADCmaxcombo_deconv, &b_fHitUADCmaxcombo_deconv);
	fChain->SetBranchAddress("fHitVADCmaxcombo_deconv", &fHitVADCmaxcombo_deconv, &b_fHitVADCmaxcombo_deconv);
	fChain->SetBranchAddress("fHitUADCmaxclustsample", &fHitUADCmaxclustsample, &b_fHitUADCmaxclustsample);
	fChain->SetBranchAddress("fHitVADCmaxclustsample", &fHitVADCmaxclustsample, &b_fHitVADCmaxclustsample);
	fChain->SetBranchAddress("fHitADCasym", &fHitADCasym, &b_fHitADCasym);
	fChain->SetBranchAddress("fHitADCavg", &fHitADCavg, &b_fHitADCavg);
	fChain->SetBranchAddress("fHitADCasym_deconv", &fHitADCasym_deconv, &b_fHitADCasym_deconv);
	fChain->SetBranchAddress("fHitADCavg_deconv", &fHitADCavg_deconv, &b_fHitADCavg_deconv);
	fChain->SetBranchAddress("fHitUTime", &fHitUTime, &b_fHitUTime);
	fChain->SetBranchAddress("fHitVTime", &fHitVTime, &b_fHitVTime);
	fChain->SetBranchAddress("fHitUTimeDeconv", &fHitUTimeDeconv, &b_fHitUTimeDeconv);
	fChain->SetBranchAddress("fHitVTimeDeconv", &fHitVTimeDeconv, &b_fHitVTimeDeconv);
	fChain->SetBranchAddress("fHitUTimeMaxStrip", &fHitUTimeMaxStrip, &b_fHitUTimeMaxStrip);
	fChain->SetBranchAddress("fHitVTimeMaxStrip", &fHitVTimeMaxStrip, &b_fHitVTimeMaxStrip);
	fChain->SetBranchAddress("fHitUTimeMaxStripFit", &fHitUTimeMaxStripFit, &b_fHitUTimeMaxStripFit);
	fChain->SetBranchAddress("fHitVTimeMaxStripFit", &fHitVTimeMaxStripFit, &b_fHitVTimeMaxStripFit);
	fChain->SetBranchAddress("fHitUTimeMaxStripDeconv", &fHitUTimeMaxStripDeconv, &b_fHitUTimeMaxStripDeconv);
	fChain->SetBranchAddress("fHitVTimeMaxStripDeconv", &fHitVTimeMaxStripDeconv, &b_fHitVTimeMaxStripDeconv);
	fChain->SetBranchAddress("fHitDeltaTDeconv", &fHitDeltaTDeconv, &b_fHitDeltaTDeconv);
	fChain->SetBranchAddress("fHitTavgDeconv", &fHitTavgDeconv, &b_fHitTavgDeconv);
	fChain->SetBranchAddress("fHitIsampMaxUclust", &fHitIsampMaxUclust, &b_fHitIsampMaxUclust);
	fChain->SetBranchAddress("fHitIsampMaxVclust", &fHitIsampMaxVclust, &b_fHitIsampMaxVclust);
	fChain->SetBranchAddress("fHitIsampMaxUstrip", &fHitIsampMaxUstrip, &b_fHitIsampMaxUstrip);
	fChain->SetBranchAddress("fHitIsampMaxVstrip", &fHitIsampMaxVstrip, &b_fHitIsampMaxVstrip);
	fChain->SetBranchAddress("fHitIsampMaxUstripDeconv", &fHitIsampMaxUstripDeconv, &b_fHitIsampMaxUstripDeconv);
	fChain->SetBranchAddress("fHitIsampMaxVstripDeconv", &fHitIsampMaxVstripDeconv, &b_fHitIsampMaxVstripDeconv);
	fChain->SetBranchAddress("fHitIcomboMaxUstripDeconv", &fHitIcomboMaxUstripDeconv, &b_fHitIcomboMaxUstripDeconv);
	fChain->SetBranchAddress("fHitIcomboMaxVstripDeconv", &fHitIcomboMaxVstripDeconv, &b_fHitIcomboMaxVstripDeconv);
	fChain->SetBranchAddress("fHitIsampMaxUclustDeconv", &fHitIsampMaxUclustDeconv, &b_fHitIsampMaxUclustDeconv);
	fChain->SetBranchAddress("fHitIsampMaxVclustDeconv", &fHitIsampMaxVclustDeconv, &b_fHitIsampMaxVclustDeconv);
	fChain->SetBranchAddress("fHitIcomboMaxUclustDeconv", &fHitIcomboMaxUclustDeconv, &b_fHitIcomboMaxUclustDeconv);
	fChain->SetBranchAddress("fHitIcomboMaxVclustDeconv", &fHitIcomboMaxVclustDeconv, &b_fHitIcomboMaxVclustDeconv);
	fChain->SetBranchAddress("fHitCorrCoeffClust", &fHitCorrCoeffClust, &b_fHitCorrCoeffClust);
	fChain->SetBranchAddress("fHitCorrCoeffMaxStrip", &fHitCorrCoeffMaxStrip, &b_fHitCorrCoeffMaxStrip);
	fChain->SetBranchAddress("fHitCorrCoeffClustDeconv", &fHitCorrCoeffClustDeconv, &b_fHitCorrCoeffClustDeconv);
	fChain->SetBranchAddress("fHitCorrCoeffMaxStripDeconv", &fHitCorrCoeffMaxStripDeconv, &b_fHitCorrCoeffMaxStripDeconv);
	fChain->SetBranchAddress("fHitADCfrac0_MaxUstrip", &fHitADCfrac0_MaxUstrip, &b_fHitADCfrac0_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac1_MaxUstrip", &fHitADCfrac1_MaxUstrip, &b_fHitADCfrac1_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac2_MaxUstrip", &fHitADCfrac2_MaxUstrip, &b_fHitADCfrac2_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac3_MaxUstrip", &fHitADCfrac3_MaxUstrip, &b_fHitADCfrac3_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac4_MaxUstrip", &fHitADCfrac4_MaxUstrip, &b_fHitADCfrac4_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac5_MaxUstrip", &fHitADCfrac5_MaxUstrip, &b_fHitADCfrac5_MaxUstrip);
	fChain->SetBranchAddress("fHitADCfrac0_MaxVstrip", &fHitADCfrac0_MaxVstrip, &b_fHitADCfrac0_MaxVstrip);
	fChain->SetBranchAddress("fHitADCfrac1_MaxVstrip", &fHitADCfrac1_MaxVstrip, &b_fHitADCfrac1_MaxVstrip);
	fChain->SetBranchAddress("fHitADCfrac2_MaxVstrip", &fHitADCfrac2_MaxVstrip, &b_fHitADCfrac2_MaxVstrip);
	fChain->SetBranchAddress("fHitADCfrac3_MaxVstrip", &fHitADCfrac3_MaxVstrip, &b_fHitADCfrac3_MaxVstrip);
	fChain->SetBranchAddress("fHitADCfrac4_MaxVstrip", &fHitADCfrac4_MaxVstrip, &b_fHitADCfrac4_MaxVstrip);
	fChain->SetBranchAddress("fHitADCfrac5_MaxVstrip", &fHitADCfrac5_MaxVstrip, &b_fHitADCfrac5_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC0_MaxUstrip", &fHitDeconvADC0_MaxUstrip, &b_fHitDeconvADC0_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC1_MaxUstrip", &fHitDeconvADC1_MaxUstrip, &b_fHitDeconvADC1_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC2_MaxUstrip", &fHitDeconvADC2_MaxUstrip, &b_fHitDeconvADC2_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC3_MaxUstrip", &fHitDeconvADC3_MaxUstrip, &b_fHitDeconvADC3_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC4_MaxUstrip", &fHitDeconvADC4_MaxUstrip, &b_fHitDeconvADC4_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC5_MaxUstrip", &fHitDeconvADC5_MaxUstrip, &b_fHitDeconvADC5_MaxUstrip);
	fChain->SetBranchAddress("fHitDeconvADC0_MaxVstrip", &fHitDeconvADC0_MaxVstrip, &b_fHitDeconvADC0_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC1_MaxVstrip", &fHitDeconvADC1_MaxVstrip, &b_fHitDeconvADC1_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC2_MaxVstrip", &fHitDeconvADC2_MaxVstrip, &b_fHitDeconvADC2_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC3_MaxVstrip", &fHitDeconvADC3_MaxVstrip, &b_fHitDeconvADC3_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC4_MaxVstrip", &fHitDeconvADC4_MaxVstrip, &b_fHitDeconvADC4_MaxVstrip);
	fChain->SetBranchAddress("fHitDeconvADC5_MaxVstrip", &fHitDeconvADC5_MaxVstrip, &b_fHitDeconvADC5_MaxVstrip);
	fChain->SetBranchAddress("trigger_type", &trigger_type, &b_trigger_type);
	fChain->SetBranchAddress("trigger_time", &trigger_time, &b_trigger_time);
	Notify();

	int idx=-1;
	Slot7Data[++idx] = CerA0;
	Slot7Data[++idx] = CerA1;
	Slot7Data[++idx] = CerA2;
	Slot7Data[++idx] = CerA3;
	Slot7Data[++idx] = CerB0;
	Slot7Data[++idx] = CerB1;
	Slot7Data[++idx] = CerB2;
	Slot7Data[++idx] = CerB3,
	Slot7Data[++idx] = CerC0;
	Slot7Data[++idx] = CerC1;
	Slot7Data[++idx] = CerC2;
	Slot7Data[++idx] = CerC3;
	Slot7Data[++idx] = CerD0;
	Slot7Data[++idx] = CerD1;
	Slot7Data[++idx] = CerD2;
	Slot7Data[++idx] = CerD3;

	idx=-1;
	Slot8Data[++idx] = SC_D;
	Slot8Data[++idx] = SC_A;
	Slot8Data[++idx] = SC_B;
	Slot8Data[++idx] = CerSum;
	Slot8Data[++idx] = LASPD_t;
	Slot8Data[++idx] = LASPD_b;
	Slot8Data[++idx] = SC_C;
	Slot8Data[++idx] = PreSh_l;
	Slot8Data[++idx] = PreSh_r;
	Slot8Data[++idx] = PreSh_t,
	Slot8Data[++idx] = Shower_l;
	Slot8Data[++idx] = Shower_r;
	Slot8Data[++idx] = Shower_t;
	Slot8Data[++idx] = PreShSum;
	Slot8Data[++idx] = ShowerSum;
	Slot8Data[++idx] = Trig;

	////////////////////////////////////////////////////////////////////////
	//extract the run number
	fRunNumber = GetRunNumber(gFile->GetName());
}

Bool_t ReadEvTree::Notify()
{
	// The Notify() function is called when a new file is opened. This
	// can be either for a new TTree in a TChain or when when a new TTree
	// is started when using PROOF. It is normally not necessary to make changes
	// to the generated code, but the routine can be extended by the
	// user if needed. The return value is currently not used.

	return kTRUE;
}

void ReadEvTree::Show(Long64_t entry)
{
	// Print contents of entry.
	// If entry is not specified, print current entry
	if (!fChain) return;
	fChain->Show(entry);
}

Int_t ReadEvTree::Cut(Long64_t entry)
{
	// This function may be called from Loop.
	// returns  1 if entry is accepted.
	// returns -1 otherwise.
	return 1;
}
#endif // #ifdef ReadEvTree_cxx
