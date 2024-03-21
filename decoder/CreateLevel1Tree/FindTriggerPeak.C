
#include <TROOT.h>
#include <TSystem.h>
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
#include <TCut.h>

#include <TChain.h>
#include <TFile.h>

#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

static int gRunNumber = 0;

int GetRunNumber()
{
  //extract the run number
  std::string str = gFile->GetName();  //"../ROOTFILE/beamtest_hallc_3032_1.root"

  std::string fileN="", file="";
  std::size_t found = str.find_last_of("/\\");
  std::string file0 = str.substr(found+1);     //take pure file name: "beamtest_hallc_3032_1.root"

  found = file0.find_last_of(".");
  std::string file1 = file0.substr(0,found);   //remove ".root": "beamtest_hallc_3032_1"

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

void CloneStrArray(const char *target[16], const char *src[16])
{
  for(int j=0;j<16;j++) {target[j]=src[j];}
}

void GetDetName(int run, const char *Slot7Name[16],const char *Slot8Name[16])
{
  static const char *Slot7Name_82deg[16]= { \
    "CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3", \
    "CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };
  static const char *Slot8Name_82deg[16]= {
    "SC0_t", "SC0_b", "SC1_t", "SC1_b", "LASPD_t", "LASPD_b", "SPD", "PreSh_l", "PreSh_r", "PreSh_t",
    "Shower_l", "Shower_r", "Shower_t", "notuse", "ShowerSum", "Trig" };

  static const char *Slot7Name_cosmic1[16]= {
    "MaROCA_sum", "MaROCB_sum", "MaROCC_sum", "MaROCD_sum", "MaROCE_sum", "MaROCF_sum", "MaROC_ch12", "MaROC_ch08",
    "_not_use_", "PreSh_sum", "Shower_sum", "TS1", "TS2", "TS3", "MaROC_ch16", "_Trig_" };
  static const char *Slot8Name_cosmic1[16]= {
    "SC_bottom", "SC_A", "SC_top", "SC_B", "SC_D", "SC_C", "SC_E", "PreSh_r", "PreSh_l", "PreSh_t",
    "Shower_r", "Shower_l", "Shower_t", "notuse", "ShowerSum", "Trig" };

  static const char *Slot7Name_cosmic4[16]= { \
    "CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3", \
    "CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };
  static const char *Slot8Name_cosmic4[16]= {
    "SC_bottom", "SC_A", "SC_top", "SC_B", "SC_D", "SC_C", "SC_E", "PreSh_l", "PreSh_r", "PreSh_t", \
    "Shower_l", "Shower_r", "Shower_t", "PreShSum", "ShowerSum", "Trig" };

  static const char *Slot7Name_highrate[16]= { \
    "CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3", \
    "CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };
  static const char *Slot8Name_highrate[16]= { \
    "SC_D", "SC_A", "SC_B", "CerSum", "LASPD_t", "LASPD_b", "SC_C", "PreSh_l", "PreSh_r", "PreSh_t", \
    "Shower_l", "Shower_r", "Shower_t", "PreShSum", "ShowerSum", "Trig" };

  if(run<=3683) {
    CloneStrArray(Slot7Name, Slot7Name_82deg);
    CloneStrArray(Slot8Name, Slot8Name_82deg);
  } else if(run>=3684 && run<=3808) {
    CloneStrArray(Slot7Name, Slot7Name_cosmic1);
    CloneStrArray(Slot8Name, Slot8Name_cosmic1);
  } else if (run>=3809 && run<=3905) {
    CloneStrArray(Slot7Name, Slot7Name_cosmic4);
    CloneStrArray(Slot8Name, Slot8Name_cosmic4);
  } else if (run>=3906) {
    CloneStrArray(Slot7Name, Slot7Name_highrate);
    CloneStrArray(Slot8Name, Slot8Name_highrate);
  }
}

TF1* FitGaus(TH1* h1, double range_in_sigma, double &mean, double &sigma)
{
  //cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
  if (h1->GetEntries()<500) return NULL;

  double xmin=h1->GetMean()-1.3*h1->GetRMS();
  double xmax=h1->GetMean()+1.0*h1->GetRMS();
  h1->Fit("gaus","RQ","",xmin,xmax);
  TF1 *f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
  if (!f) return NULL;
  mean=f->GetParameter(1);
  sigma=f->GetParameter(2);
  xmin=mean-1.2*range_in_sigma*sigma;
  xmax=mean+range_in_sigma*sigma;

  h1->Fit("gaus","RQ","",xmin,xmax);
  f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
  if (!f) return NULL;
  mean=f->GetParameter(1);
  sigma=f->GetParameter(2);
  xmin=mean-1.3*range_in_sigma*sigma;
  xmax=mean+range_in_sigma*sigma;

  h1->Fit("gaus","R","",xmin,xmax);
  f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
  if (!f) return NULL;
  mean=f->GetParameter(1);
  sigma=f->GetParameter(2);
  f->SetLineColor(1);

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
    text->SetTextColor(1);
    sprintf(str,"Sigma = %.3G",sigma);
    text=pt->AddText(str);
    text->SetTextColor(1);
    pt->Draw("same");
  }

  return f;
}

TF1* FitGaus(char* hName, double range_in_sigma, double &mean, double &sigma)
{
  TH1* h1=(TH1 *)gROOT->FindObject(hName);
  return FitGaus(h1,range_in_sigma, mean, sigma);
}

/*
//print sth like this
//run 4014
 const float kSlot7PeakPos_4[16] = { 18,18,18,18, 18,18,18,18, 18,18,18,18, 18,18,18,18};
 const float kSlot8PeakPos_4[16] = { 18,18,18,18, 18,18,18,18, 18,18,18,18, 18,18,18,18};
*/
void WritePeakFile(int trigbit, double *peak7, double *peak8)
{
  FILE * pFile;
  char buf[1024];

  pFile = fopen("Peak.inc" , "a");

  if(gRunNumber==0) gRunNumber=GetRunNumber();
  sprintf(buf,"//run %04d:\n",gRunNumber);
  fputs(buf, stdout);
  fputs(buf, pFile);

  sprintf(buf,"  const float kSlot7PeakPos_%d[] = {",trigbit);
  for (int i=0;i<15;i++)  sprintf(buf,"%s %4.1f,",buf,peak7[i]);
  sprintf(buf,"%s %4.1f };\n",buf,peak7[15]);
  fputs(buf, stdout);
  fputs(buf, pFile);

  sprintf(buf,"  const float kSlot8PeakPos_%d[] = {",trigbit);
  for (int i=0;i<15;i++)  sprintf(buf,"%s %4.1f,",buf,peak8[i]);
  sprintf(buf,"%s %4.1f };\n",buf,peak8[15]);
  fputs(buf, stdout);
  fputs(buf, pFile);

  sprintf(buf,"#run %04d, Slot7 and Slot8 trigger bit %d:\n",gRunNumber,trigbit);
  fputs(buf, stdout);
  fputs(buf, pFile);

  sprintf(buf,"Data7_%d =",trigbit);
  for (int i=0;i<16;i++)  sprintf(buf,"%s %4.1f ",buf,peak7[i]);
  sprintf(buf,"%s\n",buf);
  fputs(buf, pFile);

  sprintf(buf,"Data8_%d =",trigbit);
  for (int i=0;i<16;i++)  sprintf(buf,"%s %4.1f ",buf,peak8[i]);
  sprintf(buf,"%s\n",buf);
  fputs(buf, pFile);

  fclose (pFile);
}

void FindPeakForBit(int trigbit)
{
  if(trigbit<1 || trigbit>6) return;
  
  const char *TrigDetName[] = {"ShowerSum","SC_D","SC_D","ShowerSum","SC_B","CerSum"};
  const char *TSCut[] = {
    "(trigger_type & 1)==1","(trigger_type & 2)==2","(trigger_type & 4)==4",
    "(trigger_type & 8)==8","(trigger_type & 16)==16","(trigger_type & 32)==32"
  };
  const char *myCut = TSCut[trigbit-1];
  const char *myDet = TrigDetName[trigbit-1];

  const char *Slot7Name[16];
  const char *Slot8Name[16];
  if(gRunNumber==0) gRunNumber=GetRunNumber();
  GetDetName(gRunNumber,Slot7Name,Slot8Name);

  TTree *EvTree = (TTree*) gROOT->FindObject("EvTree");


  char hname[200], htitle[200];
  double peak7[16],peak8[16];
  
  //check stat, if it less than 10k events, skip it
  sprintf(hname,"hTrig_%s",myDet);
  if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
  EvTree->Draw(Form("%s.peaks.pos>>%s(100,0,100)",myDet,hname),myCut);
  TH1D* hTrig = (TH1D*) gROOT->FindObject(hname);
  //cout<<hname<<": "<<hTrig->GetEntries()<<endl;
  if(hTrig->GetEntries() < 10000)
  {
    //cout<<"Run "<<gRunNumber<<" does not have enough statistic for trigger bit "<<trigbit<<". Skip it ...\n";
    return;
  }

  TCanvas *c1= new TCanvas("c1","",1200,800);
  c1->Divide(4,4);
  for(int i=0;i<16;i++) {
    c1->cd(i+1);
    sprintf(hname,"h7_%02d",i);
    if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
    EvTree->Draw(Form("%s.peaks.pos>>%s(90,5,95)",Slot7Name[i],hname),myCut);
    TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
    double xx = h1->GetBinCenter(h1->GetMaximumBin());
    double yy = h1->GetMaximum()*1.05;
    TLine *vl = new TLine(xx,0.0,xx,yy);
    vl->SetLineColor(6);
    vl->SetLineWidth(2);
    vl->SetLineStyle(7);
    vl->Draw("same");
    peak7[i] = xx;
    sprintf(htitle,"%s: peak pos=%.1f",Slot7Name[i],xx);
    h1->SetTitle(htitle);
    cout<<"peak for "<<Slot7Name[i]<<": "<<xx<<endl;
  }

  TCanvas *c2= new TCanvas("c2","",1200,800);
  c2->Divide(4,4);
  for(int i=0;i<16;i++) {
    c2->cd(i+1);
    sprintf(hname,"h8_%02d",i);
    if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
    if(i<15) EvTree->Draw(Form("%s.peaks.pos>>%s(90,5,95)",Slot8Name[i],hname),myCut);
    else EvTree->Draw(Form("%s.peaks.pos>>%s(100,0,100)",Slot8Name[i],hname),myCut);
    TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
    double xx = h1->GetBinCenter(h1->GetMaximumBin());
    double yy = h1->GetMaximum()*1.05;
    TLine *vl = new TLine(xx,0.0,xx,yy);
    vl->SetLineColor(6);
    vl->SetLineWidth(2);
    vl->SetLineStyle(7);
    vl->Draw("same");
    peak8[i] = xx;
    sprintf(htitle,"%s: peak pos=%.1f",Slot8Name[i],xx);
    h1->SetTitle(htitle);
    cout<<"peak for "<<Slot8Name[i]<<": "<<xx<<endl;
  }

  if(gRunNumber==0) gRunNumber=GetRunNumber();
  c1->cd();
  c1->Print(Form("graph/Peak_trigbit%d_run%04d.pdf(",trigbit,gRunNumber), "pdf");
  c2->cd();
  c2->Print(Form("graph/Peak_trigbit%d_run%04d.pdf)",trigbit,gRunNumber), "pdf");

  //print out the array
  WritePeakFile(trigbit,peak7,peak8);
}

void FindTriggerPeak()
{
  gStyle->SetOptStat(0);
  gRunNumber = GetRunNumber();
  for(int bit=1;bit<=6;bit++) {
    FindPeakForBit(bit);
  }
}
