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

vector<int> *solid_ec_id=0,*solid_ec_hitn=0;
vector<int> *solid_ec_pid=0,*solid_ec_mpid=0,*solid_ec_tid=0,*solid_ec_mtid=0,*solid_ec_otid=0;
vector<double> *solid_ec_trackE=0,*solid_ec_totEdep=0,*solid_ec_avg_x=0,*solid_ec_avg_y=0,*solid_ec_avg_z=0,*solid_ec_avg_lx=0,*solid_ec_avg_ly=0,*solid_ec_avg_lz=0,*solid_ec_px=0,*solid_ec_py=0,*solid_ec_pz=0,*solid_ec_vx=0,*solid_ec_vy=0,*solid_ec_vz=0,*solid_ec_mvx=0,*solid_ec_mvy=0,*solid_ec_mvz=0,*solid_ec_avg_t=0;
vector<int> *solid_ec_nsteps=0,*solid_ec_procID=0;
vector<double> *solid_ec_totEdepB=0,*solid_ec_totEend=0;
vector<double> *solid_ec_Edep_seg0=0,*solid_ec_Edep_seg1=0,*solid_ec_Edep_seg2=0,*solid_ec_Edep_seg3=0,*solid_ec_Edep_seg4=0,*solid_ec_Edep_seg5=0,*solid_ec_Edep_seg6=0,*solid_ec_Edep_seg7=0,*solid_ec_Edep_seg8=0,*solid_ec_Edep_seg9=0;
vector<double> *solid_ec_EdepB_seg0=0,*solid_ec_EdepB_seg1=0,*solid_ec_EdepB_seg2=0,*solid_ec_EdepB_seg3=0,*solid_ec_EdepB_seg4=0,*solid_ec_EdepB_seg5=0,*solid_ec_EdepB_seg6=0,*solid_ec_EdepB_seg7=0,*solid_ec_EdepB_seg8=0,*solid_ec_EdepB_seg9=0;
vector<double> *solid_ec_Eend_seg0=0,*solid_ec_Eend_seg1=0,*solid_ec_Eend_seg2=0,*solid_ec_Eend_seg3=0,*solid_ec_Eend_seg4=0,*solid_ec_Eend_seg5=0,*solid_ec_Eend_seg6=0,*solid_ec_Eend_seg7=0,*solid_ec_Eend_seg8=0,*solid_ec_Eend_seg9=0;

void setup_tree_solid_ec(TTree *tree_solid_ec)
{  
tree_solid_ec->SetBranchAddress("hitn",&solid_ec_hitn);
tree_solid_ec->SetBranchAddress("id",&solid_ec_id);
tree_solid_ec->SetBranchAddress("pid",&solid_ec_pid);
tree_solid_ec->SetBranchAddress("mpid",&solid_ec_mpid);
tree_solid_ec->SetBranchAddress("tid",&solid_ec_tid);
tree_solid_ec->SetBranchAddress("mtid",&solid_ec_mtid);
tree_solid_ec->SetBranchAddress("otid",&solid_ec_otid);
tree_solid_ec->SetBranchAddress("trackE",&solid_ec_trackE);
tree_solid_ec->SetBranchAddress("totEdep",&solid_ec_totEdep);
tree_solid_ec->SetBranchAddress("avg_x",&solid_ec_avg_x);
tree_solid_ec->SetBranchAddress("avg_y",&solid_ec_avg_y);
tree_solid_ec->SetBranchAddress("avg_z",&solid_ec_avg_z);
tree_solid_ec->SetBranchAddress("avg_lx",&solid_ec_avg_lx);
tree_solid_ec->SetBranchAddress("avg_ly",&solid_ec_avg_ly);
tree_solid_ec->SetBranchAddress("avg_lz",&solid_ec_avg_lz);
tree_solid_ec->SetBranchAddress("px",&solid_ec_px);
tree_solid_ec->SetBranchAddress("py",&solid_ec_py);
tree_solid_ec->SetBranchAddress("pz",&solid_ec_pz);
tree_solid_ec->SetBranchAddress("vx",&solid_ec_vx);
tree_solid_ec->SetBranchAddress("vy",&solid_ec_vy);
tree_solid_ec->SetBranchAddress("vz",&solid_ec_vz);
tree_solid_ec->SetBranchAddress("mvx",&solid_ec_mvx);
tree_solid_ec->SetBranchAddress("mvy",&solid_ec_mvy);
tree_solid_ec->SetBranchAddress("mvz",&solid_ec_mvz);
tree_solid_ec->SetBranchAddress("avg_t",&solid_ec_avg_t);
tree_solid_ec->SetBranchAddress("nsteps",&solid_ec_nsteps);
tree_solid_ec->SetBranchAddress("procID",&solid_ec_procID);
tree_solid_ec->SetBranchAddress("totEdepB",&solid_ec_totEdepB);
tree_solid_ec->SetBranchAddress("totEend",&solid_ec_totEend);
tree_solid_ec->SetBranchAddress("Edep_seg0",&solid_ec_Edep_seg0);
tree_solid_ec->SetBranchAddress("Edep_seg1",&solid_ec_Edep_seg1);
tree_solid_ec->SetBranchAddress("Edep_seg2",&solid_ec_Edep_seg2);
tree_solid_ec->SetBranchAddress("Edep_seg3",&solid_ec_Edep_seg3);
tree_solid_ec->SetBranchAddress("Edep_seg4",&solid_ec_Edep_seg4);
tree_solid_ec->SetBranchAddress("Edep_seg5",&solid_ec_Edep_seg5);
tree_solid_ec->SetBranchAddress("Edep_seg6",&solid_ec_Edep_seg6);
tree_solid_ec->SetBranchAddress("Edep_seg7",&solid_ec_Edep_seg7);
tree_solid_ec->SetBranchAddress("Edep_seg8",&solid_ec_Edep_seg8);
tree_solid_ec->SetBranchAddress("Edep_seg9",&solid_ec_Edep_seg9);
tree_solid_ec->SetBranchAddress("EdepB_seg0",&solid_ec_EdepB_seg0);
tree_solid_ec->SetBranchAddress("EdepB_seg1",&solid_ec_EdepB_seg1);
tree_solid_ec->SetBranchAddress("EdepB_seg2",&solid_ec_EdepB_seg2);
tree_solid_ec->SetBranchAddress("EdepB_seg3",&solid_ec_EdepB_seg3);
tree_solid_ec->SetBranchAddress("EdepB_seg4",&solid_ec_EdepB_seg4);
tree_solid_ec->SetBranchAddress("EdepB_seg5",&solid_ec_EdepB_seg5);
tree_solid_ec->SetBranchAddress("EdepB_seg6",&solid_ec_EdepB_seg6);
tree_solid_ec->SetBranchAddress("EdepB_seg7",&solid_ec_EdepB_seg7);
tree_solid_ec->SetBranchAddress("EdepB_seg8",&solid_ec_EdepB_seg8);
tree_solid_ec->SetBranchAddress("EdepB_seg9",&solid_ec_EdepB_seg9);
tree_solid_ec->SetBranchAddress("Eend_seg0",&solid_ec_Eend_seg0);
tree_solid_ec->SetBranchAddress("Eend_seg1",&solid_ec_Eend_seg1);
tree_solid_ec->SetBranchAddress("Eend_seg2",&solid_ec_Eend_seg2);
tree_solid_ec->SetBranchAddress("Eend_seg3",&solid_ec_Eend_seg3);
tree_solid_ec->SetBranchAddress("Eend_seg4",&solid_ec_Eend_seg4);
tree_solid_ec->SetBranchAddress("Eend_seg5",&solid_ec_Eend_seg5);
tree_solid_ec->SetBranchAddress("Eend_seg6",&solid_ec_Eend_seg6);
tree_solid_ec->SetBranchAddress("Eend_seg7",&solid_ec_Eend_seg7);
tree_solid_ec->SetBranchAddress("Eend_seg8",&solid_ec_Eend_seg8);
tree_solid_ec->SetBranchAddress("Eend_seg9",&solid_ec_Eend_seg9);

return;
}

vector<int> *solid_ec_ps_id=0,*solid_ec_ps_hitn=0;
vector<int> *solid_ec_ps_pid=0,*solid_ec_ps_mpid=0,*solid_ec_ps_tid=0,*solid_ec_ps_mtid=0,*solid_ec_ps_otid=0;
vector<double> *solid_ec_ps_trackE=0,*solid_ec_ps_totEdep=0,*solid_ec_ps_avg_x=0,*solid_ec_ps_avg_y=0,*solid_ec_ps_avg_z=0,*solid_ec_ps_avg_lx=0,*solid_ec_ps_avg_ly=0,*solid_ec_ps_avg_lz=0,*solid_ec_ps_px=0,*solid_ec_ps_py=0,*solid_ec_ps_pz=0,*solid_ec_ps_vx=0,*solid_ec_ps_vy=0,*solid_ec_ps_vz=0,*solid_ec_ps_mvx=0,*solid_ec_ps_mvy=0,*solid_ec_ps_mvz=0,*solid_ec_ps_avg_t=0;
vector<int> *solid_ec_ps_nsteps=0,*solid_ec_ps_procID=0;
vector<double> *solid_ec_ps_totEdepB=0,*solid_ec_ps_totEend=0;


void setup_tree_solid_ec_ps(TTree *tree_solid_ec_ps)
{  
tree_solid_ec_ps->SetBranchAddress("hitn",&solid_ec_ps_hitn);
tree_solid_ec_ps->SetBranchAddress("id",&solid_ec_ps_id);
tree_solid_ec_ps->SetBranchAddress("pid",&solid_ec_ps_pid);
tree_solid_ec_ps->SetBranchAddress("mpid",&solid_ec_ps_mpid);
tree_solid_ec_ps->SetBranchAddress("tid",&solid_ec_ps_tid);
tree_solid_ec_ps->SetBranchAddress("mtid",&solid_ec_ps_mtid);
tree_solid_ec_ps->SetBranchAddress("otid",&solid_ec_ps_otid);
tree_solid_ec_ps->SetBranchAddress("trackE",&solid_ec_ps_trackE);
tree_solid_ec_ps->SetBranchAddress("totEdep",&solid_ec_ps_totEdep);
tree_solid_ec_ps->SetBranchAddress("avg_x",&solid_ec_ps_avg_x);
tree_solid_ec_ps->SetBranchAddress("avg_y",&solid_ec_ps_avg_y);
tree_solid_ec_ps->SetBranchAddress("avg_z",&solid_ec_ps_avg_z);
tree_solid_ec_ps->SetBranchAddress("avg_lx",&solid_ec_ps_avg_lx);
tree_solid_ec_ps->SetBranchAddress("avg_ly",&solid_ec_ps_avg_ly);
tree_solid_ec_ps->SetBranchAddress("avg_lz",&solid_ec_ps_avg_lz);
tree_solid_ec_ps->SetBranchAddress("px",&solid_ec_ps_px);
tree_solid_ec_ps->SetBranchAddress("py",&solid_ec_ps_py);
tree_solid_ec_ps->SetBranchAddress("pz",&solid_ec_ps_pz);
tree_solid_ec_ps->SetBranchAddress("vx",&solid_ec_ps_vx);
tree_solid_ec_ps->SetBranchAddress("vy",&solid_ec_ps_vy);
tree_solid_ec_ps->SetBranchAddress("vz",&solid_ec_ps_vz);
tree_solid_ec_ps->SetBranchAddress("mvx",&solid_ec_ps_mvx);
tree_solid_ec_ps->SetBranchAddress("mvy",&solid_ec_ps_mvy);
tree_solid_ec_ps->SetBranchAddress("mvz",&solid_ec_ps_mvz);
tree_solid_ec_ps->SetBranchAddress("avg_t",&solid_ec_ps_avg_t);
tree_solid_ec_ps->SetBranchAddress("nsteps",&solid_ec_ps_nsteps);
tree_solid_ec_ps->SetBranchAddress("procID",&solid_ec_ps_procID);
tree_solid_ec_ps->SetBranchAddress("totEdepB",&solid_ec_ps_totEdepB);
tree_solid_ec_ps->SetBranchAddress("totEend",&solid_ec_ps_totEend);

return;
}

Bool_t process_tree_solid_ec(TTree *tree_solid_ec,TTree *tree_solid_ec_ps,double &Eend_ec_sum,double &Eend_ec_ps_sum,double *Eend_ec,double *Eend_ec_ps)
{
    for (UInt_t j=0;j<solid_ec_hitn->size();j++) {
//       cout << j<< " solid_ec " << " !!! " << solid_ec_hitn->at(j) << " " << solid_ec_id->at(j) << " " << solid_ec_pid->at(j) << " " << solid_ec_mpid->at(j) << " " << solid_ec_tid->at(j) << " " << solid_ec_mtid->at(j) << " " << solid_ec_trackE->at(j) << " " << solid_ec_totEdep->at(j) << " " << solid_ec_avg_x->at(j) << " " << solid_ec_avg_y->at(j) << " " << solid_ec_avg_z->at(j) << " " << solid_ec_avg_lx->at(j) << " " << solid_ec_avg_ly->at(j) << " " << solid_ec_avg_lz->at(j) << " " << solid_ec_px->at(j) << " " << solid_ec_py->at(j) << " " << solid_ec_pz->at(j) << " " << solid_ec_vx->at(j) << " " << solid_ec_vy->at(j) << " " << solid_ec_vz->at(j) << " " << solid_ec_mvx->at(j) << " " << solid_ec_mvy->at(j) << " " << solid_ec_mvz->at(j) << " " << solid_ec_avg_t->at(j) << endl;    

       int detector_ID=solid_ec_id->at(j)/1000000;
       int subdetector_ID=(solid_ec_id->at(j)%1000000)/100000;
       int subsubdetector_ID=((solid_ec_id->at(j)%1000000)%100000)/10000;
       int component_ID=solid_ec_id->at(j)%10000;  
       //cout << detector_ID << " " << subdetector_ID << " "  << subsubdetector_ID  << " " << component_ID << ", " << solid_ec_avg_z->at(j) << endl;
       Eend_ec_sum += solid_ec_totEend->at(j);
       Eend_ec[component_ID] +=solid_ec_totEend->at(j);
       //cout<<"component_ID="<<component_ID<<"  x="<<solid_ec_avg_lx->at(j)<<"  y="<<solid_ec_avg_ly->at(j)<<endl;    
    }
    
    for (UInt_t j=0;j<solid_ec_ps_hitn->size();j++) {
       int detector_ID_ps=solid_ec_ps_id->at(j)/1000000;
       int subdetector_ID_ps=(solid_ec_ps_id->at(j)%1000000)/100000;
       int subsubdetector_ID_ps=((solid_ec_ps_id->at(j)%1000000)%100000)/10000;
       int component_ID_ps=solid_ec_ps_id->at(j)%10000;  
      Eend_ec_ps_sum += solid_ec_ps_totEend->at(j);
      Eend_ec_ps[component_ID_ps] += solid_ec_ps_totEend->at(j);
    }

return 0;

}

