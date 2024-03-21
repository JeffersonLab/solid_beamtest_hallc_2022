
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

int GetRunNumber(const char* infile="")
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

void Clone1DArray(double src[16], double target[16]) 
{   
		for(int j=0;j<16;j++) {target[j]=src[j];}
}

void Clone2DArray(double src[6][16], double target[6][16]) 
{
	for(int i=0;i<6;i++) {      
		for(int j=0;j<16;j++) {target[i][j]=src[i][j];}
	}
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
	
	cout    <<"Slot8Name[] = { ";
	for(int i=0;i<8;i++) cout<<"\""<<Slot8Name[i]<<"\", ";
	cout<<"\\\n                ";
	for(int i=8;i<15;i++) cout<<"\""<<Slot8Name[i]<<"\", ";
	cout<<"\""<<Slot8Name[15]<<"\"}; \n";
}

TF1* FitGausPeak(TH1* _h1, double _xmin, double _xmax, double &mean, double &sigma, double minimumbin=12.0)
{
	//cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
	if (_h1->GetEntries()<500) return NULL;
	
	//cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
	if (_h1->GetEntries()<500) return NULL;
	//make a copy of h1 and keep only the fitting range
	TH1 *h1 = (TH1*)_h1->Clone();
	int n = h1->GetNbinsX();
	for(int i=1;i<=n;i++) {
		if(h1->GetBinLowEdge(i)<_xmin || h1->GetBinLowEdge(i+1)>_xmax ) {
			h1->SetBinContent(i,0.0);
		}
	}
	
	const int kMinBin = 7.0;
	if (minimumbin < kMinBin) minimumbin = kMinBin;
	
	double xpeak = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
	double binw = h1->GetXaxis()->GetBinWidth(1);
	double _nbin0=24, _nbin1=23, _chi2 = 1000000.;
	double xmin, xmax;
	
	TF1 *f=0;
	double nbin0=_nbin0, nbin1=_nbin1, chi2=_chi2;
	int count=0;
	while ( nbin0+nbin1 > minimumbin ) {
		xmin = xpeak - nbin0*binw;
		xmax = xpeak + nbin1*binw;
		h1->Fit("gaus","RQ0","",xmin,xmax);
		f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
		if (!f) return NULL;
		mean=f->GetParameter(1);
		sigma=f->GetParameter(2); 
		chi2 = f->GetChisquare()/f->GetNDF();
		if(_chi2 > chi2) {
			_chi2 = chi2;
			_nbin0 = nbin0;
			_nbin1 = nbin1;
			//cout<<"Update Chi2 to "<<_chi2<<", mean="<<mean<<", error="<<f->GetParError(1)<<endl;
		}
		if((count++)%2) nbin1 -= 1.0;
		else nbin0 -= 1.0;
		if(minimumbin > kMinBin && minimumbin > 3*sigma/binw) {
			minimumbin = 3*sigma/binw;
			if (minimumbin < kMinBin) minimumbin = kMinBin;
			//cout<<h1->GetName()<<": update minimumbin to "<<minimumbin<<", sigma="<<sigma<<", error="<<f->GetParError(2)<<endl;
		}
	}
	
	//recover the minimum chi2 fit
	xmin = xpeak - _nbin0*binw;
	xmax = xpeak + _nbin1*binw;
	if(xmin<_xmin) xmin=_xmin;
	if(xmax>_xmax) xmax=_xmax;
	
	_h1->Fit("gaus","RQ","",xmin,xmax);
	f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
	if (!f) return NULL;
	mean=f->GetParameter(1);
	sigma=f->GetParameter(2);
	f->SetLineColor(6);

	if (gStyle->GetOptFit()==0)
	{
		char str[100];
		TText *text=0;

		//double xx=gStyle->GetPadLeftMargin()+0.06;
		double xx=0.40;
		TPaveText *pt = new TPaveText(xx,0.65,xx+0.45,0.85,"brNDC");
		pt->SetBorderSize(0);
		pt->SetFillColor(0);
		if(gROOT->IsBatch()) pt->SetFillStyle(4000);
		sprintf(str,"Mean = %.3G",mean);
		text=pt->AddText(str);
		text->SetTextColor(6);
		sprintf(str,"Sigma = %.3G",sigma);
		text=pt->AddText(str);
		text->SetTextColor(6);
		pt->Draw("same");
	}

	return f;
}

TF1* FitGausPeak(TH1* h1, double &mean, double &sigma, double minimumbin=12.0)
{
	//cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
	if (h1->GetEntries()<500) return NULL;
	const int kMinBin = 7.0;
	if (minimumbin < kMinBin) minimumbin = kMinBin;
	
	double xpeak = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
	double binw = h1->GetXaxis()->GetBinWidth(1);
	double _nbin0=24, _nbin1=23, _chi2 = 1000000.;
	
	TF1 *f=0;
	double nbin0=_nbin0, nbin1=_nbin1, chi2=_chi2;
	double xmin,xmax;
	int count=0;
	while ( nbin0+nbin1 > minimumbin ) {
		xmin = xpeak - nbin0*binw;
		xmax = xpeak + nbin1*binw;
		h1->Fit("gaus","RQ0","",xmin,xmax);
		f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
		if (!f) return NULL;
		mean=f->GetParameter(1);
		sigma=f->GetParameter(2); 
		chi2 = f->GetChisquare()/f->GetNDF();
		if(_chi2 > chi2) {
			_chi2 = chi2;
			_nbin0 = nbin0;
			_nbin1 = nbin1;
			//cout<<"Update Chi2 to "<<_chi2<<", mean="<<mean<<", error="<<f->GetParError(1)<<endl;
		}
		if((count++)%2) nbin1 -= 1.0;
		else nbin0 -= 1.0;
		if(minimumbin > kMinBin && minimumbin > 3*sigma/binw) {
			minimumbin = 3*sigma/binw;
			if (minimumbin < kMinBin) minimumbin = kMinBin;
			//cout<<h1->GetName()<<": update minimumbin to "<<minimumbin<<", sigma="<<sigma<<", error="<<f->GetParError(2)<<endl;
		}
	}
	
	//recover the minimum chi2 fit
	xmin = xpeak - _nbin0*binw;
	xmax = xpeak + _nbin1*binw;
	h1->Fit("gaus","RQ","",xmin,xmax);
	f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
	if (!f) return NULL;
	mean=f->GetParameter(1);
	sigma=f->GetParameter(2);
	f->SetLineColor(6);

	if (gStyle->GetOptFit()==0)
	{
		char str[100];
		TText *text=0;

		double xx=gStyle->GetPadLeftMargin()+0.06;
		TPaveText *pt = new TPaveText(xx,0.25,xx+0.45,0.45,"brNDC");
		pt->SetBorderSize(0);
		pt->SetFillColor(0);
		if(gROOT->IsBatch()) pt->SetFillStyle(4000);
		sprintf(str,"Mean = %.3G",mean);
		text=pt->AddText(str);
		text->SetTextColor(6);
		sprintf(str,"Sigma = %.3G",sigma);
		text=pt->AddText(str);
		text->SetTextColor(6);
		pt->Draw("same");
	}

	return f;
}

TF1* FitGausPeak(TH1* h1) 
{
	double mean,sigma;
	return FitGausPeak(h1,mean,sigma);
}

TF1* FitGausPeak(const char* hname) 
{
	double mean,sigma;
	TH1* h1 = (TH1*)gROOT->FindObject(hname);
	return FitGausPeak(h1,mean,sigma);
}


TF1* FitGausRange(TH1* _h1, double xmin, double xmax, double &mean, double &sigma)
{
	//cout<<"histo name="<<h1->GetName()<<" entries="<<h1->GetEntries()<<endl;
	if (_h1->GetEntries()<500) return NULL;
	//make a copy of h1 and keep only the fitting range
	TH1 *h1 = (TH1*)_h1->Clone();
	int n = h1->GetNbinsX();
	for(int i=1;i<=n;i++) {
		if(h1->GetBinLowEdge(i)<xmin || h1->GetBinLowEdge(i)>xmax ) {
			h1->SetBinContent(i,0.0);
		}
	}
	
	double xpeak = h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
	double binw = h1->GetXaxis()->GetBinWidth(1);
	
	double _chi2 = 999999.;
	double _nbin = (xmax-xmin)/2./binw;
	
	TF1* f=0;
	double chi2 = _chi2;
	double nbin = _nbin;
	while ( nbin > 4.0 ) {
		xmin = xpeak - nbin*binw;
		xmax = xpeak + nbin*binw;
		h1->Fit("gaus","RQ","",xmin,xmax);
		f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
		if (!f) return NULL;
		mean=f->GetParameter(1);
		sigma=f->GetParameter(2); 
		chi2 = f->GetChisquare()/f->GetNDF();
		if(_chi2 > chi2) {
			_chi2 = chi2;
			_nbin = nbin;
			//cout<<"Update Chi2 to "<<_chi2<<", mean="<<mean<<", error="<<f->GetParError(1)<<endl;
		}
		nbin -= 0.5;
	}

	//recover the minimum chi2 fit
	xmin = xpeak - _nbin*binw;
	xmax = xpeak + _nbin*binw;
	_h1->Fit("gaus","RQ","",xmin,xmax);
	f=(TF1*)h1->GetListOfFunctions()->FindObject("gaus");
	if (!f) return NULL;
	mean=f->GetParameter(1);
	sigma=f->GetParameter(2);
	f->SetLineColor(6);

	if (gStyle->GetOptFit()==0)
	{
		char str[100];
		TText *text=0;

		double xx=gStyle->GetPadLeftMargin()+0.06;
		TPaveText *pt = new TPaveText(xx,0.25,xx+0.45,0.45,"brNDC");
		pt->SetBorderSize(0);
		pt->SetFillColor(0);
		if(gROOT->IsBatch()) pt->SetFillStyle(4000);
		sprintf(str,"Mean = %.3G",mean);
		text=pt->AddText(str);
		text->SetTextColor(6);
		sprintf(str,"Sigma = %.3G",sigma);
		text=pt->AddText(str);
		text->SetTextColor(6);
		pt->Draw("same");
	}

	return f;
}

TF1* FitGausRange(TH1* h1, double xmin, double xmax) 
{
	double mean,sigma;
	return FitGausRange(h1,xmin,xmax);
}

TF1* FitGausRange(const char* hname, double xmin, double xmax) 
{
	double mean,sigma;
	TH1* h1 = (TH1*)gROOT->FindObject(hname);
	return FitGausRange(h1,xmin,xmax);
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

		double xx=gStyle->GetPadLeftMargin()+0.06;
		TPaveText *pt = new TPaveText(xx,0.25,xx+0.45,0.45,"brNDC");
		pt->SetBorderSize(0);
		pt->SetFillColor(0);
		if(gROOT->IsBatch()) pt->SetFillStyle(4000);
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

TF1* FitGaus(char* hName, double range_in_sigma)
{
	TH1* h1=(TH1 *)gROOT->FindObject(hName);
	double mean, sigma;
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
	fputs (buf, stdout);
	fputs (buf, pFile);

	sprintf(buf,"  const float kSlot8PeakPos_%d[] = {",trigbit);
	for (int i=0;i<15;i++)  sprintf(buf,"%s %4.1f,",buf,peak8[i]);
	sprintf(buf,"%s %4.1f };\n",buf,peak8[15]);
	fputs (buf, stdout);
	fputs (buf, pFile);

	sprintf(buf,"#run %04d, Slot7 and Slot8 trigger bit %d:\n",gRunNumber,trigbit);
	fputs(buf, stdout);
	fputs(buf, pFile);

	sprintf(buf,"Data7_%d =",trigbit);
	for (int i=0;i<16;i++)  sprintf(buf,"%s %4.1f ",buf,peak7[i]);
	sprintf(buf,"%s\n",buf);
	fputs (buf, pFile);

	sprintf(buf,"Data8_%d =",trigbit);
	for (int i=0;i<16;i++)  sprintf(buf,"%s %4.1f ",buf,peak8[i]);
	sprintf(buf,"%s\n",buf);
	fputs (buf, pFile);

	fclose (pFile);
}

void WritePedFile(int run, double *ped7, double *ped8, int printlabel=0)
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
	for (int i=0;i<16;i++)  sprintf(buf,"%s %6.1f",buf,ped7[i]);
	for (int i=0;i<16;i++)  sprintf(buf,"%s %6.1f",buf,ped8[i]);
	sprintf(buf,"%s\n",buf);
	fputs (buf, stdout);
	fputs (buf, pFile);

	fclose (pFile);
}
