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
#include <TRandom3.h>

using namespace std;

double PhotonEnergy[42]={
2.04358, 2.0664, 2.09046, 2.14023, 
2.16601, 2.20587, 2.23327, 2.26137, 
2.31972, 2.35005, 2.38116, 2.41313, 
2.44598, 2.47968, 2.53081, 2.58354, 
2.6194, 2.69589, 2.73515, 2.79685, 
2.86139, 2.95271, 3.04884, 3.12665, 
3.2393, 3.39218, 3.52508, 3.66893,
3.82396, 3.99949, 4.13281, 4.27679, 
4.48244, 4.65057, 4.89476, 5.02774, 
5.16816, 5.31437, 5.63821, 5.90401, 
6.19921,6.49921,
};  // in ev

const int n=41;
double QE_H8500_03[n] = {
0.008, 0.0124, 0.0157, 0.02125, 
0.0275, 0.034, 0.04, 0.048, 
0.062, 0.0753, 0.09, 0.1071, 
0.12144, 0.1428, 0.15, 0.16429, 
0.17857, 0.1928, 0.2, 0.2125,
0.225, 0.2375, 0.25, 0.2625, 
0.275, 0.275, 0.275, 0.275, 
0.275, 0.275, 0.2625, 0.25, 
0.2375, 0.2125, 0.192859, 0.185716, 
0.178573, 0.15714, 0.13572, 0.1143,
0.09  
}; 
double QE_H12700_03[n] = {
0.016,0.02,0.025,0.033,
0.042,0.048,0.056,0.06,
0.075,0.085,0.096,0.121,
0.147,0.166,0.182,0.194,
0.203,0.22,0.238,0.253,
0.269,0.287,0.3,0.31,
0.32,0.33,0.335,0.335,
0.335,0.33,0.325,0.31,
0.296,0.282,0.257,0.237,
0.22,0.197,0.165,0.139,
0.114
};
double QE_H12700_03_WLS_meas[n] = {
0.016,0.02,0.0243455,0.0349796,0.0400769,0.0495496,0.054666,0.0612895,0.0758019,0.0853365,0.100662,0.121331,0.144678,0.162644,0.180719,0.194414,0.202599,0.224051,0.235051,0.253334,0.268143,0.285398,0.30002,0.309013,0.319247,0.328839,0.333333,0.335,0.33337,0.327161,0.321697,0.328776,0.333637,0.318123,0.313051,0.326953,0.331335,0.331335,0.331335,0.331335,0.331335
};

double QE_LAPPD_WLS_meas[n] = {
//3.86e-2,3.05e-2,2.73e-2,
2.87e-2,3.23e-2,3.48e-2,4.20e-2,5.42e-2,6.39e-2,7.26e-2,9.06e-2,10.68e-2,11.52e-2,12.05e-2,12.88e-2,13.59e-2,14.36e-2,14.77e-2,15.06e-2,15.54e-2,16.12e-2,16.45e-2,16.19e-2,15.77e-2,15.47e-2,15.31e-2,15.35e-2,15.43e-2,15.32e-2,15.12e-2,15.17e-2,14.48e-2,13.92e-2,13.23e-2,13.57e-2,14.43e-2,14.23e-2,13.93e-2,14.23e-2,14.02e-2,14.36e-2,14.74e-2,15.14e-2,13.09e-2
};

// double *eff_PMT=QE_H12700_03_WLS_meas;
double *eff_PMT=QE_LAPPD_WLS_meas;

//safety factor
//PMT and assmbly effective area
//for pion, manual reduce 2
// double factor=0.8*0.6;
double factor=0.8;

vector<int> *solid_hgc_id=0,*solid_hgc_hitn=0;
vector<int> *solid_hgc_pid=0,*solid_hgc_mpid=0,*solid_hgc_tid=0,*solid_hgc_mtid=0,*solid_hgc_otid=0;
vector<double> *solid_hgc_trackE=0,*solid_hgc_totEdep=0,*solid_hgc_avg_x=0,*solid_hgc_avg_y=0,*solid_hgc_avg_z=0,*solid_hgc_avg_lx=0,*solid_hgc_avg_ly=0,*solid_hgc_avg_lz=0,*solid_hgc_px=0,*solid_hgc_py=0,*solid_hgc_pz=0,*solid_hgc_vx=0,*solid_hgc_vy=0,*solid_hgc_vz=0,*solid_hgc_mvx=0,*solid_hgc_mvy=0,*solid_hgc_mvz=0,*solid_hgc_avg_t=0;

void setup_tree_solid_hgc(TTree *tree_solid_hgc)
{  
tree_solid_hgc->SetBranchAddress("hitn",&solid_hgc_hitn);
tree_solid_hgc->SetBranchAddress("id",&solid_hgc_id);
tree_solid_hgc->SetBranchAddress("pid",&solid_hgc_pid);
tree_solid_hgc->SetBranchAddress("mpid",&solid_hgc_mpid);
tree_solid_hgc->SetBranchAddress("tid",&solid_hgc_tid);
tree_solid_hgc->SetBranchAddress("mtid",&solid_hgc_mtid);
tree_solid_hgc->SetBranchAddress("otid",&solid_hgc_otid);
tree_solid_hgc->SetBranchAddress("trackE",&solid_hgc_trackE);
tree_solid_hgc->SetBranchAddress("totEdep",&solid_hgc_totEdep);
tree_solid_hgc->SetBranchAddress("avg_x",&solid_hgc_avg_x);
tree_solid_hgc->SetBranchAddress("avg_y",&solid_hgc_avg_y);
tree_solid_hgc->SetBranchAddress("avg_z",&solid_hgc_avg_z);
tree_solid_hgc->SetBranchAddress("avg_lx",&solid_hgc_avg_lx);
tree_solid_hgc->SetBranchAddress("avg_ly",&solid_hgc_avg_ly);
tree_solid_hgc->SetBranchAddress("avg_lz",&solid_hgc_avg_lz);
tree_solid_hgc->SetBranchAddress("px",&solid_hgc_px);
tree_solid_hgc->SetBranchAddress("py",&solid_hgc_py);
tree_solid_hgc->SetBranchAddress("pz",&solid_hgc_pz);
tree_solid_hgc->SetBranchAddress("vx",&solid_hgc_vx);
tree_solid_hgc->SetBranchAddress("vy",&solid_hgc_vy);
tree_solid_hgc->SetBranchAddress("vz",&solid_hgc_vz);
tree_solid_hgc->SetBranchAddress("mvx",&solid_hgc_mvx);
tree_solid_hgc->SetBranchAddress("mvy",&solid_hgc_mvy);
tree_solid_hgc->SetBranchAddress("mvz",&solid_hgc_mvz);
tree_solid_hgc->SetBranchAddress("avg_t",&solid_hgc_avg_t);

return;
}

// Bool_t process_tree_solid_hgc(TTree *tree_solid_hgc,double *hit_hgc,Int_t *trigger_hgc, Int_t &ntrigsecs_hgc, Int_t PMTthresh_hgc, Int_t PEthresh_hgc,Int_t ch_hgc)
Bool_t process_tree_solid_hgc(TTree *tree_solid_hgc,double *hit_hgc,Int_t *trigger_hgc, Int_t &ntrigsecs_hgc, Int_t PMTthresh_hgc, Int_t PEthresh_hgc,Int_t ch_hgc,TH1F *htime_photon,Int_t &photon_mtid)
{ 
  	TRandom3 rand;
	rand.SetSeed(0);
	
int sensor_hgc = 16;
//int sensor_hgc = ch_hgc;
int sensor_trans_hgc = sqrt(sensor_hgc);
// cout << sensor_hgc << " " << sensor_trans_hgc << endl;

  double npe_hgc=0;
  
  int counter_in=0,counter_good=0, counter_out=0;  
  double count_photon=0;  
    for (Int_t j=0;j<solid_hgc_hitn->size();j++) {
//       cout << "solid_hgc " << " !!! " << solid_hgc_hitn->at(j) << " " << solid_hgc_id->at(j) << " " << solid_hgc_pid->at(j) << " " << solid_hgc_mpid->at(j) << " " << solid_hgc_tid->at(j) << " " << solid_hgc_mtid->at(j) << " " << solid_hgc_trackE->at(j) << " " << solid_hgc_totEdep->at(j) << " " << solid_hgc_avg_x->at(j) << " " << solid_hgc_avg_y->at(j) << " " << solid_hgc_avg_z->at(j) << " " << solid_hgc_avg_lx->at(j) << " " << solid_hgc_avg_ly->at(j) << " " << solid_hgc_avg_lz->at(j) << " " << solid_hgc_px->at(j) << " " << solid_hgc_py->at(j) << " " << solid_hgc_pz->at(j) << " " << solid_hgc_vx->at(j) << " " << solid_hgc_vy->at(j) << " " << solid_hgc_vz->at(j) << " " << solid_hgc_mvx->at(j) << " " << solid_hgc_mvy->at(j) << " " << solid_hgc_mvz->at(j) << " " << solid_hgc_avg_t->at(j) << endl;  

//       if (solid_hgc_pid->at(j)==0) {count_photon++;}
      if (solid_hgc_pid->at(j)==-22) {
          count_photon++;
//          cout<<"solid_hgc_pid="<<solid_hgc_pid->at(j)<<endl;
       }    // >= geant4.10.7
      else {
       //else cout << "pid wrong " << endl;  //there are many
	continue;
      }
      
      int detector_ID=solid_hgc_id->at(j)/1000000;
      int subdetector_ID=(solid_hgc_id->at(j)%1000000)/100000;
      int subsubdetector_ID=((solid_hgc_id->at(j)%1000000)%100000)/10000;
      int component_ID=solid_hgc_id->at(j)%10000;
      
     //cout << detector_ID << " " << subdetector_ID << " "  << subsubdetector_ID  << " " << component_ID << ", " << solid_hgc_avg_lx->at(j) << ", " << solid_hgc_avg_ly->at(j) << endl; 
     
//       if (detector_ID==2 && subdetector_ID == 2 && subsubdetector_ID == 1) {	  //1st sector only
      if (detector_ID==2){ //all sectors
	  double E_photon=solid_hgc_trackE->at(j)*1e6; //in eV
	  //double E_photon=solid_hgc_trackE->at(j); //in eV
//     cout << detector_ID << " " << subdetector_ID << " "  << subsubdetector_ID  << " " << component_ID << ", " << solid_hgc_avg_lx->at(j) << ", " << solid_hgc_avg_ly->at(j)<<"    E_photon="<<E_photon << endl; 
	  
	  //simulation shouldn't produce any photon beyond the range
//	  if (E_photon<PhotonEnergy[0] || PhotonEnergy[n]<E_photon) cout << "E_photon " << E_photon << endl;
// 	  cout << "E_photon " << E_photon << endl;
	  double weight=0;
	  bool Is_pass=false;
	  for (Int_t k=0;k<n;k++) {	      
// 	  for (Int_t k=0;k<25;k++) {	 // cut on 360nm/3.35eV
	    if (PhotonEnergy[k]<=E_photon && E_photon<PhotonEnergy[k+1]) {
	    counter_in++;
// 	    weight=1;
	    weight=eff_PMT[k]*factor;
// 	    if (solid_hgc_mtid->at(j)!=1 && solid_hgc_mpid->at(j)!=13 ) {weight=eff_PMT[k]*factor;counter_good++;
// 	      cout << "2 " << solid_hgc_mtid->at(j) << " " << solid_hgc_mpid->at(j) << endl;	    
// 	      cout << "mother " << solid_hgc_mtid->at(j) << " " << solid_hgc_mpid->at(j) << " " << solid_hgc_mvz->at(j) << endl;
// 	    }
	    if (rand.Uniform(0,1)<weight) Is_pass=true;
	    break;
	    }
	  }
	  
	  if (Is_pass){
	      npe_hgc += 1;

    // 	  int sector=solid_hgc_id->at(j)/100000-22;  //wrong id matching
// 	      int sector=solid_hgc_id->at(j)/10000-220-1;  //match id 2210000 - 2500000
	      int sector=0;  //match id 2210000 - 2500000	      
	      
// 	  int pmt_x=int((solid_hgc_avg_lx->at(j)-(-106.6))/(106.6/(sensor_trans_hgc/2))),pmt_y=int((solid_hgc_avg_ly->at(j)-(-106.6))/(106.6/(sensor_trans_hgc/2)));
	  int pmt_x=int((solid_hgc_avg_lx->at(j)-(-53.))/(53./(sensor_trans_hgc/2))),pmt_y=int((solid_hgc_avg_ly->at(j)-(-53.))/(53./(sensor_trans_hgc/2)));

	      if(0<=sector && sector<30 && 0<=pmt_x && pmt_x<sensor_trans_hgc && 0<=pmt_y && pmt_y<sensor_trans_hgc){	    
		hit_hgc[sensor_hgc*sector+sensor_trans_hgc*pmt_y+pmt_x] += 1;	
		
		htime_photon->Fill(solid_hgc_avg_t->at(j));
		photon_mtid=solid_hgc_mtid->at(j);
 //		cout << "photon_mtid " << photon_mtid << endl;
//   	  cout << sector << " " << sensor_hgc*sector+sensor_trans_hgc*pmt_y+pmt_x << " " << solid_hgc_avg_lx->at(j) << " " << pmt_x << " " << solid_hgc_avg_ly->at(j) << " " << pmt_y << " " << solid_hgc_avg_x->at(j) << " " << " " << solid_hgc_avg_y->at(j) << " " << solid_hgc_avg_z->at(j) << endl;    
		
    // 	    hit_hgc[16*sector+4*(3-pmt_y)+pmt_x] += weight;		
	      }
	      else cout << "wrong sector or pmt " << sector << " " << solid_hgc_avg_lx->at(j) << " " << pmt_x << " " << solid_hgc_avg_ly->at(j) << " " << pmt_y << endl;
	  }
      }
      else {
	cout << "hit id wrong " << solid_hgc_id->at(j) << endl;
      }
    }
  
  for(UInt_t i = 0; i < 30; i++){
    Int_t ntrigpmts_hgc =0;
    for(Int_t j = 0; j < sensor_hgc; j++){       
      if(hit_hgc[i*sensor_hgc+j] >= PEthresh_hgc) ntrigpmts_hgc++;    
    }
    trigger_hgc[i]=ntrigpmts_hgc;    
    if(ntrigpmts_hgc >= PMTthresh_hgc) {
      ntrigsecs_hgc++;
    }
  }
  if(ntrigsecs_hgc){
    return true;
  }else{
    return false;
  }    
   
//     if (npe_hgc>0)  cout << "solid_hgc_hitn->size() " << solid_hgc_hitn->size() << " " << count_photon << " " << counter_in << " " << counter_good << endl;
    
// if (npe_hgc>0) return true;
// else return false;

}
