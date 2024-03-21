//this script is used to create level1 tree for SoLID Ecal beamtest at HallC
//It contains the following functions:
//1)calculate pedestal,
//2) plot the pulse for one event using level0 tree
//3) plot the spectrum ( the pulse shape for a lot of events)
//4) create level1 tree

#define ReadEvTree_cxx

#include "ReadEvTree_highrate.h"
#include "GEM_Cluster_Matching.C"
#include "GravityWeight.h"

#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TPaveText.h>
#include <TTree.h>
#include <TObject.h>

#include <iostream>

#ifndef LEVEL1_Tree_DEBUG
//this macro will create more tree branches for debugging
//do not turn it on for regular level1 tree production
#define LEVEL1_Tree_DEBUG 0
#endif

#define verbose 0

using namespace std;

//   In a ROOT session, you can do:
//      root> .L ReadEvTree.C
//      root> ReadEvTree t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch

/*
 root cmd to plot spectrum in cmd line
int i=0;
EvTree->Draw("SC0_t.raw:Iteration$>>h(100,0,100,512,0,512)","SC0_t.raw>36","l*",10,i+=10)
EvTree->Draw("Trig.raw/100+100:Iteration$","","lsame",10,i=0)
*/

//////////////////////////////////////////////////////////////////////////
//global variables

//double gPed7[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
//double gPed8[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
//double gPed7[16]={108.1, 104.7, 100.6, 101.8, 101.5,  98.6, 103.1,  96.2, 108.4, 102.7, 107.6, 111.3, 105.2, 103.0, 104.4,  99.9};  //20030112
//double gPed8[16]={117.0, 109.7, 121.1, 116.4, 103.9, 104.0, 107.5, 111.0, 110.5, 105.6, 105.2, 102.5, 106.0, 107.9, 113.1, 103.2};  //20030112
double gPed7[16]={106.7, 104.0,  99.3, 101.7, 101.3,  98.7, 103.8,  95.1, 109.3, 102.7, 107.5, 110.6, 105.4, 101.8, 103.5,  99.4};  //20030120
double gPed8[16]={115.1, 110.7, 122.8, 118.7, 105.1, 105.0, 108.5, 107.2, 107.0, 102.1, 104.4, 100.7, 104.9, 107.7, 113.3, 103.3};  //20030120


/*
//the following has been defined in the header file
const char *Slot7Name[16]= {
"CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3",
"CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };
   
const char *Slot8Name[16]= {
"SC_D", "SC_A", "SC_B", "CerSum", "LASPD_t", "LASPD_b", "SC_C", "PreSh_l", "PreSh_r", "PreSh_t",
"Shower_l", "Shower_r", "Shower_t", "PreShSum", "ShowerSum", "Trig" };

fdec::Fadc250Data *Slot7Data[16]= {
    CerA0, CerA1, CerA2, CerA3, CerB0, CerB1, CerB2, CerB3,
    CerC0, CerC1, CerC2, CerC3, CerD0, CerD1, CerD2, CerD3 };

   
fdec::Fadc250Data *Slot8Data[16]= {
    SC_D, SC_A, SC_B, CerSum, LASPD_t, LASPD_b, SC_C, PreSh_r, PreSh_l, PreSh_t,
    Shower_r, Shower_l, Shower_t, PreShSum, ShowerSum, Trig };
    * 
*/
//////////////////////////////////////////////////////////////////////////


TF1* FitGaus(TH1* h1, double range_in_sigma=1.0)
{
  //cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
  if (h1->GetEntries()<500) return NULL;

  double xmin=h1->GetMean()-1.0*h1->GetRMS();
  double xmax=h1->GetMean()+1.0*h1->GetRMS();
  h1->Fit("gaus","RQ","",xmin,xmax);
  TF1 *f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
  if (!f) return NULL;
  double mean=f->GetParameter(1);
  double sigma=f->GetParameter(2);
  xmin=mean-range_in_sigma*sigma;
  xmax=mean+range_in_sigma*sigma;

  h1->Fit("gaus","RQ","",xmin,xmax);
  f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
  if (!f) return NULL;
  mean=f->GetParameter(1);
  sigma=f->GetParameter(2);
  f->SetLineColor(2);

  if (gStyle->GetOptFit()==0)
  {
    char str[100];
    TText *text=0;

    double xx=gStyle->GetPadLeftMargin()+0.03;
    TPaveText *pt = new TPaveText(xx,0.20,xx+0.45,0.45,"brNDC");
    pt->SetBorderSize(0);
    pt->SetFillColor(0);
    sprintf(str,"Mean = %.3G",mean);
    text=pt->AddText(str);
    text->SetTextColor(2);
    sprintf(str,"Sigma = %.3G",sigma);
    text=pt->AddText(str);
    text->SetTextColor(2);
    pt->Draw("same");
  }

  return f;
}

TF1* FitGaus(char* hName, double range_in_sigma=1.0)
{
  TH1* h1=(TH1 *)gROOT->FindObject(hName);
  return FitGaus(h1,range_in_sigma);
}

void WritePedFile(int run, int printlabel=0)
{
  FILE * pFile;
  char buf[1024];

  pFile = fopen("Pedastal.inc" , "a");

  sprintf(buf,"%4s","run#");
  for (int i=0;i<16;i++)  sprintf(buf,"%s ch7.%02d",buf,i);
  for (int i=0;i<16;i++)  sprintf(buf,"%s ch8.%02d",buf,i);
  sprintf(buf,"%s\n",buf);
  fputs (buf, stdout);
  if (printlabel) {fputs(buf, pFile);}

  sprintf(buf,"%04d",run);
  for (int i=0;i<16;i++)  sprintf(buf,"%s %6.1f",buf,gPed7[i]);
  for (int i=0;i<16;i++)  sprintf(buf,"%s %6.1f",buf,gPed8[i]);
  sprintf(buf,"%s\n",buf);
  fputs (buf, stdout);
  fputs (buf, pFile);

  fclose (pFile);
}
//////////////////////////////////////////////////////////////////////////

//create level1 tree
void ReadEvTree::Loop(Long64_t istart, Long64_t iend)
{
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  if (iend<0 || iend>nentries) iend=nentries;
  if (iend<istart) {
    std::cout<<"Loop(): iend("<<iend<<") < istart("<<istart<<"), nentries="<<nentries<<", I quit...\n";
    return;
  }

  Long64_t nev2loop = iend - istart + 1;
  //////////////////////////////////////////////////////////////////////////
 
  //extract the file name
  std::string strFullPath = gFile->GetName();  //"../ROOTFILE/beamtest_hallc_3032_1.root"

  size_t found=strFullPath.find_last_of("/\\");
  TString strDir = strFullPath.substr(0,found+1);  //with '/\\' at the end
  TString strFile = strFullPath.substr(found+1);

  if (strFile.Index("_hallc") >= 0) strFile.ReplaceAll("_hallc","_level1");
  else strFile.ReplaceAll(".root","_level1.root");
#ifdef  LEVEL1_Tree_DEBUG
  strFile.ReplaceAll(".root",Form("_debug%d.root",LEVEL1_Tree_DEBUG));
#endif

  //epics tree
  TTree *oldEpicsTree = (TTree*)gFile->Get("EpicsTree"); //get the tree address
  int Epics_event_number = 0;
  int Epics_event_number_start = 0;
  int Epics_event_number_end = 0;
  if (oldEpicsTree) {
    // Deactivate all branches: using "*" for all branches
    oldEpicsTree->SetBranchStatus("*",0);
    //Activate needed branches only
    if(oldEpicsTree->FindBranch("Epics_event_number")) {
      oldEpicsTree->SetBranchStatus("Epics_event_number",1);
      oldEpicsTree->SetBranchStatus("Epics_IPM1H01_XPOS",1);
      oldEpicsTree->SetBranchStatus("Epics_IPM1H01_YPOS",1);
      oldEpicsTree->SetBranchStatus("Epics_IPM1H04_XPOS",1);
      oldEpicsTree->SetBranchStatus("Epics_IPM1H04_YPOS",1);
      oldEpicsTree->SetBranchStatus("Epics_hac_bcm_average",1);
      oldEpicsTree->SetBranchStatus("Epics_hac_unser_current",1);
      oldEpicsTree->SetBranchStatus("Epics_ibcm1",1);
      oldEpicsTree->SetBranchStatus("Epics_ecSHMS_Angle",1);
      oldEpicsTree->SetBranchStatus("Epics_ecSDI_HP",1);
      oldEpicsTree->SetBranchStatus("Epics_ecSHB_HP",1);
      oldEpicsTree->SetBranchStatus("Epics_HALLC_p",1);

      //I need to use this branch to cut events
      oldEpicsTree->SetBranchAddress("Epics_event_number", &Epics_event_number);
    }
  }


  TFile* fout = new TFile((strDir+strFile).Data(), "RECREATE");
  TTree* T = new TTree("T", "solid hallc beamtest level1 tree");
  TTree *newEpicsTree = 0;
  if (oldEpicsTree) {
    newEpicsTree = oldEpicsTree->CloneTree(0);  //clone of old tree header
  }

  /////////////////////////////////////////////////////////////////////////
  int EntryNum=0;	//this will keep the entry index of the original level0 tree  


  char brNName[100],brNTitle[100],brDataName[100],brDataTitle[100];
  
  const int kMaxParticleNum=10;   //keep no more than 10 particles per event
  //For each FADC channel, keep the following parameters
  //const kDataParameterName[]={"Integral","Height","Pos","Left","Right"};
  
  /////////////////////////////////////////////////////////////////////////
  //Slot7
  int Slot7PartN[16], Slot7Raw[16][100];
  float Slot7Integral[16][kMaxParticleNum];
  int Slot7Height[16][kMaxParticleNum],Slot7Pos[16][kMaxParticleNum];
  int Slot7Left[16][kMaxParticleNum], Slot7Right[16][kMaxParticleNum];
  float CerASum,CerBSum,CerCSum,CerDSum,CerSum;
  int CerFiredChN;
/*
TODO:   remove saturated events, do timing cut for each trigger type
*/
  for (int i=0;i<16;i++) {
    sprintf(brNName,"%s_n",Slot7Name[i]);
    sprintf(brNTitle,"%s_n/I",Slot7Name[i]);    
    T->Branch(brNName,&Slot7PartN[i],brNTitle);
    
    //sprintf(brDataName,"%s_Integral",Slot7Name[i]);
    //sprintf(brDataTitle,"%s_Integral[%s]/F",Slot7Name[i],brNName);
    sprintf(brDataName,"%s",Slot7Name[i]);
    sprintf(brDataTitle,"%s[%s]/F",Slot7Name[i],brNName);
    T->Branch(brDataName,&Slot7Integral[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Height",Slot7Name[i]);
    sprintf(brDataTitle,"%s_Height[%s]/I",Slot7Name[i],brNName);
    T->Branch(brDataName,&Slot7Height[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Pos",Slot7Name[i]);
    sprintf(brDataTitle,"%s_Pos[%s]/I",Slot7Name[i],brNName);
    T->Branch(brDataName,&Slot7Pos[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Left",Slot7Name[i]);
    sprintf(brDataTitle,"%s_Left[%s]/I",Slot7Name[i],brNName);
    T->Branch(brDataName,&Slot7Left[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Right",Slot7Name[i]);
    sprintf(brDataTitle,"%s_Right[%s]/I",Slot7Name[i],brNName);
    T->Branch(brDataName,&Slot7Right[i][0],brDataTitle);
       
#ifdef  LEVEL1_Tree_DEBUG
    sprintf(brDataName,"%s_Raw",Slot7Name[i]);
    sprintf(brDataTitle,"%s_Raw[100]/I",Slot7Name[i]);
    T->Branch(brDataName,&Slot7Raw[i][0],brDataTitle);
#endif
  }
  
  //Initial the tree leavies values
  for (int i=0;i<16;i++) {    
    for (int j=0;j<kMaxParticleNum;j++) {
      Slot7Integral[i][j]=0.0;    
      Slot7Height[i][j]=Slot7Pos[i][j]=Slot7Left[i][j]=Slot7Right[i][j]=0;
    }
#ifdef  LEVEL1_Tree_DEBUG
    for (int j=0; j<100; j++)  Slot7Raw[i][j]=0;
#endif
  }
  CerASum=CerBSum=CerCSum=CerDSum=CerSum=CerFiredChN=0;
  
  /////////////////////////////////////////////////////////////////////////
  //Slot8
  int Slot8PartN[16], Slot8Raw[16][100];
  float Slot8Integral[16][kMaxParticleNum];
  int Slot8Height[16][kMaxParticleNum],Slot8Pos[16][kMaxParticleNum];
  int Slot8Left[16][kMaxParticleNum], Slot8Right[16][kMaxParticleNum];

  for (int i=0;i<16;i++) {
    sprintf(brNName,"%s_n",Slot8Name[i]);
    sprintf(brNTitle,"%s_n/I",Slot8Name[i]);    
    T->Branch(brNName,&Slot8PartN[i],brNTitle);
    
    //sprintf(brDataName,"%s_Integral",Slot8Name[i]);
    //sprintf(brDataTitle,"%s_Integral[%s]/F",Slot8Name[i],brNName);
    sprintf(brDataName,"%s",Slot8Name[i]);
    sprintf(brDataTitle,"%s[%s]/F",Slot8Name[i],brNName);
    T->Branch(brDataName,&Slot8Integral[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Height",Slot8Name[i]);
    sprintf(brDataTitle,"%s_Height[%s]/I",Slot8Name[i],brNName);
    T->Branch(brDataName,&Slot8Height[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Pos",Slot8Name[i]);
    sprintf(brDataTitle,"%s_Pos[%s]/I",Slot8Name[i],brNName);
    T->Branch(brDataName,&Slot8Pos[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Left",Slot8Name[i]);
    sprintf(brDataTitle,"%s_Left[%s]/I",Slot8Name[i],brNName);
    T->Branch(brDataName,&Slot8Left[i][0],brDataTitle);
    
    sprintf(brDataName,"%s_Right",Slot8Name[i]);
    sprintf(brDataTitle,"%s_Right[%s]/I",Slot8Name[i],brNName);
    T->Branch(brDataName,&Slot8Right[i][0],brDataTitle);
       
#ifdef  LEVEL1_Tree_DEBUG
    sprintf(brDataName,"%s_Raw",Slot8Name[i]);
    sprintf(brDataTitle,"%s_Raw[100]/I",Slot8Name[i]);
    T->Branch(brDataName,&Slot8Raw[i][0],brDataTitle);
#endif
  }
  
  //Initial the tree leavies values
  for (int i=0;i<16;i++) {    
    for (int j=0;j<kMaxParticleNum;j++) {
      Slot8Integral[i][j]=0.0;    
      Slot8Height[i][j]=Slot8Pos[i][j]=Slot8Left[i][j]=Slot8Right[i][j]=0;
    }
#ifdef  LEVEL1_Tree_DEBUG
    for (int j=0; j<100; j++)  Slot8Raw[i][j]=0;
#endif
  }
  
  /////////////////////////////////////////////////////////////////////////
  //other branches
  
  T->Branch("CerFiredChN", &CerFiredChN,"CerFiredChN/I");
  T->Branch("CerASum", &CerASum,"CerASum/F");
  T->Branch("CerBSum", &CerBSum,"CerBSum/F");
  T->Branch("CerCSum", &CerCSum,"CerCSum/F");
  T->Branch("CerDSum", &CerDSum,"CerDSum/F");
  T->Branch("CerSum", &CerSum,"CerSum/F");
  
  T->Branch("TrigType", &trigger_type,"TrigType/I");
  //event_number branch used to match between trees
  T->Branch("event_number",&event_number,"event_number/I");
  T->Branch("EntryNum", &EntryNum,"EntryNum/I");


  /////////////////////////////////////////////////////////////////////////
  //Ecal charge weighted cluster position
  EcalEvent *pEcalEvent = new EcalEvent();

  T->Branch("Shower_Cluster_AdcSum", &pEcalEvent->Shower_Cluster_AdcSum, "Shower_Cluster_AdcSum/F");
  T->Branch("Shower_Cluster_x", &pEcalEvent->Shower_Cluster_x, "Shower_Cluster_x/F");
  T->Branch("Shower_Cluster_y", &pEcalEvent->Shower_Cluster_y, "Shower_Cluster_y/F");
  T->Branch("Shower_Cluster_z", &pEcalEvent->Shower_Cluster_z, "Shower_Cluster_z/F");
  T->Branch("PreSh_Cluster_AdcSum", &pEcalEvent->PreSh_Cluster_AdcSum, "PreSh_Cluster_AdcSum/F");
  T->Branch("PreSh_Cluster_x", &pEcalEvent->PreSh_Cluster_x, "PreSh_Cluster_x/F");
  T->Branch("PreSh_Cluster_y", &pEcalEvent->PreSh_Cluster_y, "PreSh_Cluster_y/F");
  T->Branch("PreSh_Cluster_z", &pEcalEvent->PreSh_Cluster_z, "PreSh_Cluster_z/F");

  //////////////////////////////////////////////////////////////////////////
  //GEM stuff
  std::unordered_map<int, LayerCluster> layer_cluster_map;

  int GEM00_n,GEM01_n,GEM10_n,GEM11_n;
  float GEM00_x[MAX_CLUSTERS_PER_PLANE],GEM00_y[MAX_CLUSTERS_PER_PLANE];
  float GEM01_x[MAX_CLUSTERS_PER_PLANE],GEM01_y[MAX_CLUSTERS_PER_PLANE];
  float GEM10_x[MAX_CLUSTERS_PER_PLANE],GEM10_y[MAX_CLUSTERS_PER_PLANE];
  float GEM11_x[MAX_CLUSTERS_PER_PLANE],GEM11_y[MAX_CLUSTERS_PER_PLANE];

  float GEM00_x_adc[MAX_CLUSTERS_PER_PLANE],GEM00_y_adc[MAX_CLUSTERS_PER_PLANE];
  float GEM01_x_adc[MAX_CLUSTERS_PER_PLANE],GEM01_y_adc[MAX_CLUSTERS_PER_PLANE];
  float GEM10_x_adc[MAX_CLUSTERS_PER_PLANE],GEM10_y_adc[MAX_CLUSTERS_PER_PLANE];
  float GEM11_x_adc[MAX_CLUSTERS_PER_PLANE],GEM11_y_adc[MAX_CLUSTERS_PER_PLANE];

  //initial the tree leavies values
  GEM00_n=GEM01_n=GEM10_n=GEM10_n=0;
  for (int i=0;i<MAX_CLUSTERS_PER_PLANE;i++) {
      GEM00_x[i]=GEM00_y[i]=-9999.0;
      GEM01_x[i]=GEM01_y[i]=-9999.0;
      GEM10_x[i]=GEM10_y[i]=-9999.0;
      GEM11_x[i]=GEM11_y[i]=-9999.0;

      GEM00_x_adc[i]=GEM00_y_adc[i]=0.0;
      GEM01_x_adc[i]=GEM01_y_adc[i]=0.0;
      GEM10_x_adc[i]=GEM10_y_adc[i]=0.0;
      GEM11_x_adc[i]=GEM11_y_adc[i]=0.0;
  }

  T->Branch("GEM00_n", &GEM00_n,"GEM00_n/I");
  T->Branch("GEM00_x", &GEM00_x[0],"GEM00_x[GEM00_n]/F");
  T->Branch("GEM00_y", &GEM00_y[0],"GEM00_y[GEM00_n]/F");

  T->Branch("GEM01_n", &GEM01_n,"GEM01_n/I");
  T->Branch("GEM01_x", &GEM01_x[0],"GEM01_x[GEM01_n]/F");
  T->Branch("GEM01_y", &GEM01_y[0],"GEM01_y[GEM01_n]/F");

  T->Branch("GEM10_n", &GEM10_n,"GEM10_n/I");
  T->Branch("GEM10_x", &GEM10_x[0],"GEM10_x[GEM10_n]/F");
  T->Branch("GEM10_y", &GEM10_y[0],"GEM10_y[GEM10_n]/F");

  T->Branch("GEM11_n", &GEM11_n,"GEM11_n/I");
  T->Branch("GEM11_x", &GEM11_x[0],"GEM11_x[GEM11_n]/F");
  T->Branch("GEM11_y", &GEM11_y[0],"GEM11_y[GEM11_n]/F");

  //Xinzhan asked me to copy all GEM stuff into level 1 tree, 
  //I temperaly put them into debug mode (LEVEL1_Tree_DEBUG>=1)
  /*
  // the following have been declaration in the header file
	Int_t           GEM_nCluster;
	Int_t           GEM_planeID[MAX_CLUSTERS_PER_PLANE];   //[GEM_nCluster]
	Int_t           GEM_prodID[MAX_CLUSTERS_PER_PLANE];    //[GEM_nCluster]
	Int_t           GEM_moduleID[MAX_CLUSTERS_PER_PLANE];  //[GEM_nCluster]
	Int_t           GEM_axis[MAX_CLUSTERS_PER_PLANE];      //[GEM_nCluster]
	Int_t           GEM_size[MAX_CLUSTERS_PER_PLANE];      //[GEM_nCluster]
	Float_t         GEM_adc[MAX_CLUSTERS_PER_PLANE];       //[GEM_nCluster]
	Float_t         GEM_pos[MAX_CLUSTERS_PER_PLANE];       //[GEM_nCluster]
	Int_t           GEM_stripNo[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];   //[GEM_nCluster]
	Float_t         GEM_stripAdc[MAX_CLUSTERS_PER_PLANE][MAX_CLUSTER_SIZE];  //[GEM_nCluster]
	Int_t           GEM_nAPV;
	Int_t           GEM_apv_crate_id[8];   //[GEM_nAPV]
	Int_t           GEM_apv_mpd_id[8];   //[GEM_nAPV]
	Int_t           GEM_apv_adc_ch[8];   //[GEM_nAPV]
  */
  
#ifdef   LEVEL1_Tree_DEBUG
  if(LEVEL1_Tree_DEBUG>=1) {
    //these adc for each cluster will be useful for further charge matching
    //the existing charge matching algorithm is very poor at this job
    T->Branch("GEM00_x_adc", &GEM00_x_adc[0],"GEM00_x_adc[GEM00_n]/F");
    T->Branch("GEM00_y_adc", &GEM00_y_adc[0],"GEM00_y_adc[GEM00_n]/F");
    T->Branch("GEM01_x_adc", &GEM01_x_adc[0],"GEM01_x_adc[GEM01_n]/F");
    T->Branch("GEM01_y_adc", &GEM01_y_adc[0],"GEM01_y_adc[GEM01_n]/F");
    T->Branch("GEM10_x_adc", &GEM10_x_adc[0],"GEM10_x_adc[GEM10_n]/F");
    T->Branch("GEM10_y_adc", &GEM10_y_adc[0],"GEM10_y_adc[GEM10_n]/F");
    T->Branch("GEM11_x_adc", &GEM11_x_adc[0],"GEM11_x_adc[GEM11_n]/F");
    T->Branch("GEM11_y_adc", &GEM11_y_adc[0],"GEM11_y_adc[GEM11_n]/F");
  }
  
  if(LEVEL1_Tree_DEBUG >=2 ) {
    T->Branch("GEM_nCluster", &GEM_nCluster,"GEM_nCluster/I");
    T->Branch("GEM_planeID", &GEM_planeID[0],"GEM_planeID[GEM_nCluster]/I");
    T->Branch("GEM_prodID", &GEM_prodID[0],"GEM_prodID[GEM_nCluster]/I");
    T->Branch("GEM_moduleID", &GEM_moduleID[0],"GEM_moduleID[GEM_nCluster]/I");
    T->Branch("GEM_axis", &GEM_axis[0],"GEM_axis[GEM_nCluster]/I");
    T->Branch("GEM_size", &GEM_size[0],"GEM_size[GEM_nCluster]/I");
    T->Branch("GEM_adc", &GEM_adc[0],"GEM_adc[GEM_nCluster]/F");
    T->Branch("GEM_pos", &GEM_pos[0],"GEM_pos[GEM_nCluster]/F");
    T->Branch("GEM_stripNo", &GEM_stripNo[0],"GEM_stripNo[GEM_nCluster]/I");
    T->Branch("GEM_stripAdc", &GEM_stripAdc[0],"GEM_stripAdc[GEM_nCluster]/F");
    
    T->Branch("GEM_nAPV", &GEM_nAPV,"GEM_nAPV/I");    
    T->Branch("GEM_apv_crate_id", &GEM_apv_crate_id[0],"GEM_apv_crate_id[GEM_nAPV]/I");
    T->Branch("GEM_apv_mpd_id", &GEM_apv_mpd_id[0],"GEM_apv_mpd_id[GEM_nAPV]/I");
    T->Branch("GEM_apv_adc_ch", &GEM_apv_adc_ch[0],"GEM_apv_adc_ch[GEM_nAPV]/I");
  }

#endif

  ////////////////////////////////////////////////////////////////////////
  //currently I have to remove the peak postion matching for each wave  because there are
  //multiple trigger bit in each event. Each trigger bit might sit at various time space, depends on hardware ... 
  //loop over events to fill tree

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=istart; jentry<iend;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    
    // if (Cut(ientry) < 0) continue;
    
    if (jentry==istart) Epics_event_number_start = event_number;
    if (jentry+1==iend) Epics_event_number_end = event_number;

    if (((jentry-istart+1)%10000)==0) std::cout<<jentry-istart+1<<"/"<<iend-istart<<" events processed \r"<<std::flush;
    EntryNum=jentry;

    ////////////////reset the tree leavies values///////////////////////
    //This block has to be executed at the begining of each event
    //otherwise the 'break' statement will jump over it 
    for (int i=0;i<16;i++) { 
      Slot7PartN[i]=Slot8PartN[i]=0;         
      for (int j=0;j<kMaxParticleNum;j++) {
        Slot7Integral[i][j]=0.0;    
        Slot7Height[i][j]=Slot7Pos[i][j]=Slot7Left[i][j]=Slot7Right[i][j]=0;
        
        Slot8Integral[i][j]=0.0;    
        Slot8Height[i][j]=Slot8Pos[i][j]=Slot8Left[i][j]=Slot8Right[i][j]=0;
      }
      
#ifdef  LEVEL1_Tree_DEBUG
      for (int j=0; j<100; j++) {Slot7Raw[i][j]=Slot8Raw[i][j]=0;}
#endif
    }
    CerASum=CerBSum=CerCSum=CerDSum=CerSum=0;
    CerFiredChN=0;
    
    pEcalEvent->Reset();


    if (GEM_nCluster>0) {
      GEM00_n=GEM01_n=GEM10_n=GEM11_n=0;
      for (int i=0;i<MAX_CLUSTERS_PER_PLANE;i++) {
        GEM00_x[i]=GEM00_y[i]=-9999.0;
        GEM01_x[i]=GEM01_y[i]=-9999.0;
        GEM10_x[i]=GEM10_y[i]=-9999.0;
        GEM11_x[i]=GEM11_y[i]=-9999.0;

        GEM00_x_adc[i]=GEM00_y_adc[i]=0.0;
        GEM01_x_adc[i]=GEM01_y_adc[i]=0.0;
        GEM10_x_adc[i]=GEM10_y_adc[i]=0.0;
        GEM11_x_adc[i]=GEM11_y_adc[i]=0.0;
      }
      layer_cluster_map.clear();
    }
    ////////////////////////end of reset////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    //start to process slot 7 
    //I am giving up my own waveform analyze algorithm, which has similar result as the one in the decoder 
    //We need to develpe one algorithm which can deconvolute multiple peaks in the waveform 
    for (int ich=0; ich<16; ich++) {
      Slot7PartN[ich] = Slot7Data[ich]->peaks.size();
      if(Slot7PartN[ich]>kMaxParticleNum) Slot7PartN[ich]=kMaxParticleNum;
      
      for(int k=0;k<Slot7PartN[ich];k++) {
        Slot7Height[ich][k] =(int)(Slot7Data[ich]->peaks[k].height);
        Slot7Pos[ich][k] =(int)(Slot7Data[ich]->peaks[k].pos);
        Slot7Left[ich][k] =(int)(Slot7Data[ich]->peaks[0].left);
        Slot7Right[ich][k] =(int)(Slot7Data[ich]->peaks[0].right);
        Slot7Integral[ich][k] =(float)(Slot7Data[ich]->peaks[0].integral);        
      }

      //count fired channels
      int IsFired = 0;
      const int kCerThre = 40; 
      for(int k=0;k<Slot7PartN[ich];k++) {
        if(Slot7Data[ich]->peaks[k].height > kCerThre && IsFired==0) {IsFired=1; break;}
      }
      CerFiredChN += IsFired;
      
#ifdef  LEVEL1_Tree_DEBUG
      for (size_t j = 0; j < Slot7Data[ich]->raw.size(); j++) {
        Slot7Raw[ich][j] = Slot7Data[ich]->raw[j] - gPed7[ich];
      }
#endif
    }
    //since we do not know how many particles, I just sum up the first particle
    CerASum = Slot7Integral[0][0]+Slot7Integral[1][0]+Slot7Integral[2][0]+Slot7Integral[3][0];
    CerBSum = Slot7Integral[4][0]+Slot7Integral[5][0]+Slot7Integral[6][0];//+Slot7Integral[3][0];
    CerCSum = Slot7Integral[8][0]+Slot7Integral[9][0]+Slot7Integral[10][0]+Slot7Integral[11][0];
    CerDSum = Slot7Integral[12][0]+Slot7Integral[13][0]+Slot7Integral[14][0]+Slot7Integral[15][0];
    CerSum = CerASum + CerBSum + CerCSum + CerDSum;
  
    ////////////////////////////////////////////////////////////////////
   //slot8
   for (int ich=0; ich<16; ich++) {
      Slot8PartN[ich] = Slot8Data[ich]->peaks.size();
      if(Slot8PartN[ich]>kMaxParticleNum) Slot8PartN[ich]=kMaxParticleNum;
      
      for(int k=0;k<Slot8PartN[ich];k++) {
        Slot8Height[ich][k] =(int)(Slot8Data[ich]->peaks[k].height);
        Slot8Pos[ich][k] =(int)(Slot8Data[ich]->peaks[k].pos);
        Slot8Left[ich][k] =(int)(Slot8Data[ich]->peaks[0].left);
        Slot8Right[ich][k] =(int)(Slot8Data[ich]->peaks[0].right);
        Slot8Integral[ich][k] =(float)(Slot8Data[ich]->peaks[0].integral);
      }

#ifdef  LEVEL1_Tree_DEBUG
      for (size_t j = 0; j < Slot8Data[ich]->raw.size(); j++) {
        Slot8Raw[ich][j] = Slot8Data[ich]->raw[j] - gPed7[ich];
      }
#endif
    }
    ///////////Ecal cluster weighting///////////////////////////////////
    pEcalEvent->DoWeighting(Slot8Integral[7][0],Slot8Integral[8][0],Slot8Integral[9][0],Slot8Integral[10][0],Slot8Integral[11][0],Slot8Integral[12][0]);

    ///////////GEM cluster matching/////////////////////////////////////
    if (GEM_nCluster>=2) {

      if (verbose>=3) cout<<"\ndebug jentry "<<jentry<<": nCluster="<<GEM_nCluster<<"\n";

      // loop over each cluster
      for (int i=0; i<GEM_nCluster; i++) {
        GEMCluster cluster;
        cluster.layerID = GEM_planeID[i];
        cluster.detID = GEM_prodID[i];
        cluster.layerPosIndex = GEM_moduleID[i];
        cluster.plane = GEM_axis[i];
        cluster.size = GEM_size[i];
        cluster.adc = GEM_adc[i];
        cluster.pos = GEM_pos[i];

        if (verbose>=3) cout<<i<<":  layerID="<<GEM_planeID[i]<<", layerPosIndex="<<GEM_moduleID[i]<<", x-y-plan="<<GEM_axis[i]<<", adc="<<GEM_adc[i]<<"\n";
        layer_cluster_map[GEM_planeID[i]].addCluster(cluster);
      }

      // match clusters
      for (auto &i: layer_cluster_map) {
          i.second.Match();
      }

      // extract the result
      for (auto &layer: layer_cluster_map) {
        if (verbose>=3) cout<<"\t debug layer "<<layer.first<<"\n";
        for (auto &chamber : layer.second.chamber_cluster) {
          if (chamber.Hits2D.size()<=0) continue;
          if (verbose>=3) cout<<"\t \t debug chamber "<<chamber.chamberPosIndex<<"\n";
          for (auto &hit: chamber.Hits2D) {

            int gem_id = hit.xc.layerID*10 + hit.xc.layerPosIndex;
            if (verbose>=3) cout<<"\t \t \t debug hit, gem_id = "<<gem_id<<", x_adc="<<hit.xc.adc<<", y_adc="<<hit.yc.adc<<" \n";
            switch(gem_id) {
            case 0:
              {
                if (GEM00_n>=MAX_CLUSTERS_PER_PLANE) break;
                GEM00_x[GEM00_n] = hit.xc.pos;
                GEM00_y[GEM00_n] = hit.yc.pos;
                GEM00_x_adc[GEM00_n] = hit.xc.adc;
                GEM00_y_adc[GEM00_n] = hit.yc.adc;
                GEM00_n++;
              }
              break;
            case 1:
              {
                if (GEM01_n>=MAX_CLUSTERS_PER_PLANE) break;
                GEM01_x[GEM01_n] = hit.xc.pos;
                GEM01_y[GEM01_n] = hit.yc.pos;
                GEM01_x_adc[GEM01_n] = hit.xc.adc;
                GEM01_y_adc[GEM01_n] = hit.yc.adc;
                GEM01_n++;
              }
              break;
            case 10:
              {
                if (GEM10_n>=MAX_CLUSTERS_PER_PLANE) break;
                GEM10_x[GEM10_n] = hit.xc.pos;
                GEM10_y[GEM10_n] = hit.yc.pos;
                GEM10_x_adc[GEM10_n] = hit.xc.adc;
                GEM10_y_adc[GEM10_n] = hit.yc.adc;
                GEM10_n++;
              }
              break;
            case 11:
              {
                if (GEM11_n>=MAX_CLUSTERS_PER_PLANE) break;
                GEM11_x[GEM11_n] = hit.xc.pos;
                GEM11_y[GEM11_n] = hit.yc.pos;
                GEM11_x_adc[GEM11_n] = hit.xc.adc;
                GEM11_y_adc[GEM11_n] = hit.yc.adc;
                GEM11_n++;
              }
              break;
            default:
              std::cout << "Unknown gem_id" << gem_id << std::endl;
              break;
            } //end of switch
          } //end of hit
        } //end of chamber
      } //end of layer
    }

    T->Fill();

  }
  cout<<endl;

  //////////////////////////START of epcis tree/////////////////////////

  int nEntries = 0;
  if (oldEpicsTree) nEntries = oldEpicsTree->GetEntries();
  for (int jentry=0; jentry<nEntries;jentry++) {
    oldEpicsTree->GetEntry(jentry);
    if (Epics_event_number >= Epics_event_number_start) {
      if (Epics_event_number <= Epics_event_number_end) newEpicsTree->Fill();
      else break;
    }
    if ((jentry+1)%10000==0) std::cout<<"Filling epics tree: "<<jentry+1<<"/"<<nEntries<<" ... \r"<<std::flush;
  }
  //////////////////////////end of epcis tree///////////////////////////
  cout<<"\ndone!\n";

  fout->Write(0,TObject::kOverwrite);
  fout->Close();
}


////////////////////////////////////////////////////////////////////////

//plot the pulse for given number of events
void ReadEvTree::PlotSpectrum(Long64_t istart, Long64_t iend)
{
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  if (iend<0 || iend>nentries) iend=nentries;
  if (iend<istart) {
    std::cout<<"PlotSpectrum(): iend("<<iend<<") < istart("<<istart<<"), nentries="<<nentries<<", I quit...\n";
    return;
  }

  Long64_t nev2loop = iend - istart + 1;
  //////////////////////////////////////////////////////////////////////////
  
  TH2F *h2Spectrum7[16];
  TH2F *h2Spectrum8[16];
  for (int i=0; i<16; i++) {
    TH2F *h2_7 = (TH2F*) gROOT->FindObject(Form("h2%s",Slot7Name[i]));
    if (h2_7) delete h2_7;
    h2Spectrum7[i] = new TH2F(Form("h2%s",Slot7Name[i]),Slot7Name[i],100,0,100,562,-50,512);

    TH2F *h2_8 = (TH2F*) gROOT->FindObject(Form("h2%s",Slot8Name[i]));
    if (h2_8) delete h2_8;
    if (i!=15) h2Spectrum8[i] = new TH2F(Form("h2%s",Slot8Name[i]),Slot8Name[i],100,0,100,1124,-100,1024);
    else h2Spectrum8[i] = new TH2F(Form("h2%s",Slot8Name[i]),Slot8Name[i],100,0,100,190,-100,9400);
  }

  ////////////////////////////////////////////////////////////////////////
  //loop over events to fill histo

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=istart; jentry<iend;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

    for (int ich=0; ich<16; ich++) {
      for (size_t iw = 0; iw < Slot7Data[ich]->raw.size(); iw++) {
        h2Spectrum7[ich]->Fill(iw, Slot7Data[ich]->raw[iw]-gPed7[ich]);
      }
      for (size_t iw = 0; iw < Slot8Data[ich]->raw.size(); iw++) {
        h2Spectrum8[ich]->Fill(iw, Slot8Data[ich]->raw[iw]-gPed8[ich]);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //plot them out
  TCanvas *c07 = (TCanvas*)gROOT->FindObject("c7"); if (c07) delete c07;
  TCanvas *c7 = new TCanvas("c7","slot7",1200,900);

  TCanvas *c08 = (TCanvas*)gROOT->FindObject("c8"); if (c08) delete c08;
  TCanvas *c8 = new TCanvas("c8","slot8",1200,900);

  c7->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c7->cd(ich+1);
    h2Spectrum7[ich]->SetMarkerStyle(20);
    h2Spectrum7[ich]->Draw("colz");
  }
  c7->cd(0);
  //c7->SaveAs(Form("graph/Slot7_Spectrum_run%04d.png",fRunNumber));

  c8->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c8->cd(ich+1);
    h2Spectrum8[ich]->SetMarkerStyle(20);
    h2Spectrum8[ich]->Draw("colz");
  }
  c8->cd(0);
  //c8->SaveAs(Form("graph/Slot8_Spectrum_run%04d.png",fRunNumber));

  c7->Print(Form("graph/Spectrum_run%04d.pdf(",fRunNumber), "pdf");
  c8->Print(Form("graph/Spectrum_run%04d.pdf)",fRunNumber), "pdf");

}

//////////////////////////////////////////////////////////////////////////


bool ReadEvTree::ExtractPedestal(Long64_t istart, Long64_t iend)
{
  if (fChain == 0) return false;

  Long64_t nentries = fChain->GetEntriesFast();

  if (iend<0 || iend>nentries) iend=nentries;
  if (iend<istart) {
    std::cout<<"ExtractPedestal(): iend("<<iend<<") < istart("<<istart<<"), nentries="<<nentries<<", I quit...\n";
    return false;
  }

  Long64_t nev2loop = iend - istart + 1;
  if (nev2loop<1000) return false;
  //////////////////////////////////////////////////////////////////////////

  gStyle->SetPadRightMargin(0.01);
  
  //extract pedestal
  TH1F *h1Ped7[16],*h1Ped8[16];

  for (int i=0; i<16; i++) {
    TH1F *h1_7 = (TH1F*) gROOT->FindObject(Form("h1Ped%s",Slot7Name[i]));
    if (h1_7) delete h1_7;
    h1Ped7[i] = new TH1F(Form("h1Ped%s",Slot7Name[i]),Slot7Name[i],200,0,200);

    TH1F *h1_8 = (TH1F*) gROOT->FindObject(Form("h1Ped%s",Slot8Name[i]));
    if (h1_8) delete h1_8;
    h1Ped8[i] = new TH1F(Form("h1Ped%s",Slot8Name[i]),Slot8Name[i],800,0,800);
  }

  ////////////////////////////////////////////////////////////////////////
  //loop over events to fill histo

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=istart; jentry<iend;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

    //std::cout<<"event = "<<jentry<<"\n";

    for (int ich=0; ich<16; ich++) {
      size_t end7 = Slot7Data[ich]->raw.size();
      if (Slot7Data[ich]->peaks.size() > 0)  end7 = Slot7Data[ich]->peaks[0].left - 2;
      if (end7>50)  end7=50;  //sometimes the above Slot7Data[ich]->peaks[0].left will return a crazy number
      size_t end8 = Slot8Data[ich]->raw.size();
      if (Slot8Data[ich]->peaks.size() > 0)  end8 = Slot8Data[ich]->peaks[0].left - 2;
      if (end8>50)  end8=50;

      //std::cout<<"ich = "<<ich<<"   end7 = "<<end7<<"   end8 = "<<end8<<"\n";

      for (size_t iw = 0; iw < end7; iw++)  h1Ped7[ich]->Fill(Slot7Data[ich]->raw[iw]);
      for (size_t iw = 0; iw < end8; iw++)  h1Ped8[ich]->Fill(Slot8Data[ich]->raw[iw]);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //plot them out
  TCanvas *c07 = (TCanvas*)gROOT->FindObject("c7"); if (c07) delete c07;
  TCanvas *c7 = new TCanvas("c7","slot7",1200,900);

  TCanvas *c08 = (TCanvas*)gROOT->FindObject("c8"); if (c08) delete c08;
  TCanvas *c8 = new TCanvas("c8","slot8",1200,900);

  c7->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c7->cd(ich+1);
    gPed7[ich] = h1Ped7[ich]->GetMean();
    h1Ped7[ich]->GetXaxis()->SetRangeUser(gPed7[ich]-8,gPed7[ich]+12);
    h1Ped7[ich]->Draw();
    TF1* fgaus = FitGaus(h1Ped7[ich],3.0);
    if (fgaus) gPed7[ich] = fgaus->GetParameter(1);
    h1Ped7[ich]->SetTitle(Form("%s, ped=%.1f; raw adc",Slot7Name[ich],gPed7[ich]));

    //add vertical line
    TLine *vL = new TLine(gPed7[ich],0,gPed7[ich],h1Ped7[ich]->GetMaximum()*1.05);
    vL->SetLineColor(4);
    vL->SetLineWidth(2);
    vL->Draw("same");
  }
  c7->cd(0);
  //c7->SaveAs(Form("graph/Slot7_Pedestal_%04d.jpg",fRunNumber));

  c8->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c8->cd(ich+1);
    gPed8[ich] = h1Ped8[ich]->GetMean();
    h1Ped8[ich]->GetXaxis()->SetRangeUser(gPed8[ich]-15,gPed8[ich]+20);
    h1Ped8[ich]->Draw();
    TF1* fgaus = 0;
    if (ich==1 || ich==9) fgaus = FitGaus(h1Ped8[ich],1.5);
    else  fgaus = FitGaus(h1Ped8[ich],2.5);
    if (fgaus) gPed8[ich] = fgaus->GetParameter(1);
    h1Ped8[ich]->SetTitle(Form("%s, ped=%.1f; raw adc",Slot8Name[ich],gPed8[ich]));

    //add vertical line
    TLine *vL = new TLine(gPed8[ich],0,gPed8[ich],h1Ped8[ich]->GetMaximum()*1.05);
    vL->SetLineColor(4);
    vL->SetLineWidth(2);
    vL->Draw("same");
  }
  c8->cd(0);
  //c8->SaveAs(Form("graph/Slot8_Pedestal_%04d.jpg",fRunNumber));

  c7->Print(Form("graph/Pedestal_run%04d.pdf(",fRunNumber), "pdf");
  c8->Print(Form("graph/Pedestal_run%04d.pdf)",fRunNumber), "pdf");

  return true;
}


//////////////////////////////////////////////////////////////////////////

//plot the pulse for one event
void ReadEvTree::PlotEvent(Long64_t entry)
{
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();
  if (entry >= nentries) {
    cout<<"reach the end of file. \n";
    return;
  }

  //////////////////////////////////////////////////////////////////////////
 
  TH2F *h2Spectrum7[16];
  TH2F *h2Spectrum8[16];
  for (int i=0; i<16; i++) {
    TH2F *h2_7 = (TH2F*) gROOT->FindObject(Form("h2%s",Slot7Name[i]));
    if (h2_7) delete h2_7;
    h2Spectrum7[i] = new TH2F(Form("h2%s",Slot7Name[i]),Slot7Name[i],100,0,100,562,-50,512);

    TH2F *h2_8 = (TH2F*) gROOT->FindObject(Form("h2%s",Slot8Name[i]));
    if (h2_8) delete h2_8;
    if (i!=15) h2Spectrum8[i] = new TH2F(Form("h2%s",Slot8Name[i]),Slot8Name[i],100,0,100,1124,-100,1024);
    else h2Spectrum8[i] = new TH2F(Form("h2%s",Slot8Name[i]),Slot8Name[i],100,0,100,190,-100,9400);
  }

  //////////////////////////////////////////////////////////////////////////
  fChain->GetEntry(entry);

  for (int ich=0; ich<16; ich++) {
    for (size_t iw = 0; iw < Slot7Data[ich]->raw.size(); iw++) {
      h2Spectrum7[ich]->Fill(iw, Slot7Data[ich]->raw[iw]-gPed7[ich]);
    }
    for (size_t iw = 0; iw < Slot8Data[ich]->raw.size(); iw++) {
      if (ich!=15) h2Spectrum8[ich]->Fill(iw, Slot8Data[ich]->raw[iw]);
      else h2Spectrum8[ich]->Fill(iw, Slot8Data[ich]->raw[iw]-gPed7[ich]);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //plot them out
  TCanvas *c07 = (TCanvas*)gROOT->FindObject("c7"); if (c07) delete c07;
  TCanvas *c7 = new TCanvas("c7","slot7",1200,900);

  TCanvas *c08 = (TCanvas*)gROOT->FindObject("c8"); if (c08) delete c08;
  TCanvas *c8 = new TCanvas("c8","slot8",1200,900);

  c7->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c7->cd(ich+1);
    h2Spectrum7[ich]->SetMarkerStyle(20);
    h2Spectrum7[ich]->Draw();
  }
  c7->cd(0);
  //c7->SaveAs(Form("graph/Slot7_run%04d_event%04d.jpg",fRunNumber,(int)entry));

  c8->Divide(4,4,0.001,0.001);
  for (int ich=0; ich<16; ich++) {
    c8->cd(ich+1);
    h2Spectrum8[ich]->SetMarkerStyle(20);
    h2Spectrum8[ich]->Draw();
  }
  c8->cd(0);
  //c8->SaveAs(Form("graph/Slot8_run%04d_event%04d.jpg",fRunNumber,(int)entry));
}


//create level1 tree for current openned level0 file
void PlotSpectrum(int istart=100000, int iend=300000)
{
  ReadEvTree t;
  t.PlotSpectrum(istart,iend);
}


//extract pedstal, plot spectrum then create level1 tree for current openned level0 file
void run(int istart=0, int iend=-1)
{
  ReadEvTree t;
  t.ExtractPedestal(0, 100000); WritePedFile(t.fRunNumber,0);

  t.PlotSpectrum(0,300000);

  //create level 1 tree
  t.Loop(0, -1);
  return;
  //gStyle->SetPadRightMargin(0.15);
  //for (int i=0;i<10;i++) t.PlotEvent(i);
}


//extract pedestal for given runs
void DoPedestal(int run_start, int run_end)
{
  char filename[255];
  for (int run=run_start;run<=run_end;run++) {
    sprintf(filename,"../ROOTFILE/beamtest_hallc_%4d_0.root",run);

    if (gSystem->AccessPathName(filename)) {
      std::cout << "DoPedestal(): file \"" << filename << "\" does not exist" << std::endl;
      continue;
    }

    std::cout<<"loading file "<<filename<<endl;
    ReadEvTree t(filename);

    gStyle->SetPadRightMargin(0.01);
    bool ret = t.ExtractPedestal(0, 100000);  //return false if less than 1k events

    if (ret) {
      if (run==run_start) WritePedFile(run,1);
      else WritePedFile(run,0);

      gStyle->SetPadRightMargin(0.15);
      t.PlotSpectrum(0, -1);
    }
  }
}

//create level1 tree for current openned level0 file
void CreateLevel1Tree()
{
  ReadEvTree t;
  t.ExtractPedestal(0, 100000); WritePedFile(t.fRunNumber,0);

  gStyle->SetPadRightMargin(0.1);
#if defined LEVEL1_Tree_DEBUG && (LEVEL1_Tree_DEBUG >= 1)
  t.Loop(0, 200000);
#else
  t.Loop(0, -1);
#endif
}

//create level1 tree for given runs
void DoLevel1Tree(int run_start, int run_end)
{
  char filename[255];
  bool pedready = false;
  for (int run=run_start;run<=run_end;run++) {
    pedready = true;
    for (int subrun=0;subrun<500;subrun++) {
      sprintf(filename,"../ROOTFILE/beamtest_hallc_%04d_%d.root",run,subrun);

      if (gSystem->AccessPathName(filename)) {
        //std::cout << "DoLevel1Tree(): file \"" << filename << "\" does not exist" << std::endl;
        continue;
      }

      std::cout<<"loading file "<<filename<<endl;
      ReadEvTree t(filename);

      if (!pedready) {
        pedready = t.ExtractPedestal(0, 100000);  //return false if less than 1k events
        //cout<<"t.ExtractPedestal(0, 100000) return "<<pedready<<endl;
        if (run==run_start) WritePedFile(run,1);
        else WritePedFile(run,0);
      }

      if (pedready) {
        //cout<<"pedestal is extracted, ready to call Loop(0,-1);\n";
        t.Loop(0, -1);
      }
    }
  }
}

void DoLevel1Tree_highrate(int run_start, int run_end)
{
	DoLevel1Tree(run_start, run_end);	
}

void run_highrate()
{
  run();
}
