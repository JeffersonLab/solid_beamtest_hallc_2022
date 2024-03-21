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
#define MAX_CLUSTER_SIZE 100
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
	Int_t           GEM_prodID[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_moduleID[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_axis[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_size[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Float_t         GEM_adc[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Float_t         GEM_pos[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_stripNo[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];   //[GEM_nCluster]
	Float_t         GEM_stripAdc[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];   //[GEM_nCluster]
	Int_t           GEM_nAPV;
	Int_t           GEM_apv_crate_id[8];   //[GEM_nAPV]
	Int_t           GEM_apv_mpd_id[8];   //[GEM_nAPV]
	Int_t           GEM_apv_adc_ch[8];   //[GEM_nAPV]

	fdec::Fadc250Data *MaROCA_sum;
	fdec::Fadc250Data *MaROCB_sum;
	fdec::Fadc250Data *MaROCC_sum;
	fdec::Fadc250Data *MaROCD_sum;
	fdec::Fadc250Data *MaROCE_sum;
	fdec::Fadc250Data *MaROCF_sum;
	fdec::Fadc250Data *MaROC_ch12;
	fdec::Fadc250Data *MaROC_ch08;
	fdec::Fadc250Data *_not_use_;
	fdec::Fadc250Data *PreSh_sum;
	fdec::Fadc250Data *Shower_sum;
	fdec::Fadc250Data *TS1;
	fdec::Fadc250Data *TS2;
	fdec::Fadc250Data *TS3;
	fdec::Fadc250Data *MaROC_ch16;
	fdec::Fadc250Data *_Trig_;
	fdec::Fadc250Data *SC_bottom;
	fdec::Fadc250Data *SC_A;
	fdec::Fadc250Data *SC_top;
	fdec::Fadc250Data *SC_B;
	fdec::Fadc250Data *SC_D;
	fdec::Fadc250Data *SC_C;
	fdec::Fadc250Data *SC_E;
	fdec::Fadc250Data *PreSh_r;
	fdec::Fadc250Data *PreSh_l;
	fdec::Fadc250Data *PreSh_t;
	fdec::Fadc250Data *Shower_r;
	fdec::Fadc250Data *Shower_l;
	fdec::Fadc250Data *Shower_t;
	fdec::Fadc250Data *notuse;
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
	TBranch        *b_GEM_nAPV;   //!
	TBranch        *b_GEM_apv_crate_id;   //!
	TBranch        *b_GEM_apv_mpd_id;   //!
	TBranch        *b_GEM_apv_adc_ch;   //!
	TBranch        *b_MaROCA_sum;   //!
	TBranch        *b_MaROCB_sum;   //!
	TBranch        *b_MaROCC_sum;   //!
	TBranch        *b_MaROCD_sum;   //!
	TBranch        *b_MaROCE_sum;   //!
	TBranch        *b_MaROCF_sum;   //!
	TBranch        *b_MaROC_ch12;   //!
	TBranch        *b_MaROC_ch08;   //!
	TBranch        *b__not_use_;   //!
	TBranch        *b_PreSh_sum;   //!
	TBranch        *b_Shower_sum;   //!
	TBranch        *b_TS1;   //!
	TBranch        *b_TS2;   //!
	TBranch        *b_TS3;   //!
	TBranch        *b_MaROC_ch16;   //!
	TBranch        *b__Trig_;   //!
	TBranch        *b_SC_bottom;   //!
	TBranch        *b_SC_A;   //!
	TBranch        *b_SC_top;   //!
	TBranch        *b_SC_B;   //!
	TBranch        *b_SC_D;   //!
	TBranch        *b_SC_C;   //!
	TBranch        *b_SC_E;   //!
	TBranch        *b_PreSh_r;   //!
	TBranch        *b_PreSh_l;   //!
	TBranch        *b_PreSh_t;   //!
	TBranch        *b_Shower_r;   //!
	TBranch        *b_Shower_l;   //!
	TBranch        *b_Shower_t;   //!
	TBranch        *b_notuse;   //!
	TBranch        *b_ShowerSum;   //!
	TBranch        *b_Trig;   //!
	TBranch        *b_trigger_type;   //!
	TBranch        *b_trigger_time;   //!


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
		TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("../ROOTFILE/beamtest_hallc_3032_0.root");
		if (!f || !f->IsOpen()) {
			f = new TFile("../ROOTFILE/beamtest_hallc_3032_0.root");
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
	MaROCA_sum = 0;
	MaROCB_sum = 0;
	MaROCC_sum = 0;
	MaROCD_sum = 0;
	MaROCE_sum = 0;
	MaROCF_sum = 0;
	MaROC_ch12 = 0;
	MaROC_ch08 = 0;
	_not_use_ = 0;
	PreSh_sum = 0;
	Shower_sum = 0;
	TS1 = 0;
	TS2 = 0;
	TS3 = 0;
	MaROC_ch16 = 0;
	_Trig_ = 0;
	SC_bottom = 0;
	SC_A = 0;
	SC_top = 0;
	SC_B = 0;
	SC_D = 0;
	SC_C = 0;
	SC_E = 0;
	PreSh_r = 0;
	PreSh_l = 0;
	PreSh_t = 0;
	Shower_r = 0;
	Shower_l = 0;
	Shower_t = 0;
	notuse = 0;
	ShowerSum = 0;
	Trig = 0;
	// Set branch addresses and branch pointers
	if (!tree) return;
	fChain = tree;
	fCurrent = -1;
	fChain->SetMakeClass(1);

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
	fChain->SetBranchAddress("GEM_nAPV", &GEM_nAPV, &b_GEM_nAPV);
	fChain->SetBranchAddress("GEM_apv_crate_id", GEM_apv_crate_id, &b_GEM_apv_crate_id);
	fChain->SetBranchAddress("GEM_apv_mpd_id", GEM_apv_mpd_id, &b_GEM_apv_mpd_id);
	fChain->SetBranchAddress("GEM_apv_adc_ch", GEM_apv_adc_ch, &b_GEM_apv_adc_ch);
	fChain->SetBranchAddress("MaROCA_sum", &MaROCA_sum, &b_MaROCA_sum);
	fChain->SetBranchAddress("MaROCB_sum", &MaROCB_sum, &b_MaROCB_sum);
	fChain->SetBranchAddress("MaROCC_sum", &MaROCC_sum, &b_MaROCC_sum);
	fChain->SetBranchAddress("MaROCD_sum", &MaROCD_sum, &b_MaROCD_sum);
	fChain->SetBranchAddress("MaROCE_sum", &MaROCE_sum, &b_MaROCE_sum);
	fChain->SetBranchAddress("MaROCF_sum", &MaROCF_sum, &b_MaROCF_sum);
	fChain->SetBranchAddress("MaROC_ch12", &MaROC_ch12, &b_MaROC_ch12);
	fChain->SetBranchAddress("MaROC_ch08", &MaROC_ch08, &b_MaROC_ch08);
	fChain->SetBranchAddress("_not_use_", &_not_use_, &b__not_use_);
	fChain->SetBranchAddress("PreSh_sum", &PreSh_sum, &b_PreSh_sum);
	fChain->SetBranchAddress("Shower_sum", &Shower_sum, &b_Shower_sum);
	fChain->SetBranchAddress("TS1", &TS1, &b_TS1);
	fChain->SetBranchAddress("TS2", &TS2, &b_TS2);
	fChain->SetBranchAddress("TS3", &TS3, &b_TS3);
	fChain->SetBranchAddress("MaROC_ch16", &MaROC_ch16, &b_MaROC_ch16);
	fChain->SetBranchAddress("_Trig_", &_Trig_, &b__Trig_);
	fChain->SetBranchAddress("SC_bottom", &SC_bottom, &b_SC_bottom);
	fChain->SetBranchAddress("SC_A", &SC_A, &b_SC_A);
	fChain->SetBranchAddress("SC_top", &SC_top, &b_SC_top);
	fChain->SetBranchAddress("SC_B", &SC_B, &b_SC_B);
	fChain->SetBranchAddress("SC_D", &SC_D, &b_SC_D);
	fChain->SetBranchAddress("SC_C", &SC_C, &b_SC_C);
	fChain->SetBranchAddress("SC_E", &SC_E, &b_SC_E);
	fChain->SetBranchAddress("PreSh_r", &PreSh_r, &b_PreSh_r);
	fChain->SetBranchAddress("PreSh_l", &PreSh_l, &b_PreSh_l);
	fChain->SetBranchAddress("PreSh_t", &PreSh_t, &b_PreSh_t);
	fChain->SetBranchAddress("Shower_r", &Shower_r, &b_Shower_r);
	fChain->SetBranchAddress("Shower_l", &Shower_l, &b_Shower_l);
	fChain->SetBranchAddress("Shower_t", &Shower_t, &b_Shower_t);
	fChain->SetBranchAddress("notuse", &notuse, &b_notuse);
	fChain->SetBranchAddress("ShowerSum", &ShowerSum, &b_ShowerSum);
	fChain->SetBranchAddress("Trig", &Trig, &b_Trig);
	fChain->SetBranchAddress("trigger_type", &trigger_type, &b_trigger_type);
	fChain->SetBranchAddress("trigger_time", &trigger_time, &b_trigger_time);
	Notify();

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
