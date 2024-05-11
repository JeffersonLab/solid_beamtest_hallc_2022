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

vector<int> *solid_spd_id=0,*solid_spd_hitn=0;
vector<int> *solid_spd_pid=0,*solid_spd_mpid=0,*solid_spd_tid=0,*solid_spd_mtid=0,*solid_spd_otid=0;
vector<double> *solid_spd_trackE=0,*solid_spd_totEdep=0,*solid_spd_avg_x=0,*solid_spd_avg_y=0,*solid_spd_avg_z=0,*solid_spd_avg_lx=0,*solid_spd_avg_ly=0,*solid_spd_avg_lz=0,*solid_spd_px=0,*solid_spd_py=0,*solid_spd_pz=0,*solid_spd_vx=0,*solid_spd_vy=0,*solid_spd_vz=0,*solid_spd_mvx=0,*solid_spd_mvy=0,*solid_spd_mvz=0,*solid_spd_avg_t=0;
vector<int> *solid_spd_nsteps=0,*solid_spd_procID=0;
vector<double> *solid_spd_totEdepB=0,*solid_spd_totEend=0;

void setup_tree_solid_spd(TTree *tree_solid_spd)
{  
tree_solid_spd->SetBranchAddress("hitn",&solid_spd_hitn);
tree_solid_spd->SetBranchAddress("id",&solid_spd_id);
tree_solid_spd->SetBranchAddress("pid",&solid_spd_pid);
tree_solid_spd->SetBranchAddress("mpid",&solid_spd_mpid);
tree_solid_spd->SetBranchAddress("tid",&solid_spd_tid);
tree_solid_spd->SetBranchAddress("mtid",&solid_spd_mtid);
tree_solid_spd->SetBranchAddress("otid",&solid_spd_otid);
tree_solid_spd->SetBranchAddress("trackE",&solid_spd_trackE);
tree_solid_spd->SetBranchAddress("totEdep",&solid_spd_totEdep);
tree_solid_spd->SetBranchAddress("avg_x",&solid_spd_avg_x);
tree_solid_spd->SetBranchAddress("avg_y",&solid_spd_avg_y);
tree_solid_spd->SetBranchAddress("avg_z",&solid_spd_avg_z);
tree_solid_spd->SetBranchAddress("avg_lx",&solid_spd_avg_lx);
tree_solid_spd->SetBranchAddress("avg_ly",&solid_spd_avg_ly);
tree_solid_spd->SetBranchAddress("avg_lz",&solid_spd_avg_lz);
tree_solid_spd->SetBranchAddress("px",&solid_spd_px);
tree_solid_spd->SetBranchAddress("py",&solid_spd_py);
tree_solid_spd->SetBranchAddress("pz",&solid_spd_pz);
tree_solid_spd->SetBranchAddress("vx",&solid_spd_vx);
tree_solid_spd->SetBranchAddress("vy",&solid_spd_vy);
tree_solid_spd->SetBranchAddress("vz",&solid_spd_vz);
tree_solid_spd->SetBranchAddress("mvx",&solid_spd_mvx);
tree_solid_spd->SetBranchAddress("mvy",&solid_spd_mvy);
tree_solid_spd->SetBranchAddress("mvz",&solid_spd_mvz);
tree_solid_spd->SetBranchAddress("avg_t",&solid_spd_avg_t);
tree_solid_spd->SetBranchAddress("nsteps",&solid_spd_nsteps);
tree_solid_spd->SetBranchAddress("procID",&solid_spd_procID);
tree_solid_spd->SetBranchAddress("totEdepB",&solid_spd_totEdepB);
tree_solid_spd->SetBranchAddress("totEend",&solid_spd_totEend);

return ;

}

bool find_id_spd_FA(double hit_phi,double hit_r,int &sector,int &block,bool Is_debug=false){  
  double DEG=180./3.1415926;   //rad to degree  
  
  int sec_shift=0;  // shift to match electron turning in field
  if (hit_phi>=90+sec_shift) sector=int((hit_phi-90-sec_shift)/6+1);
  else sector=int((hit_phi+360-90-sec_shift)/6+1);		
   
  //block from 105 to 210cm with 10,20,30,45cm length
  if(105<=hit_r && hit_r<115){
	  block=1;
  }else if(115<=hit_r && hit_r<135){
	  block=2;
  }else if(135<=hit_r && hit_r<165){
	  block=3;
  }else if(165<=hit_r && hit_r<210){
	  block=4;
  }
  //do a check for index
  if(sector<1||sector>60||block<1 || block>4){
	  if (Is_debug) cout<<"spd index is wrong "<<sector<<" "<<block<<endl;
	  return false;
  }
  else return true;
}

bool find_id_spd_LA(double hit_phi,double hit_r,int &sector,bool Is_debug=false){
  double DEG=180./3.1415926;   //rad to degree  
  
  int sec_shift=0;  // shift to match electron turning in field
  if (hit_phi>=90+sec_shift) sector=int((hit_phi-90-sec_shift)/6+1);
  else sector=int((hit_phi+360-90-sec_shift)/6+1);	
  //do a check for index
  if(sector<1||sector>60){
	  if (Is_debug) cout<<"spd index is wrong "<<sector<<endl;
	  return false;
  }
  else return true;
}

bool process_tree_solid_spd(TTree *tree_solid_spd,double *hit_spd_FA,double *hit_spd_LA,int *trigger_spd_FA,int *trigger_spd_LA,int &ntrigsecs_spd_FA,int &ntrigsecs_spd_LA,double spd_threshold_FA =0.5,double spd_threshold_LA=1.5,bool Is_debug=false)
{
    double DEG=180./3.1415926;   //rad to degree  
    
    double tot_edep_spd_forward[60][4]={0};   //only forward has 4 blocks in r dimension    
    double tot_edep_spd_large[60]={0};       
    
    ntrigsecs_spd_FA=0;    
    ntrigsecs_spd_LA=0;		    
       
    //loop over data tree
    for(int j=0; j<solid_spd_hitn->size(); j++){

	    double hit_phi=atan2(solid_spd_avg_y->at(j), solid_spd_avg_x->at(j))*DEG;  //(-180,180)
	    
	    double hit_r=sqrt(solid_spd_avg_y->at(j)*solid_spd_avg_y->at(j)+solid_spd_avg_x->at(j)*solid_spd_avg_x->at(j))/10.; // in cm
		      
	    if(int(solid_spd_id->at(j))==5100000){ //FASPD
	      
		    int sector=0,block=0;
		    if (find_id_spd_FA(hit_phi,hit_r,sector,block))		  tot_edep_spd_forward[sector-1][block-1] += solid_spd_totEdep->at(j);
	    }
	    
	    if(int(solid_spd_id->at(j))==5200000){ //LASPD
	      
		    int sector=0;
		    if(find_id_spd_LA(hit_phi,hit_r,sector)) tot_edep_spd_large[sector-1] += solid_spd_totEdep->at(j);
	    }			
	    
    } //loop over hits
   


    for(int l_sec=0; l_sec< 60; l_sec++){
	for(int l_block=0; l_block<4; l_block++){
	   hit_spd_FA[l_sec*4+l_block] = tot_edep_spd_forward[l_sec][l_block];
	   
		if(tot_edep_spd_forward[l_sec][l_block] >= spd_threshold_FA){
		  ntrigsecs_spd_FA++;
// 		  trigger_spd_FA[l_sec][l_block]=1;
		  trigger_spd_FA[l_sec*4+l_block]=1;		  
		}
	}
    }

    for(int l_sec=0; l_sec< 60; l_sec++){		
      hit_spd_LA[l_sec] = tot_edep_spd_large[l_sec];      
      
      if(tot_edep_spd_large[l_sec] >= spd_threshold_LA){
	ntrigsecs_spd_LA++;				
	trigger_spd_LA[l_sec]=1;
      }
    }
    
    return true;

}
		
double process_tree_solid_spd_simple(TTree *tree_solid_spd,double &Edep_sc1,double &Edep_sc2,double &Edep_spd1,double &Edep_spd2)
{
  double totEdep=0;
//     for (Int_t j=0;j<1;j++) {  
    for (Int_t j=0;j<solid_spd_hitn->size();j++) {
//       cout << "solid_spd " << " !!! " << solid_spd_hitn->at(j) << " " << solid_spd_id->at(j) << " " << solid_spd_pid->at(j) << " " << solid_spd_mpid->at(j) << " " << solid_spd_tid->at(j) << " " << solid_spd_mtid->at(j) << " " << solid_spd_trackE->at(j) << " " << solid_spd_totEdep->at(j) << " " << solid_spd_avg_x->at(j) << " " << solid_spd_avg_y->at(j) << " " << solid_spd_avg_z->at(j) << " " << solid_spd_avg_lx->at(j) << " " << solid_spd_avg_ly->at(j) << " " << solid_spd_avg_lz->at(j) << " " << solid_spd_px->at(j) << " " << solid_spd_py->at(j) << " " << solid_spd_pz->at(j) << " " << solid_spd_vx->at(j) << " " << solid_spd_vy->at(j) << " " << solid_spd_vz->at(j) << " " << solid_spd_mvx->at(j) << " " << solid_spd_mvy->at(j) << " " << solid_spd_mvz->at(j) << " " << solid_spd_avg_t->at(j) << endl;  

//       int detector_ID=solid_spd_id->at(j)/1000000;
//       int subdetector_ID=(solid_spd_id->at(j)%1000000)/100000;
//       int subsubdetector_ID=((solid_spd_id->at(j)%1000000)%100000)/10000;
//       int component_ID=solid_spd_id->at(j)%10000;
      
//     cout << detector_ID << " " << subdetector_ID << " "  << subsubdetector_ID  << " " << component_ID << ", " << solid_spd_totEdep->at(j) << endl; 
           
//       if (detector_ID==5 && subdetector_ID == 1 && subsubdetector_ID == 0) totEdep +=solid_spd_totEdep->at(j);     
      
	if (solid_spd_id->at(j)==1) Edep_sc1 +=  solid_spd_totEdep->at(j);
	if (solid_spd_id->at(j)==2) Edep_sc2 +=  solid_spd_totEdep->at(j);
	if (solid_spd_id->at(j)==3) Edep_spd1 +=  solid_spd_totEdep->at(j);
	if (solid_spd_id->at(j)==4) Edep_spd2 +=  solid_spd_totEdep->at(j);
         
    }    

return 0;

}
		


