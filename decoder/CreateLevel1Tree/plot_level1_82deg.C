
#include <TROOT.h>
#include <TSystem.h>
#include <TChain.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TPaveText.h>
#include <TCut.h>

#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "tools.C"
extern int gRunNumber;

const char  *TSCut[] = {
	"(TrigType & 1)==1","(TrigType & 2)==2","(TrigType & 4)==4",
	"(TrigType & 8)==8","(TrigType & 16)==16","(TrigType & 32)==32"
};

static int IsInit = 0;
const char *Slot7Name[16];
const char *Slot8Name[16];
char grName[255];

void Init()
{
	gStyle->SetOptStat(0);
	gStyle->SetPadBottomMargin(0.12);
	gStyle->SetPadTopMargin(0.12);
	gStyle->SetOptFit(0);
	gStyle->SetPadRightMargin(0.05);
	//gStyle->SetTitleSize(0.5); //0.35 default
	gStyle->SetTitleH(0.11);
	gStyle->SetTitleOffset(0.7,"X");
	gStyle->SetTitleOffset(0.8,"Y");
	gStyle->SetTitleSize(0.07,"X");
	gStyle->SetTitleSize(0.07,"Y");

	gStyle->SetLabelOffset(0.001,"X");
	gStyle->SetLabelOffset(0.001,"Y");
	gStyle->SetLabelSize(0.05,"X");
	gStyle->SetLabelSize(0.05,"Y");

	gRunNumber = GetRunNumber();

	system("mkdir -p gr_monitor");
	sprintf(grName,"gr_monitor/monitor_level1_run%04d.pdf",gRunNumber);

	GetDetName(gRunNumber,Slot7Name,Slot8Name);
	IsInit = 1;
	cout<<"plot_level1() initilizing run "<<gRunNumber<<"... done!"<<endl;
}

int GetStat(int trigbit)
{
	if(!IsInit) Init();
	TCut myCut = TSCut[trigbit-1];

	TTree *T = (TTree*) gROOT->FindObject("T");
	if(!T) return 0; 
	
	char hname[200];
	char str[100];
	TText *text=0;

	const int TrigDetIdx[] = {14,0,0,14,2,3};
	int ch=TrigDetIdx[trigbit-1];
	
	sprintf(hname,"hTrig");
	if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
	
	T->Draw(Form("Slot8Pos[%d]>>%s(100,0,100)",ch,hname),myCut && "TrigType<64");
	TH1D* hTrig = (TH1D*) gROOT->FindObject(hname); //if entry is zero, this histo will not be created
	
	int nEvent = 0;
	if(hTrig) 
	{
		nEvent = (int) hTrig->GetEntries();
		hTrig->SetTitle(Form("%s;peak pos",Slot8Name[ch]));
		
		TPaveText *pt = new TPaveText(0.40,0.40,0.80,0.70,"brNDC");
		pt->SetBorderSize(0);
		pt->SetFillColor(0);
		sprintf(str,"Run = %d",gRunNumber);
		text=pt->AddText(str);
		text->SetTextColor(1);
		sprintf(str,"TrigBit = %d",trigbit);
		text=pt->AddText(str);
		text->SetTextColor(1);
		sprintf(str,"NEvent = %d",nEvent);
		text=pt->AddText(str);
		text->SetTextColor(1);
		pt->Draw("same");
	}

	cout<<"Run="<<gRunNumber<<", trigbit="<<trigbit<<": number_of_events="<<nEvent<<endl;
	return nEvent;
}

void plotPeak(int trigbit)
{
	//if (GetStat(trigbit)<10000) return;

	if(!IsInit) Init();
	const char  *myCut = TSCut[trigbit-1];

	TTree *T = (TTree*) gROOT->FindObject("T");
	char hname[100], htitle[200];
	double peak7[16],peak8[16];

	TCanvas *c1= new TCanvas("c1",Form("slot7 peak bit %d",trigbit),1200,800);
	c1->Divide(4,4);
	for(int i=0;i<16;i++) {
		c1->cd(i+1);
		sprintf(hname,"h7Pos_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		T->Draw(Form("Slot7Pos[%d]>>%s(56,2,58)",i,hname),myCut);
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		double xx = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
		double yy = h1->GetMaximum()*1.05;
		TLine *vl = new TLine(xx,0.0,xx,yy);
		vl->SetLineColor(6);
		vl->SetLineWidth(2);
		vl->SetLineStyle(7);
		vl->Draw("same");
		peak7[i] = xx;
		sprintf(htitle,"%s: peak=%.1f; time(4ns)",Slot7Name[i],xx);
		h1->SetTitle(htitle);
		cout<<"peak for "<<Slot7Name[i]<<": "<<xx<<endl;
	}
	c1->cd();
	c1->Print(grName, "pdf");

	TCanvas *c2= new TCanvas("c2",Form("slot8 peak bit %d",trigbit),1200,800);
	c2->Divide(4,4);
	for(int i=0;i<15;i++) {
		c2->cd(i+1);
		sprintf(hname,"h8Pos_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		T->Draw(Form("Slot8Pos[%d]>>%s(56,2,58)",i,hname),myCut);
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		double xx = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
		double yy = h1->GetMaximum()*1.05;
		TLine *vl = new TLine(xx,0.0,xx,yy);
		vl->SetLineColor(6);
		vl->SetLineWidth(2);
		vl->SetLineStyle(7);
		vl->Draw("same");
		peak8[i] = xx;
		sprintf(htitle,"%s: peak=%.1f; time(4ns)",Slot8Name[i],xx);
		h1->SetTitle(htitle);
		cout<<"peak for "<<Slot8Name[i]<<": "<<xx<<endl;
	}
	c2->cd();
	c2->Print(grName, "pdf");

	//print out the array
	//WritePeakFile(trigbit,peak7,peak8);
}

void plotHeight(int trigbit)
{
	//if (GetStat(trigbit)<10000) return;

	if(!IsInit) Init();
	const char  *myCut = TSCut[trigbit-1];

	TTree *T = (TTree*) gROOT->FindObject("T");
	char hname[100], htitle[200];

	TCanvas *c3= new TCanvas("c3",Form("slot7 height bit %d",trigbit),1200,800);
	c3->Divide(4,4);
	for(int i=0;i<16;i++) {
		c3->cd(i+1);
		sprintf(hname,"h7Height_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		if(i==7) T->Draw(Form("Slot7Height[%d]>>%s(150,5,455)",i,hname),myCut);
		else T->Draw(Form("Slot7Height[%d]>>%s(50,5,155)",i,hname),myCut);
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		sprintf(htitle,"%s Height;Height (ADC)",Slot7Name[i]);
		h1->SetTitle(htitle);
		double mean,sigma;
		if(i!=7) FitGausPeak(h1,mean,sigma,9);
	}
	c3->cd();
	c3->Print(grName, "pdf");

	TCanvas *c4= new TCanvas("c4",Form("slot8 height bit %d",trigbit),1200,800);
	c4->Divide(4,4);
	for(int i=0;i<15;i++) {
		c4->cd(i+1);
		sprintf(hname,"h8Height_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		if(i<4)  T->Draw(Form("Slot8Height[%d]>>%s(100,5,305)",i,hname),myCut);
		else if(i==6)  T->Draw(Form("Slot8Height[%d]>>%s(75,5,155)",i,hname),myCut);
		else if(i==4 || i==5)  T->Draw(Form("Slot8Height[%d]>>%s(100,5,305)",i,hname),myCut);
		else if(i==7 || i==8 || i==9 || i==13)  T->Draw(Form("Slot8Height[%d]>>%s(100,5,305)",i,hname),myCut);
		else {
			if(trigbit==1 || trigbit==4)  T->Draw(Form("Slot8Height[%d]>>%s(150,5,455)",i,hname),myCut);
			else  T->Draw(Form("Slot8Height[%d]>>%s(100,5,305)",i,hname),myCut);
		}
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		sprintf(htitle,"%s Height;Height (ADC)",Slot8Name[i]);
		h1->SetTitle(htitle);
	}
	c4->cd();
	c4->Print(grName, "pdf");
}


void plotIntegral(int trigbit)
{
	//if (GetStat(trigbit)<10000) return;

	if(!IsInit) Init();
	const char  *myCut = TSCut[trigbit-1];

	TTree *T = (TTree*) gROOT->FindObject("T");
	char hname[100], htitle[200];

	TCanvas *c5= new TCanvas("c5",Form("slot7 integral bit %d",trigbit),1200,800);
	c5->Divide(4,4);
	for(int i=0;i<16;i++) {
		c5->cd(i+1);
		sprintf(hname,"h7Integral_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		if(i==7) T->Draw(Form("Cer[%d]>>%s(120,20,820)",i,hname),myCut);
		else T->Draw(Form("Cer[%d]>>%s(50,20,320)",i,hname),myCut);
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		sprintf(htitle,"%s Integral;Integral (ADC)",Slot7Name[i]);
		h1->SetTitle(htitle);
		double mean,sigma;
		if(i!=7) FitGausPeak(h1,mean,sigma,9);
	}
	c5->cd();
	c5->Print(grName, "pdf");

	TCanvas *c6= new TCanvas("c6",Form("slot8 integral bit %d",trigbit),1200,800);
	c6->Divide(4,4);
	for(int i=0;i<15;i++) {
		c6->cd(i+1);
		sprintf(hname,"h8Integral_%02d",i);
		if(gROOT->FindObject(hname)) delete gROOT->FindObject(hname);
		if(i<4)  T->Draw(Form("%s>>%s(100,50,1050)",Slot8Name[i],hname),myCut);
		else if(i==6)  T->Draw(Form("%s>>%s(50,50,550)",Slot8Name[i],hname),myCut);
		else if(i==4 || i==5)  T->Draw(Form("%s>>%s(100,50,1050)",Slot8Name[i],hname),myCut);
		else if(i==7 || i==8 || i==9 || i==13)  T->Draw(Form("%s>>%s(100,50,3050)",Slot8Name[i],hname),myCut);
		else { //i==10,11,12,14,  all showers
			if(trigbit==1 || trigbit==4)  T->Draw(Form("%s>>%s(150,50,3050)",Slot8Name[i],hname),myCut);
			else  T->Draw(Form("%s>>%s(100,50,3050)",Slot8Name[i],hname),myCut);
		}
		TH1D* h1 = (TH1D*) gROOT->FindObject(hname);
		sprintf(htitle,"%s Integral;Integral (ADC)",Slot8Name[i]);
		h1->SetTitle(htitle);
	}
	c6->cd();
	c6->Print(grName, "pdf");
}


void plot_level1_82deg()
{
	Init();

	bool startpage = true;
	TCanvas *c00= new TCanvas("c00","",1200,800);
	for(int bit=1;bit<=6;bit++) {
		c00->cd();
		if (GetStat(bit)<10000) continue;
		if(startpage) {c00->Print(Form("%s(",grName), "pdf"); startpage=false;}
		else c00->Print(Form("%s",grName), "pdf");
		plotPeak(bit);
		plotHeight(bit);
		plotIntegral(bit);
	}
	c00->Clear();
	if(!startpage) c00->Print(Form("%s)",grName), "pdf");
}


//plot monitoring histo for given runs
void plot_level1_82deg(int run_start, int run_end)
{
	char filename[255];
	for (int run=run_start;run<=run_end;run++) {
		if(run<=3683) {
			sprintf(filename,"/volatile/halla/solid/jixie/ecal_beamtest_level1/82deg/pass1/beamtest_level1_%04d_0.root",run);
		} else if(run<=3906) {
			 sprintf(filename,"/volatile/halla/solid/jixie/ecal_beamtest_level1/cosmic/pass0/beamtest_level1_%04d_0.root",run);
		} else if (run<=4285) {
			sprintf(filename,"/volatile/halla/solid/jixie/ecal_beamtest_level1/7deg/pass1/beamtest_level1_%04d_0.root",run);
		} else {
			sprintf(filename,"/volatile/halla/solid/jixie/ecal_beamtest_level1/18deg/ROOTFILE/beamtest_level1_%04d_0.root",run);
		}
		if (gSystem->AccessPathName(filename)) {
			//std::cout << "plot_level1(): file \"" << filename << "\" does not exist" << std::endl;
			sprintf(filename,"/home/solid/replay/ROOTFILE/beamtest_level1_%4d_0.root",run);  //for uvasolid2
			if (gSystem->AccessPathName(filename)) {
				//std::cout << "plot_level1(): file \"" << filename << "\" does not exist" << std::endl;
				continue;
			}
		}

		TFile *file = new TFile(filename);
		plot_level1_82deg();
		file->Close();
	}
}
