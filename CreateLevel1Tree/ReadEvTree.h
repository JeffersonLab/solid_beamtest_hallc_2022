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
  fdec::Fadc250Data *SC0_t;
  fdec::Fadc250Data *SC0_b;
  fdec::Fadc250Data *SC1_t;
  fdec::Fadc250Data *SC1_b;
  fdec::Fadc250Data *LASPD_t;
  fdec::Fadc250Data *LASPD_b;
  fdec::Fadc250Data *SPD;
  fdec::Fadc250Data *PreSh_l;
  fdec::Fadc250Data *PreSh_r;
  fdec::Fadc250Data *PreSh_t;
  fdec::Fadc250Data *Shower_l;
  fdec::Fadc250Data *Shower_r;
  fdec::Fadc250Data *Shower_t;
  fdec::Fadc250Data *notuse;
  fdec::Fadc250Data *ShowerSum;
  fdec::Fadc250Data *Trig;
  Int_t           trigger_type;

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
  TBranch        *b_SC0_t;   //!
  TBranch        *b_SC0_b;   //!
  TBranch        *b_SC1_t;   //!
  TBranch        *b_SC1_b;   //!
  TBranch        *b_LASPD_t;   //!
  TBranch        *b_LASPD_b;   //!
  TBranch        *b_SPD;   //!
  TBranch        *b_PreSh_l;   //!
  TBranch        *b_PreSh_r;   //!
  TBranch        *b_PreSh_t;   //!
  TBranch        *b_Shower_l;   //!
  TBranch        *b_Shower_r;   //!
  TBranch        *b_Shower_t;   //!
  TBranch        *b_notuse;   //!
  TBranch        *b_ShowerSum;   //!
  TBranch        *b_Trig;   //!
  TBranch        *b_trigger_type;   //!


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
  SC0_t = 0;
  SC0_b = 0;
  SC1_t = 0;
  SC1_b = 0;
  LASPD_t = 0;
  LASPD_b = 0;
  SPD = 0;
  PreSh_l = 0;
  PreSh_r = 0;
  PreSh_t = 0;
  Shower_l = 0;
  Shower_r = 0;
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
  fChain->SetBranchAddress("SC0_t", &SC0_t, &b_SC0_t);
  fChain->SetBranchAddress("SC0_b", &SC0_b, &b_SC0_b);
  fChain->SetBranchAddress("SC1_t", &SC1_t, &b_SC1_t);
  fChain->SetBranchAddress("SC1_b", &SC1_b, &b_SC1_b);
  fChain->SetBranchAddress("LASPD_t", &LASPD_t, &b_LASPD_t);
  fChain->SetBranchAddress("LASPD_b", &LASPD_b, &b_LASPD_b);
  fChain->SetBranchAddress("SPD", &SPD, &b_SPD);
  fChain->SetBranchAddress("PreSh_l", &PreSh_l, &b_PreSh_l);
  fChain->SetBranchAddress("PreSh_r", &PreSh_r, &b_PreSh_r);
  fChain->SetBranchAddress("PreSh_t", &PreSh_t, &b_PreSh_t);
  fChain->SetBranchAddress("Shower_l", &Shower_l, &b_Shower_l);
  fChain->SetBranchAddress("Shower_r", &Shower_r, &b_Shower_r);
  fChain->SetBranchAddress("Shower_t", &Shower_t, &b_Shower_t);
  fChain->SetBranchAddress("notuse", &notuse, &b_notuse);
  fChain->SetBranchAddress("ShowerSum", &ShowerSum, &b_ShowerSum);
  fChain->SetBranchAddress("Trig", &Trig, &b_Trig);
  fChain->SetBranchAddress("trigger_type", &trigger_type, &b_trigger_type);
  Notify();
  
  ////////////////////////////////////////////////////////////////////////
  //extract the run number
  std::string str = gFile->GetName();  //"../ROOTFILE/beamtest_hallc_3032_1.root"
  
  std::size_t found = str.find_last_of("/\\");
  std::string file = str.substr(found+1);   //"beamtest_hallc_3032_1.root"

  found = file.find_last_of("_");
  std::string file1 = file.substr(0,found); //"beamtest_hallc_3032"
  
  found = file1.find_last_of("_");
  std::string file2= file1.substr(found+1); //"3032"
  
  fRunNumber = 0;
  if(file2.length()>0) fRunNumber = atoi(file2.c_str());
  std::cout<<"file = \""<<file<<"\"  --> fRunNumber = "<<fRunNumber<<std::endl;
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
