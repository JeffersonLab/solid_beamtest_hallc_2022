/*
//Skim Hall C beam test replayed root files 
//input: Level1_3153.root output: Level1_3153_skimmed#.root, where # is the skim level
//usage: 
//  root -b -q Level1_3153.root skim_level1.C+\(2\)
//  root -b -q 'skim_level1.C+(3152,3162,2)'
//__________________________________________________________________________
*/
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>

#include <TROOT.h>
#include <TDirectory.h>
#include <TMath.h>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TCut.h>
#include <TString.h>

using namespace std;

//##########################config block start#############################
//#define SKIM_DEBUG 0
bool IsLinux=true;
//jlab config
const char* InPath="/work/halla/solid/jixie/ecal_beamtest_hallc/ROOTFILE/";
const char* OutPath="./skimmed/";

//////////////////////////////////////////////////////////////////
//T->SetAlias("PreShSum","PreSh_r+PreSh_l+PreSh_t");
//T->SetAlias("ShSum","Shower_r+Shower_l+Shower_t");
const char* gStrCut[7] = {
  "(TrigType==1 && SC0_t+SC0_b>300 && SC1_b+SC1_t>300) || (TrigType==2 && PreSh_t>200 && Shower_t>200)",   //trigger cut //skim level0
  "PreShSum>1 && ShowerSum>1",  //ShowerCut //skim level1
  "GEM00_n>0 && GEM10_n>0",  //GEMCut  //skim level2
  "LASPD_t+LASPD_b>300",   //LASPDCut  //skim level3

  "PreSh_t>120 && Shower_t>120",  //TopBlockCut //skim level11
  "CerSum>100 && PreShSum>200 && ShowerSum>200",  //electron cut  //skim level19
  "GEM00_n==1 && GEM10_n==1 && abs(GEM00_x[0])<20 && abs(GEM00_y[0])<20 && abs(GEM10_x[0])<20 && abs(GEM10_y[0])<20"  //Sqr20x20Cut //skim level35
};


//##########################config block end################################

void Skim(const char* infile, int level=2, const char *treename="T", int overwrite=1);

//-------------------------global variable block start----------------------

//counters
unsigned int gCounter=0;
unsigned int iCounter[7]={0,0,0,0,0,0,0};

//Declaration of leaves types
Float_t         Slot7Integral0[16];
Int_t           Slot7Left0[16];
Int_t           Slot7Right0[16];
Int_t           Slot7Left[16];
Int_t           Slot7Pos[16];
Int_t           Slot7Right[16];
Float_t         Slot7ADC[16];
Int_t           Slot7Height[16];
Float_t         Slot8Integral0[16];
Int_t           Slot8Left0[16];
Int_t           Slot8Right0[16];
Int_t           Slot8Left[16];
Int_t           Slot8Pos[16];
Int_t           Slot8Right[16];
Float_t         Slot8ADC[16];
Int_t           Slot8Height[16];

Int_t           event_number;
Float_t         Cer[16];
Float_t         SC0_t;
Float_t         SC0_b;
Float_t         SC1_t;
Float_t         SC1_b;
Float_t         LASPD_t;
Float_t         LASPD_b;
Float_t         SPD;
Float_t         PreSh_l;
Float_t         PreSh_r;
Float_t         PreSh_t;
Float_t         Shower_l;
Float_t         Shower_r;
Float_t         Shower_t;
Float_t         ShowerSum;
Float_t         CerASum;
Float_t         CerBSum;
Float_t         CerCSum;
Float_t         CerDSum;
Float_t         CerSum;
Int_t           EntryNum;
Int_t           TrigType;
Float_t         Shower_Cluster_AdcSum;
Float_t         Shower_Cluster_x;
Float_t         Shower_Cluster_y;
Float_t         Shower_Cluster_z;
Float_t         PreSh_Cluster_AdcSum;
Float_t         PreSh_Cluster_x;
Float_t         PreSh_Cluster_y;
Float_t         PreSh_Cluster_z;
Int_t           GEM00_n;
Float_t         GEM00_x[100];
Float_t         GEM00_y[100];
Int_t           GEM01_n;
Float_t         GEM01_x[100];
Float_t         GEM01_y[100];
Int_t           GEM10_n;
Float_t         GEM10_x[100];
Float_t         GEM10_y[100];
Int_t           GEM11_n;
Float_t         GEM11_x[100];
Float_t         GEM11_y[100];

Float_t         PreShSum;

//-------------------------global variable block end------------------------

// Set branch addresses.
void SetBranchAddress(TTree* T)
{
  //this block if for debug tree: //#define LEVEL1_Tree_DEBUG 0
  if (T->FindBranch("Slot7Integral0")) {
    T->SetBranchAddress("Slot7Integral0",Slot7Integral0);
    T->SetBranchAddress("Slot7Left0",Slot7Left0);
    T->SetBranchAddress("Slot7Right0",Slot7Right0);
    T->SetBranchAddress("Slot7Left",Slot7Left);
    T->SetBranchAddress("Slot7Pos",Slot7Pos);
    T->SetBranchAddress("Slot7Right",Slot7Right);
    T->SetBranchAddress("Slot7ADC",Slot7ADC);
    T->SetBranchAddress("Slot7Height",Slot7Height);
    T->SetBranchAddress("Slot8Integral0",Slot8Integral0);
    T->SetBranchAddress("Slot8Left0",Slot8Left0);
    T->SetBranchAddress("Slot8Right0",Slot8Right0);
    T->SetBranchAddress("Slot8Left",Slot8Left);
    T->SetBranchAddress("Slot8Pos",Slot8Pos);
    T->SetBranchAddress("Slot8Right",Slot8Right);
    T->SetBranchAddress("Slot8ADC",Slot8ADC);
    T->SetBranchAddress("Slot8Height",Slot8Height);
  }
  
  T->SetBranchAddress("event_number",&event_number);
  T->SetBranchAddress("Cer",Cer);
  T->SetBranchAddress("SC0_t",&SC0_t);
  T->SetBranchAddress("SC0_b",&SC0_b);
  T->SetBranchAddress("SC1_t",&SC1_t);
  T->SetBranchAddress("SC1_b",&SC1_b);
  T->SetBranchAddress("LASPD_t",&LASPD_t);
  T->SetBranchAddress("LASPD_b",&LASPD_b);
  T->SetBranchAddress("SPD",&SPD);
  T->SetBranchAddress("PreSh_l",&PreSh_l);
  T->SetBranchAddress("PreSh_r",&PreSh_r);
  T->SetBranchAddress("PreSh_t",&PreSh_t);
  T->SetBranchAddress("Shower_l",&Shower_l);
  T->SetBranchAddress("Shower_r",&Shower_r);
  T->SetBranchAddress("Shower_t",&Shower_t);
  T->SetBranchAddress("ShowerSum",&ShowerSum);
  T->SetBranchAddress("CerASum",&CerASum);
  T->SetBranchAddress("CerBSum",&CerBSum);
  T->SetBranchAddress("CerCSum",&CerCSum);
  T->SetBranchAddress("CerDSum",&CerDSum);
  T->SetBranchAddress("CerSum",&CerSum);
  T->SetBranchAddress("EntryNum",&EntryNum);
  T->SetBranchAddress("TrigType",&TrigType);
  T->SetBranchAddress("Shower_Cluster_AdcSum",&Shower_Cluster_AdcSum);
  T->SetBranchAddress("Shower_Cluster_x",&Shower_Cluster_x);
  T->SetBranchAddress("Shower_Cluster_y",&Shower_Cluster_y);
  T->SetBranchAddress("Shower_Cluster_z",&Shower_Cluster_z);
  T->SetBranchAddress("PreSh_Cluster_AdcSum",&PreSh_Cluster_AdcSum);
  T->SetBranchAddress("PreSh_Cluster_x",&PreSh_Cluster_x);
  T->SetBranchAddress("PreSh_Cluster_y",&PreSh_Cluster_y);
  T->SetBranchAddress("PreSh_Cluster_z",&PreSh_Cluster_z);

  T->SetBranchAddress("GEM00_n",&GEM00_n);
  T->SetBranchAddress("GEM00_x",GEM00_x);
  T->SetBranchAddress("GEM00_y",GEM00_y);
  T->SetBranchAddress("GEM01_n",&GEM01_n);
  T->SetBranchAddress("GEM01_x",&GEM01_x);
  T->SetBranchAddress("GEM01_y",&GEM01_y);
  T->SetBranchAddress("GEM10_n",&GEM10_n);
  T->SetBranchAddress("GEM10_x",GEM10_x);
  T->SetBranchAddress("GEM10_y",GEM10_y);
  T->SetBranchAddress("GEM11_n",&GEM11_n);
  T->SetBranchAddress("GEM11_x",&GEM11_x);
  T->SetBranchAddress("GEM11_y",&GEM11_y);
}


void Skim(const char* infile, int skimlevel, const char *treename, int overwrite)
{    
  if(skimlevel<0 ) skimlevel=0;
  char cmd[255];
  if(!IsLinux) sprintf(cmd,"mkdir %s",OutPath);
  else sprintf(cmd,"mkdir -p %s",OutPath);
  system(cmd);
  //////////////////////////////////////////////////
  //check input file exist or not
  Long_t id,size,flags,mt;
  Int_t iFound = gSystem->GetPathInfo(infile,&id,&size,&flags,&mt);
  if(iFound!=0) {
      cout<<"Source root file \""<<infile<<" \"not found. exit ...\n";
      return; //File not exist (for loop through all root files)
  }
  
  //extract the output file name
  std::string strFullPath = infile;  //"../ROOTFILE/beamtest_hallc_3032_1.root"  
  size_t found=strFullPath.find_last_of("/\\");
  TString strDir = strFullPath.substr(0,found+1);  //with '/\\' at the end
  TString strFile = strFullPath.substr(found+1);  
  strFile.ReplaceAll(".root",Form("_skimmed%d.root",skimlevel));
  TString outDir = OutPath;
  std::string outfile = (outDir+strFile).Data();
  
  //check output file exist or not using method "AccessPathName"
  //AccessPathName(const char *path, EAccessMode mode=kFileExists)  Returns FALSE if one can access a file using the specified access mode. 
  if(!gSystem->AccessPathName(outfile.c_str(),kFileExists)  && overwrite==0) {
      cout<<"Output root file \""<<outfile<<" \" found. skip this file...\n";
      return; 
  }
      
  cout <<"\n*****************************New File Start*******************************\n";
  cout << "Skim(): try to skim file "<<infile<<"..."<<endl;
  sprintf(cmd,"ls -lh %s",infile);
  system(cmd);
  
  TFile *oldfile = new TFile(infile);
  TTree *oldtree = (TTree*)oldfile->Get(treename); //get the tree address
  // Activate all branches: using "*" for all branches
  oldtree->SetBranchStatus("*",1);
  SetBranchAddress(oldtree);
  
  TTree *oldepicstree = (TTree*)oldfile->Get("EpicsTree"); //get the tree address
  // Activate all branches: using "*" for all branches
  oldepicstree->SetBranchStatus("*",1);
    
  //Create a new file + a clone of old tree header. Do not copy events       
  TFile *newfile = new TFile(outfile.c_str(),"recreate");
  TTree *newtree = oldtree->CloneTree(0);
  //add one more leaf into new tree
  newtree->Branch("PreShSum",&PreShSum,"PreShSum/F");
  
  TTree *newepicstree = oldepicstree->CloneTree(0);
  
  UInt_t nentries = (UInt_t)oldtree->GetEntries();
  cout<<"Number of entries ="<<nentries<<endl;
  
   //reset counters
  gCounter=0;
  for(int ii=0;ii<7;ii++) iCounter[ii]=0;
  
  for (UInt_t i=0;i<nentries; i++) {
    oldtree->GetEntry(i);
    
    PreShSum = PreSh_l + PreSh_r + PreSh_t;
    ShowerSum = Shower_l + Shower_r + Shower_t;
    
    int pPassLevel = -1;
    //level 0
    if (TrigType == 1) {
      if (SC0_t+SC0_b>300 && SC1_b+SC1_t>300) {pPassLevel=0;iCounter[0]++;} 
      else {if(skimlevel>=0) continue;};
    } 
    else if (TrigType == 2 || TrigType == 4) {
      if (PreSh_t>200 && Shower_t>200) {pPassLevel=0;iCounter[0]++;} 
      else {if(skimlevel>=0) continue;};
    } 
    else {
      if (ShowerSum>200) {pPassLevel=0;iCounter[0]++;} 
      else {if(skimlevel>=0) continue;};      
    }
  
    //level 1
    if (PreShSum>1 && ShowerSum>1) {pPassLevel+=1;iCounter[1]++;} 
    else {if(skimlevel>=1) continue;};
    
    //level 2
    if (GEM00_n>0 && GEM10_n>0) {pPassLevel+=2;iCounter[2]++;} 
    else {if(skimlevel>=2) continue;};
    
    //level 3
    if (LASPD_t+LASPD_b>300) {pPassLevel+=4;iCounter[3]++;} 
    else {if(skimlevel==3) continue;};
    
    //level 11
    if(PreSh_t>120 && Shower_t>120) {pPassLevel+=8;iCounter[4]++;}  //TopBlockCut
    else {if(skimlevel==11) continue;};
    
    //level 19
    if(CerSum>100 && PreShSum>200 && ShowerSum>200) {pPassLevel+=16;iCounter[5]++;}  //electron cut
    else {if(skimlevel==19) continue;};
    
    //level 35
    if(GEM00_n==1 && GEM10_n==1 && fabs(GEM00_x[0])<20 && fabs(GEM00_y[0])<20 && \
      fabs(GEM10_x[0])<20 && fabs(GEM10_y[0])<20) {
      pPassLevel+=32;  //Sqr20x20Cut
      iCounter[6]++;
    }
    else {if(skimlevel==35) continue;};
    
    
    
    newtree->Fill(); gCounter++;
    
    if( !((i+1)%1000) || i+1==nentries )
      printf("%6d/%6d events processed, %6d pass level %d skim......\r", i+1,nentries,gCounter,skimlevel);
  }
  cout<<endl;
  
  //print the stat
  for(int ii=0;ii<7;ii++) {
    printf("%6d events pass level %d skim (%s)\n", iCounter[ii],ii, gStrCut[ii]);
  }
  
  //copy epics tree
  nentries = oldepicstree->GetEntries();
  for (uint i=0;i<nentries; i++) {
    oldepicstree->GetEntry(i);
    newepicstree->Fill();
  }
  
  newfile->Write();
  delete newfile;
  delete oldfile;
}

//run Main code
void skim_level1(int skimlevel=2) {
  Skim(gFile->GetName(),skimlevel,"T");
};

void skim_level1(int run_start, int run_end, int skimlevel=2, int overwrite=1)
{
  char filename[255];
  for(int run=run_start;run<=run_end;run++) {
    for(int subrun=0;subrun<100;subrun++) {
      sprintf(filename,"%s/beamtest_level1_%04d_%d.root",InPath,run,subrun);
       
      if(gSystem->AccessPathName(filename)){
        //std::cout << "file \"" << filename << "\" does not exist" << std::endl;
        continue;
      }
      Skim(filename,skimlevel,"T");
    }
  }
}
