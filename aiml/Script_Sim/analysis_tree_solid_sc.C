#include <iostream> 
#include <fstream>
#include <cmath> 
#include <math.h> 
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TLorentzVector.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TMinuit.h>
#include <TPaveText.h>
#include <TText.h>
#include <TSystem.h>
#include <TArc.h>
#include <TMath.h>

using namespace std;

vector<int> *solid_sc_id=0,*solid_sc_hitn=0;
vector<int> *solid_sc_pid=0,*solid_sc_mpid=0,*solid_sc_tid=0,*solid_sc_mtid=0,*solid_sc_otid=0;
vector<double> *solid_sc_trackE=0,*solid_sc_totEdep=0,*solid_sc_avg_x=0,*solid_sc_avg_y=0,*solid_sc_avg_z=0,*solid_sc_avg_lx=0,*solid_sc_avg_ly=0,*solid_sc_avg_lz=0,*solid_sc_px=0,*solid_sc_py=0,*solid_sc_pz=0,*solid_sc_vx=0,*solid_sc_vy=0,*solid_sc_vz=0,*solid_sc_mvx=0,*solid_sc_mvy=0,*solid_sc_mvz=0,*solid_sc_avg_t=0;
vector<int> *solid_sc_nsteps=0,*solid_sc_procID=0;
vector<double> *solid_sc_totEdepB=0,*solid_sc_totEend=0;

void setup_tree_solid_sc(TTree *tree_solid_spd)
{  
tree_solid_sc->SetBranchAddress("hitn",&solid_sc_hitn);
tree_solid_sc->SetBranchAddress("id",&solid_sc_id);
tree_solid_sc->SetBranchAddress("pid",&solid_sc_pid);
tree_solid_sc->SetBranchAddress("mpid",&solid_sc_mpid);
tree_solid_sc->SetBranchAddress("tid",&solid_sc_tid);
tree_solid_sc->SetBranchAddress("mtid",&solid_sc_mtid);
tree_solid_sc->SetBranchAddress("otid",&solid_sc_otid);
tree_solid_sc->SetBranchAddress("trackE",&solid_sc_trackE);
tree_solid_sc->SetBranchAddress("totEdep",&solid_sc_totEdep);
tree_solid_sc->SetBranchAddress("avg_x",&solid_sc_avg_x);
tree_solid_sc->SetBranchAddress("avg_y",&solid_sc_avg_y);
tree_solid_sc->SetBranchAddress("avg_z",&solid_sc_avg_z);
tree_solid_sc->SetBranchAddress("avg_lx",&solid_sc_avg_lx);
tree_solid_sc->SetBranchAddress("avg_ly",&solid_sc_avg_ly);
tree_solid_sc->SetBranchAddress("avg_lz",&solid_sc_avg_lz);
tree_solid_sc->SetBranchAddress("px",&solid_sc_px);
tree_solid_sc->SetBranchAddress("py",&solid_sc_py);
tree_solid_sc->SetBranchAddress("pz",&solid_sc_pz);
tree_solid_sc->SetBranchAddress("vx",&solid_sc_vx);
tree_solid_sc->SetBranchAddress("vy",&solid_sc_vy);
tree_solid_sc->SetBranchAddress("vz",&solid_sc_vz);
tree_solid_sc->SetBranchAddress("mvx",&solid_sc_mvx);
tree_solid_sc->SetBranchAddress("mvy",&solid_sc_mvy);
tree_solid_sc->SetBranchAddress("mvz",&solid_sc_mvz);
tree_solid_sc->SetBranchAddress("avg_t",&solid_sc_avg_t);
tree_solid_sc->SetBranchAddress("nsteps",&solid_sc_nsteps);
tree_solid_sc->SetBranchAddress("procID",&solid_sc_procID);
tree_solid_sc->SetBranchAddress("totEdepB",&solid_sc_totEdepB);
tree_solid_sc->SetBranchAddress("totEend",&solid_sc_totEend);

return ;

}
		
double process_tree_solid_sc_simple(TTree *tree_solid_sc,double &Edep_sc1,double &Edep_sc2,double &Edep_sc1,double &Edep_sc2)
{
  double totEdep=0;
//     for (Int_t j=0;j<1;j++) {  
    for (Int_t j=0;j<solid_sc_hitn->size();j++) {
//       cout << "solid_sc " << " !!! " << solid_sc_hitn->at(j) << " " << solid_sc_id->at(j) << " " << solid_sc_pid->at(j) << " " << solid_sc_mpid->at(j) << " " << solid_sc_tid->at(j) << " " << solid_sc_mtid->at(j) << " " << solid_sc_trackE->at(j) << " " << solid_sc_totEdep->at(j) << " " << solid_sc_avg_x->at(j) << " " << solid_sc_avg_y->at(j) << " " << solid_sc_avg_z->at(j) << " " << solid_sc_avg_lx->at(j) << " " << solid_sc_avg_ly->at(j) << " " << solid_sc_avg_lz->at(j) << " " << solid_sc_px->at(j) << " " << solid_sc_py->at(j) << " " << solid_sc_pz->at(j) << " " << solid_sc_vx->at(j) << " " << solid_sc_vy->at(j) << " " << solid_sc_vz->at(j) << " " << solid_sc_mvx->at(j) << " " << solid_sc_mvy->at(j) << " " << solid_sc_mvz->at(j) << " " << solid_sc_avg_t->at(j) << endl;  

//       int detector_ID=solid_sc_id->at(j)/1000000;
//       int subdetector_ID=(solid_sc_id->at(j)%1000000)/100000;
//       int subsubdetector_ID=((solid_sc_id->at(j)%1000000)%100000)/10000;
//       int component_ID=solid_sc_id->at(j)%10000;
      
//     cout << detector_ID << " " << subdetector_ID << " "  << subsubdetector_ID  << " " << component_ID << ", " << solid_sc_totEdep->at(j) << endl; 
           
//       if (detector_ID==5 && subdetector_ID == 1 && subsubdetector_ID == 0) totEdep +=solid_sc_totEdep->at(j);     
      
	if (solid_sc_id->at(j)==1) Edep_sc1 +=  solid_sc_totEdep->at(j);
	if (solid_sc_id->at(j)==2) Edep_sc2 +=  solid_sc_totEdep->at(j);
	if (solid_spd_id->at(j)==3) Edep_spd1 +=  solid_spd_totEdep->at(j);
	if (solid_spd_id->at(j)==4) Edep_spd2 +=  solid_spd_totEdep->at(j);
         
    }    

return 0;

}
		


