//this script is used to create level1 tree for SoLID Ecal beamtest at HallC
//It contains the following functions:
//1)calculate pedestal,
//2) plot the pulse for one event using level0 tree
//3) plot the spectrum ( the pulse shape for a lot of events)
//4) create level1 tree

#define ReadEvTree_cxx

#include "ReadEvTree_cosmic2.h"
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
//#define LEVEL1_Tree_DEBUG 0
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
double gPed7[16]={104.4, 100.5, 100.3,  95.6,  95.2,  99.2, 104.0,  93.2,  96.1,  93.4, 101.2,  93.3,  99.8,  94.9,  93.9,  95.6};
double gPed8[16]={101.6, 105.9, 112.5, 115.8, 113.0, 105.6, 106.6,  98.7,  98.0,  93.4,  96.9, 106.8,  83.6,  89.9, 118.3,  92.8};
 

const char *Slot7Name[16]= {
"CerA0", "CerA1", "CerA2", "CerA3", "CerB0", "CerB1", "CerB2", "CerB3",
"CerC0", "CerC1", "CerC2", "CerC3", "CerD0", "CerD1", "CerD2", "CerD3" };
	 
const char *Slot8Name[16]= {
"SC_bottom", "SC_A", "SC_top", "SC_B", "SC_D", "SC_C", "SC_E", "PreSh_l", "PreSh_r", "PreSh_t",
"Shower_l", "Shower_r", "Shower_t", "PreShSum", "ShowerSum", "Trig" };

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
	
	fdec::Fadc250Data *Slot7Data[16]= {
		CerA0, CerA1, CerA2, CerA3, CerB0, CerB1, CerB2, CerB3,
		CerC0, CerC1, CerC2, CerC3, CerD0, CerD1, CerD2, CerD3 };

	fdec::Fadc250Data *Slot8Data[16]= {
		SC_bottom, SC_A, SC_top, SC_B, SC_D, SC_C, SC_E, PreSh_r, PreSh_l, PreSh_t,
		Shower_r, Shower_l, Shower_t, PreShSum, ShowerSum, Trig };

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

	int EntryNum=0;
	int Slot7Raw[16][100],Slot8Raw[16][100];
	float Slot7ADC[16],Slot8ADC[16];
	float CerASum,CerBSum,CerCSum,CerDSum,CerSum;

	float Slot7Integral0[16],Slot8Integral0[16];
	int Slot7Height[16],Slot8Height[16];
	int Slot7Left0[16], Slot7Right0[16];
	int Slot7Left[16], Slot7Pos[16], Slot7Right[16];
	int Slot8Left0[16], Slot8Right0[16];
	int Slot8Left[16], Slot8Pos[16], Slot8Right[16];

	//Initial the tree leavies values
	for (int i=0;i<16;i++) {
		Slot7ADC[i]=Slot8ADC[i]=0.0;
	}
	CerASum=CerBSum=CerCSum=CerDSum=CerSum=0;
#ifdef  LEVEL1_Tree_DEBUG
	for (int i=0;i<16;i++) {
		Slot7Height[i]=Slot8Height[i]=0;
		Slot7Left0[i]=Slot7Left[i]=Slot7Pos[i]=Slot7Right0[i]=Slot7Right[i]=0;
		Slot8Left0[i]=Slot8Left[i]=Slot8Pos[i]=Slot8Right0[i]=Slot8Right[i]=0;
		Slot7ADC[i]=Slot8ADC[i]=Slot7Integral0[i]=Slot8Integral0[i]=0.0;

		if (LEVEL1_Tree_DEBUG >= 1) {
			for (int j=0; j<100; j++)  Slot7Raw[i][j]=Slot8Raw[i][j]=0;
		}
	}
#endif

#ifdef  LEVEL1_Tree_DEBUG
	T->Branch("Slot7Integral0",&Slot7Integral0,"Slot7Integral0[16]/F");
	T->Branch("Slot7Left0",&Slot7Left0,"Slot7Left0[16]/I");
	T->Branch("Slot7Right0",&Slot7Right0,"Slot7Right0[16]/I");

	T->Branch("Slot7Left",&Slot7Left,"Slot7Left[16]/I");
	T->Branch("Slot7Pos",&Slot7Pos,"Slot7Pos[16]/I");
	T->Branch("Slot7Right",&Slot7Right,"Slot7Right[16]/I");
	T->Branch("Slot7ADC",&Slot7ADC,"Slot7ADC[16]/F");
	T->Branch("Slot7Height",&Slot7Height,"Slot7Height[16]/I");

	T->Branch("Slot8Integral0",&Slot8Integral0,"Slot8Integral0[16]/F");
	T->Branch("Slot8Left0",&Slot8Left0,"Slot8Left0[16]/I");
	T->Branch("Slot8Right0",&Slot8Right0,"Slot8Right0[16]/I");

	T->Branch("Slot8Left",&Slot8Left,"Slot8Left[16]/I");
	T->Branch("Slot8Pos",&Slot8Pos,"Slot8Pos[16]/I");
	T->Branch("Slot8Right",&Slot8Right,"Slot8Right[16]/I");
	T->Branch("Slot8ADC",&Slot8ADC,"Slot8ADC[16]/F");
	T->Branch("Slot8Height",&Slot8Height,"Slot8Height[16]/I");

	if (LEVEL1_Tree_DEBUG >= 1) {
		T->Branch("Slot7Raw",&Slot7Raw,"Slot7Raw[16][100]/I");
		T->Branch("Slot8Raw",&Slot8Raw,"Slot8Raw[16][100]/I");
	}
#endif

	//event_number branch
	T->Branch("event_number",&event_number,"event_number/I");

	T->Branch("Cer",&Slot7ADC,"Cer[16]/F");
	//for (int i=0;i<16;i++) {
	////T->Branch(Slot7Name[i],&Slot7ADC[i],Form("%s/D",Slot7Name[i]));
	//}
	for (int i=0;i<15;i++) {
		T->Branch(Slot8Name[i],&Slot8ADC[i],Form("%s/F",Slot8Name[i]));
	}
	
	T->Branch("CerASum", &CerASum,"CerASum/F");
	T->Branch("CerBSum", &CerBSum,"CerBSum/F");
	T->Branch("CerCSum", &CerCSum,"CerCSum/F");
	T->Branch("CerDSum", &CerDSum,"CerDSum/F");
	T->Branch("CerSum", &CerSum,"CerSum/F");
	
	T->Branch("EntryNum", &EntryNum,"EntryNum/I");
	T->Branch("TrigType", &trigger_type,"TrigType/I");
	T->Branch("trigger_time", &trigger_time, "trigger_time/l");

	////////////////////////////////////////////////////////////////////////
	EcalEvent *pEcalEvent = new EcalEvent();

	T->Branch("Shower_Cluster_AdcSum", &pEcalEvent->Shower_Cluster_AdcSum, "Shower_Cluster_AdcSum/F");
	T->Branch("Shower_Cluster_x", &pEcalEvent->Shower_Cluster_x, "Shower_Cluster_x/F");
	T->Branch("Shower_Cluster_y", &pEcalEvent->Shower_Cluster_y, "Shower_Cluster_y/F");
	T->Branch("Shower_Cluster_z", &pEcalEvent->Shower_Cluster_z, "Shower_Cluster_z/F");
	T->Branch("PreSh_Cluster_AdcSum", &pEcalEvent->PreSh_Cluster_AdcSum, "PreSh_Cluster_AdcSum/F");
	T->Branch("PreSh_Cluster_x", &pEcalEvent->PreSh_Cluster_x, "PreSh_Cluster_x/F");
	T->Branch("PreSh_Cluster_y", &pEcalEvent->PreSh_Cluster_y, "PreSh_Cluster_y/F");
	T->Branch("PreSh_Cluster_z", &pEcalEvent->PreSh_Cluster_z, "PreSh_Cluster_z/F");

	////////////////////////////////////////////////////////////////////////
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

#ifdef   LEVEL1_Tree_DEBUG
	T->Branch("GEM00_x_adc", &GEM00_x_adc[0],"GEM00_x_adc[GEM00_n]/F");
	T->Branch("GEM00_y_adc", &GEM00_y_adc[0],"GEM00_y_adc[GEM00_n]/F");
	T->Branch("GEM01_x_adc", &GEM01_x_adc[0],"GEM01_x_adc[GEM01_n]/F");
	T->Branch("GEM01_y_adc", &GEM01_y_adc[0],"GEM01_y_adc[GEM01_n]/F");
	T->Branch("GEM10_x_adc", &GEM10_x_adc[0],"GEM10_x_adc[GEM10_n]/F");
	T->Branch("GEM10_y_adc", &GEM10_y_adc[0],"GEM10_y_adc[GEM10_n]/F");
	T->Branch("GEM11_x_adc", &GEM11_x_adc[0],"GEM11_x_adc[GEM11_n]/F");
	T->Branch("GEM11_y_adc", &GEM11_y_adc[0],"GEM11_y_adc[GEM11_n]/F");
#endif

	////////////////////////////////////////////////////////////////////////
	// peak position for trig type 1
	const float kSlot7PeakPos[16]={18,18,18,18, 18,18,18,18, 18,18,18,18, 18,18,18,18};
	const float kSlot8PeakPos[16]={24,23,25,23, 24,24,17.5,29, 29,30,33,33, 33.5,25,33,70};

	////////////////////////////////////////////////////////////////////////
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
		//This block has to be excuted at the begining of each event
		//otherwise the 'break' statement will jump over it 
		for (int i=0;i<16;i++) {
			Slot7ADC[i]=Slot8ADC[i]=0.0;
		}
		CerASum=CerBSum=CerCSum=CerDSum=CerSum=0;
		pEcalEvent->Reset();

#ifdef  LEVEL1_Tree_DEBUG
		for (int i=0;i<16;i++) {
			Slot7Height[i]=Slot8Height[i]=0;
			Slot7Left0[i]=Slot7Left[i]=Slot7Pos[i]=Slot7Right0[i]=Slot7Right[i]=0;
			Slot8Left0[i]=Slot8Left[i]=Slot8Pos[i]=Slot8Right0[i]=Slot8Right[i]=0;
			Slot7Integral0[i]=Slot8Integral0[i]=0.0;

			if (LEVEL1_Tree_DEBUG >= 1) {
				for (int j=0; j<100; j++) {Slot7Raw[i][j]=Slot8Raw[i][j]=0;}
			}
		}
#endif

		if (GEM_nCluster>=2) {
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

		//start to process slot 7 and 8
		for (int ich=0; ich<16; ich++) {
			int pos=(int)(Slot7Data[ich]->peaks[0].pos);
			//the next line is to cut out-of-time pulse, however, it will not work if there are more than 1 trigger type
			if (fabs(pos-kSlot7PeakPos[ich]-4)>46.0) continue;

			//find the start and end point of the pulse whenever it reach 1% of the pulse height or adc=2, which is earlier
			float adcCut = (Slot7Data[ich]->raw[pos] - gPed7[ich])*0.01;
			if (adcCut<2) adcCut = 2.0;

			//determine left and right
			float adcSum = 0;
			int iw=0;
			int left=pos, right=pos;
			for (iw = pos-1; iw >= 0; iw--) {
				if (Slot7Data[ich]->raw[iw]>=4095) {adcSum=-100;break;}  //skip saturated event
				float adc = Slot7Data[ich]->raw[iw] - gPed7[ich];
				if (adc<0) adc=0;
				adcSum += adc;
				if (iw<pos-4 && adc<adcCut) break;
				left=iw;
			}
			for (iw = pos; iw < (int)Slot8Data[ich]->raw.size(); iw++) {
				if (Slot7Data[ich]->raw[iw]>=4095) {adcSum=-100;break;}  //skip saturated event
				float adc = Slot7Data[ich]->raw[iw] - gPed7[ich];
				if (adc<0) adc=0;
				adcSum += adc;
				if (iw>pos+4 && adc<adcCut) break;
				right=iw;
			}
			Slot7ADC[ich] = adcSum;

#ifdef  LEVEL1_Tree_DEBUG
			Slot7Left0[ich] = Slot7Data[ich]->peaks[0].left;
			Slot7Right0[ich] = Slot7Data[ich]->peaks[0].right;
			Slot7Integral0[ich] = Slot7Data[ich]->peaks[0].integral;

			Slot7Left[ich] = left;
			Slot7Pos[ich] = pos;
			Slot7Right[ich] = right;
			Slot7Height[ich] = (int)Slot7Data[ich]->peaks[0].height;

			//copy raw data
			if (LEVEL1_Tree_DEBUG >= 1) {
				for (size_t j = 0; j < Slot7Data[ich]->raw.size(); j++) {
					Slot7Raw[ich][j] = Slot7Data[ich]->raw[j] - gPed7[ich];
				}
			}
#endif
		}
		CerASum = Slot7ADC[0]+Slot7ADC[1]+Slot7ADC[2]+Slot7ADC[3];
		CerBSum = Slot7ADC[4]+Slot7ADC[5]+Slot7ADC[6];//+Slot7ADC[3];
		CerCSum = Slot7ADC[8]+Slot7ADC[9]+Slot7ADC[10]+Slot7ADC[11];
		CerDSum = Slot7ADC[12]+Slot7ADC[13]+Slot7ADC[14]+Slot7ADC[15];
		CerSum = CerASum + CerBSum + CerCSum + CerDSum;


		for (int ich=0; ich<15; ich++) {
			//if (ich==13) continue;
			int pos=(int)(Slot8Data[ich]->peaks[0].pos);
			//the next line is to cut out-of-time pulse, however, it will not work if there are more than 1 trigger type
			if (fabs(pos-kSlot8PeakPos[ich]-4)>46.0) continue;

			//find the start and end point of the pulse whenever it reach 1% of the pulse height or adc=2, which is earlier
			float adcCut = (Slot8Data[ich]->raw[pos] - gPed8[ich])*0.01;
			if (adcCut<2) adcCut = 2.0;

			//determine left and right
			float adcSum = 0;
			int iw=0;
			int left=pos, right=pos;
			for (iw = pos-1; iw >= 0; iw--) {
				if (Slot8Data[ich]->raw[iw]>=4095) {adcSum=-100;break;}  //skip saturated event
				float adc = Slot8Data[ich]->raw[iw] - gPed8[ich];
				if (adc<0) adc=0;
				adcSum += adc;
				if (iw<pos-4 && adc<adcCut) break;
				left=iw;
			}
			for (iw = pos; iw < (int)Slot8Data[ich]->raw.size(); iw++) {
				if (Slot8Data[ich]->raw[iw]>=4095) {adcSum=-100;break;}  //skip saturated event
				float adc = Slot8Data[ich]->raw[iw] - gPed8[ich];
				if (adc<0) adc=0;
				adcSum += adc;
				if (iw>pos+4 && adc<adcCut) break;
				right=iw;
			}
			Slot8ADC[ich] = adcSum;

#ifdef  LEVEL1_Tree_DEBUG
			Slot8Left0[ich] = Slot8Data[ich]->peaks[0].left;
			Slot8Right0[ich] = Slot8Data[ich]->peaks[0].right;
			Slot8Integral0[ich] = Slot8Data[ich]->peaks[0].integral;

			Slot8Left[ich] = left;
			Slot8Pos[ich] = pos;
			Slot8Right[ich] = right;
			Slot8Height[ich] = (int)Slot8Data[ich]->peaks[0].height;

			//copy raw data
			if (LEVEL1_Tree_DEBUG >= 1) {
				for (size_t j = 0; j < Slot8Data[ich]->raw.size(); j++) {
					Slot8Raw[ich][j] = Slot8Data[ich]->raw[j] - gPed8[ich];
				}
			}
#endif
		}
		///////////Ecal cluster weighting///////////////////////////////////
		pEcalEvent->DoWeighting(Slot8ADC[7],Slot8ADC[8],Slot8ADC[9],Slot8ADC[10],Slot8ADC[11],Slot8ADC[12]);

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
	
	fdec::Fadc250Data *Slot7Data[16]= {
		CerA0, CerA1, CerA2, CerA3, CerB0, CerB1, CerB2, CerB3,
		CerC0, CerC1, CerC2, CerC3, CerD0, CerD1, CerD2, CerD3 };

	fdec::Fadc250Data *Slot8Data[16]= {
		SC_bottom, SC_A, SC_top, SC_B, SC_D, SC_C, SC_E, PreSh_r, PreSh_l, PreSh_t,
		Shower_r, Shower_l, Shower_t, PreShSum, ShowerSum, Trig };

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
	
	fdec::Fadc250Data *Slot7Data[16]= {
		CerA0, CerA1, CerA2, CerA3, CerB0, CerB1, CerB2, CerB3,
		CerC0, CerC1, CerC2, CerC3, CerD0, CerD1, CerD2, CerD3 };

	fdec::Fadc250Data *Slot8Data[16]= {
		SC_bottom, SC_A, SC_top, SC_B, SC_D, SC_C, SC_E, PreSh_r, PreSh_l, PreSh_t,
		Shower_r, Shower_l, Shower_t, PreShSum, ShowerSum, Trig };
		
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
	
	fdec::Fadc250Data *Slot7Data[16]= {
		CerA0, CerA1, CerA2, CerA3, CerB0, CerB1, CerB2, CerB3,
		CerC0, CerC1, CerC2, CerC3, CerD0, CerD1, CerD2, CerD3 };

	fdec::Fadc250Data *Slot8Data[16]= {
		SC_bottom, SC_A, SC_top, SC_B, SC_D, SC_C, SC_E, PreSh_r, PreSh_l, PreSh_t,
		Shower_r, Shower_l, Shower_t, PreShSum, ShowerSum, Trig };

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

void run_cosmic2(int istart=0, int iend=-1)
{
	run(istart,iend);
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
		pedready = false;
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

void DoLevel1Tree_cosmic2(int run_start, int run_end)
{
	DoLevel1Tree(run_start, run_end);	
}
