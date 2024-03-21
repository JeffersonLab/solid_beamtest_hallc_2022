
#include "tools.C"

void plotGEM() 
{
	gStyle->SetOptStat(0);
	gStyle->SetPadBottomMargin(0.12);
	gStyle->SetPadTopMargin(0.12);
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

	int run = GetRunNumber();
	TTree *EvTree = (TTree*) gROOT->FindObject("EvTree");
	TH1F *h1;
	double nEntry = (double)EvTree->GetEntries();
	
	int plotCluster = 0;
	char target[2][100], title[2][100];
	if(plotCluster) {
		sprintf(target[0],"fHitIsampMaxUclust");
		sprintf(title[0],"Uclust");
		sprintf(target[1],"fHitIsampMaxVclust");
		sprintf(title[1],"Vclust");
	}
	else {
		sprintf(target[0],"fHitIsampMaxUstrip");
		sprintf(title[0],"Ustrip");
		sprintf(target[1],"fHitIsampMaxVstrip");
		sprintf(title[1],"Vstrip");
	}
	
	
	TCanvas *c2 = new TCanvas("c2","GEM latency",1200,800);
	c2->Divide(4,3);
	c2->cd(1);
	EvTree->Draw("fNtracks_found>>h1","fNtracks_found>0");
	h1 = (TH1F*) gROOT->FindObject("h1");
	double n = (double)h1->GetEntries();
	h1->SetTitle(Form("run %d: %.0f/%.0f (%.1f%%);number of tracks",run,n,nEntry,n/nEntry*100.));
	
	c2->cd(2);
	EvTree->Draw(Form("%s>>hUall(8,-1,7)",target[0]));
	h1 = (TH1F*) gROOT->FindObject("hUall");
	h1->SetTitle(Form("%s All Layers; %s",title[0],target[0]));
	
	c2->cd(3);
	EvTree->Draw(Form("%s>>hVall(8,-1,7)",target[1]));
	h1 = (TH1F*) gROOT->FindObject("hVall");
	h1->SetTitle(Form("%s All Layers; %s",title[1],target[1]));
	
	
	for(int i=0;i<4;i++) {
		c2->cd(i+5);
		EvTree->Draw(Form("%s>>hU%d(8,-1,7)",target[0],i),Form("fHitLayer==%d",i));
		h1 = (TH1F*) gROOT->FindObject(Form("hU%d",i));
		h1->SetTitle(Form("%s Layer %d;%s",title[0],i,target[0]));
	}
	for(int i=0;i<4;i++) {
		c2->cd(i+9);
		EvTree->Draw(Form("%s>>hV%d(8,-1,7)",target[1],i),Form("fHitLayer==%d",i));
		h1 = (TH1F*) gROOT->FindObject(Form("hV%d",i));
		h1->SetTitle(Form("%s Layer %d;%s",title[1],i,target[1]));
	}
	c2->SaveAs(Form("graph/GEM_Latency_%04d.png",run));
	
}
