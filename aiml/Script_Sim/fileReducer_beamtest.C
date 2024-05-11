#include <iostream> 
#include <fstream>
#include <cmath> 
#include "math.h" 
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TH1F.h"
#include "TLorentzVector.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TMinuit.h"
#include "TPaveText.h"
#include "TText.h"
#include "TSystem.h"
#include "TArc.h"
#include "TString.h"
#include <vector>
#include "TRandom3.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TFile.h"

using namespace std;

#include "analysis_tree_solid_hgc.C"
#include "analysis_tree_solid_ec.C"
#include "analysis_tree_solid_spd.C"
#include "analysis_tree_solid_gem.C"
#define MAX_CLUSTERS_PER_PLANE 2000
// some numbers to be hard coded 
// make sure they are correct while using this script
//################################################################################################################################################## 

const double DEG=180./3.1415926;   //rad to degree

//#####################################################################################################################################################

//input:
//    infile: the path of input root file from GEMC_SOLID
//    numberofFile: how many number of files in the infile, usually 1.0E4 events per file,
//                  for beam on target root tree, 1.0E9 corresponds to 80 triggers, use 80 for it
//    key:  the string used to tell what kind of run it is
//    evgen: event type, 0 is beam on target, 1 is eDIS, 2 is eAll, 3 is bggen, 4 is even file 
int fileReducer_beamtest(string inputfile_name,int numberOfFile=1, int event_actual=1, int evgen=1){

	char the_filename[500];
	sprintf(the_filename, "%s",inputfile_name.substr(0,inputfile_name.rfind(".")).c_str());
	TFile* outFile = new TFile(Form("%s_reduce_tree_rate.root",the_filename), "RECREATE");
	TTree* outTree = new TTree("T", "HallC beam test simulation tree");
	ofstream outfile1(Form("%s_output.csv",the_filename));
	
    cout<<"numberOfFile="<<numberOfFile<<"  event_actual="<<event_actual<<"  evgen="<<evgen<<endl;
    
	float px_gen = 0;
	float py_gen = 0;
	float pz_gen = 0;
	float vx_gen = 0;
	float vy_gen = 0;
	float vz_gen = 0;
	int pid_gen = 0;

	float p_gen=0, theta_gen=0, phi_gen=0;

	float rate = 0;
	float rateRad = 0;

	int ecN=0;
	float PreShP,PreShPx,PreShPy,PreShPz,PreShSum, PreSh_l, PreSh_r, PreSh_t;
	//  float ShP,ShPx,ShPy,ShPz;
	float ShowerSum, Shower_l, Shower_r, Shower_t;
	float LASPD_Eendsum, SPD_Eendsum, SC0_Eendsum, SC1_Eendsum;
	float LASPD_Eend, SPD_Eend, SC0_Eend, SC1_Eend;
	float SC0_P;
	float SC1_P;
	float SPD_P;
	float LASPD_P;
	int GEM00_n,GEM10_n;
	int GEM00_np,GEM10_np;
	float GEM00_x[MAX_CLUSTERS_PER_PLANE],GEM00_y[MAX_CLUSTERS_PER_PLANE];
	float GEM10_x[MAX_CLUSTERS_PER_PLANE],GEM10_y[MAX_CLUSTERS_PER_PLANE];
	float GEM00_vx[MAX_CLUSTERS_PER_PLANE],GEM00_vy[MAX_CLUSTERS_PER_PLANE];
	float GEM10_vx[MAX_CLUSTERS_PER_PLANE],GEM10_vy[MAX_CLUSTERS_PER_PLANE];

	outTree->Branch("rate",       &rate,       "rate/F"      ); //vx at vertex
	outTree->Branch("vx",       &vx_gen,       "vx/F"      ); //vx at vertex
	outTree->Branch("vy",       &vy_gen,       "vy/F"      ); //vy at vertex
	outTree->Branch("vz",       &vz_gen,       "vz/F"      ); //vz at vertex
	outTree->Branch("px",       &px_gen,       "px/F"      ); //px at vertex
	outTree->Branch("py",       &py_gen,       "py/F"      ); //py at vertex
	outTree->Branch("pz",       &pz_gen,       "pz/F"      ); //pz at vertex
	outTree->Branch("p",        &p_gen,        "p/F"       ); //ptot at vertex
	outTree->Branch("pid",      &pid_gen,        "pid/I"       ); //ptot at vertex
	// preshower tree
	outTree->Branch("PreShP",      &PreShP,   "PreShP/F"    ); //momentum hit on the virtual plane of presh 
	outTree->Branch("PreShPx",      &PreShPx,   "PreShPx/F"    ); //momentum px hit on the virtual plane of presh
	outTree->Branch("PreShPy",      &PreShPy,   "PreShPy/F"    ); //momentum py hit on the virtual plane of presh
	outTree->Branch("PreShPz",      &PreShPz,   "PreShPz/F"    ); //momentum pz hit on the virtual plane of presh
	outTree->Branch("PreShSum",      &PreShSum,   "PreShSum/F"    ); //Sum of the deposit energy in three preshower modules
	outTree->Branch("PreSh_l",      &PreSh_l,   "PreSh_l/F"    ); //Deposit energy in the left preshower module
	outTree->Branch("PreSh_r",      &PreSh_r,   "PreSh_r/F"    ); //Deposit energy in the right preshower module
	outTree->Branch("PreSh_t",      &PreSh_t,   "PreSh_t/F"    ); //Deposit energy in the top preshower module
	// shower tree
	outTree->Branch("ShowerSum",      &ShowerSum,   "ShowerSum/F"    ); //Sum of the deposit energy in three shower modules
	outTree->Branch("Shower_l",      &Shower_l,   "Shower_l/F"    ); //Deposit energy in the left shower module
	outTree->Branch("Shower_r",      &Shower_r,   "Shower_r/F"    ); //Deposit energy in the right shower module
	outTree->Branch("Shower_t",      &Shower_t,   "Shower_t/F"    ); //Deposit energy in the top shower module
	outTree->Branch("SC0_P",      &SC0_P,   "SC0_P/F"    ); //momentum hit on the virtual plane of SC0
	outTree->Branch("SC0_Eendsum",      &SC0_Eendsum,   "SC0_Eendsum/F"    ); //total Eendosit energy in the SC0
	outTree->Branch("SC0_Eend",      &SC0_Eend,   "SC0_Eend/F"    ); //prime partile deposit energy in the SC0
	outTree->Branch("SC1_P",      &SC1_P,   "SC1_P/F"    ); //momentum hit on the virtual plane of SC1
	outTree->Branch("SC1_Eendsum",      &SC1_Eendsum,   "SC1_Eendsum/F"    ); //Deposit energy in the SC1
	outTree->Branch("SC1_Eend",      &SC1_Eend,   "SC1_Eend/F"    ); //prime particle deposit energy in the SC1
	outTree->Branch("SPD_P",      &SPD_P,   "SPD_P/F"    ); //momentum hit on the virtual plane of SPD
	outTree->Branch("SPD_Eendsum",      &SPD_Eendsum,   "SPD_Eendsum/F"    ); //Deposit energy in the SPD
	outTree->Branch("SPD_Eend",      &SPD_Eend,   "SPD_Eend/F"    ); // prime particle deposit energy in the SPD
	outTree->Branch("LASPD_P",      &LASPD_P,   "LASPD_P/F"    ); //momentum hit on the virtual plane of LASPD
	outTree->Branch("LASPD_Eendsum",      &LASPD_Eendsum,   "LASPD_Eendsum/F"    ); //Deposit energy in the LASPD
	outTree->Branch("LASPD_Eend",      &LASPD_Eend,   "LASPD_Eend/F"    ); //prime partile deposit energy in the LASPD
	outTree->Branch("GEM00_n",      &GEM00_n,   "GEM00_n/I"    ); //total hits on the virtual plane of GEM00 
	outTree->Branch("GEM00_np",      &GEM00_np,   "GEM00_np/I"    ); //prime partile index hit on the virtual plane of GEM00 
	outTree->Branch("GEM00_x", &GEM00_x[0],"GEM00_x[GEM00_n]/F");// local x of GEM00
	outTree->Branch("GEM00_y", &GEM00_y[0],"GEM00_y[GEM00_n]/F");// local y of GEM00
	outTree->Branch("GEM00_vy", &GEM00_vy[0],"GEM00_vy[GEM00_n]/F");// vertex y of GEM00
	outTree->Branch("GEM00_vx", &GEM00_vx[0],"GEM00_vx[GEM00_n]/F");// vertex x of GEM00

	outTree->Branch("GEM10_n",      &GEM10_n,   "GEM10_n/I"    ); //total hits on the virtual plane of GEM00 
	outTree->Branch("GEM10_np",      &GEM10_np,   "GEM10_np/I"    ); //prime partile index hit on the virtual plane of GEM00 
	outTree->Branch("GEM10_x", &GEM10_x[0],"GEM10_x[GEM10_n]/F");// local x of GEM10
	outTree->Branch("GEM10_y", &GEM10_y[0],"GEM10_y[GEM10_n]/F");// local y of GEM10
	outTree->Branch("GEM10_vy", &GEM10_vy[0],"GEM10_vy[GEM10_n]/F");// vertex y of GEM10
	outTree->Branch("GEM10_vx", &GEM10_vx[0],"GEM10_vx[GEM10_n]/F");// vertex x of GEM10
	//TFile *file=new TFile(infile, "READ");
	TFile *file=new TFile(inputfile_name.c_str());
	TTree *tree_header = 0;
	vector <double> *var1=0,*var2=0,*var3=0,*var4=0,*var5=0,*var6=0,*var7=0,*var8=0, *var9=0,*var10;

	tree_header = (TTree*) file->Get("userHeader");
	tree_header->SetBranchAddress("userVar001",&var1);     //1
	tree_header->SetBranchAddress("userVar002",&var2);     //x
	tree_header->SetBranchAddress("userVar003",&var3);     //y
	tree_header->SetBranchAddress("userVar004",&var4);     //W
	tree_header->SetBranchAddress("userVar005",&var5);     //Q2
	tree_header->SetBranchAddress("userVar006",&var6);     //rate
	tree_header->SetBranchAddress("userVar007",&var7);     //radrate
	tree_header->SetBranchAddress("userVar008",&var8);     //Ei, incoming beam energy after energy loss????
	tree_header->SetBranchAddress("userVar009",&var9);     //Abeam
	tree_header->SetBranchAddress("userVar010",&var10);     //Abeam

	TTree *tree_generated = (TTree*) file->Get("generated");
	vector <int> *gen_pid=0;
	vector <double> *gen_px=0,*gen_py=0,*gen_pz=0,*gen_vx=0,*gen_vy=0,*gen_vz=0;
	tree_generated->SetBranchAddress("pid",&gen_pid);   //particle ID
	tree_generated->SetBranchAddress("px",&gen_px);     //momentum of the generated particle at target
	tree_generated->SetBranchAddress("py",&gen_py);
	tree_generated->SetBranchAddress("pz",&gen_pz);
	tree_generated->SetBranchAddress("vx",&gen_vx);     //vertex of the generated particle at target
	tree_generated->SetBranchAddress("vy",&gen_vy);
	tree_generated->SetBranchAddress("vz",&gen_vz);

	TTree *tree_flux = (TTree*) file->Get("flux");
	vector<double> *flux_id=0,*flux_hitn=0;
	vector<double> *flux_pid=0,*flux_mpid=0,*flux_tid=0,*flux_mtid=0,*flux_otid=0;
	vector<double> *flux_trackE=0,*flux_totEdep=0,*flux_avg_x=0,*flux_avg_y=0,*flux_avg_z=0,*flux_avg_lx=0,*flux_avg_ly=0, *flux_avg_lz=0,*flux_px=0,*flux_py=0,*flux_pz=0,*flux_vx=0,*flux_vy=0,*flux_vz=0,*flux_mvx=0,*flux_mvy=0,*flux_mvz=0,*flux_avg_t=0;

	tree_flux->SetBranchAddress("hitn",&flux_hitn);     // hit number
	tree_flux->SetBranchAddress("id",&flux_id);         //hitting detector ID

	tree_flux->SetBranchAddress("pid",&flux_pid);       //pid
	tree_flux->SetBranchAddress("mpid",&flux_mpid);     // mother pid
	tree_flux->SetBranchAddress("tid",&flux_tid);       // track id
	tree_flux->SetBranchAddress("mtid",&flux_mtid);     // mother track id
	tree_flux->SetBranchAddress("otid",&flux_otid);     // original track id
	tree_flux->SetBranchAddress("trackE",&flux_trackE);   //track energy of 1st step,  track here is G4 track
	tree_flux->SetBranchAddress("totEdep",&flux_totEdep); //totEdep in all steps, track here is G4 track
	tree_flux->SetBranchAddress("avg_x",&flux_avg_x);     //average x, weighted by energy deposition in each step
	tree_flux->SetBranchAddress("avg_y",&flux_avg_y);     //average y
	tree_flux->SetBranchAddress("avg_z",&flux_avg_z);     //average z
	tree_flux->SetBranchAddress("avg_lx",&flux_avg_lx);   // local average x
	tree_flux->SetBranchAddress("avg_ly",&flux_avg_ly);   // local average y
	tree_flux->SetBranchAddress("avg_lz",&flux_avg_lz);   // local average z
	tree_flux->SetBranchAddress("px",&flux_px);          // px of 1st step
	tree_flux->SetBranchAddress("py",&flux_py);          // px of 1st step
	tree_flux->SetBranchAddress("pz",&flux_pz);          // px of 1st step
	tree_flux->SetBranchAddress("vx",&flux_vx);          // x coordinate of 1st step
	tree_flux->SetBranchAddress("vy",&flux_vy);          // y coordinate of 1st step
	tree_flux->SetBranchAddress("vz",&flux_vz);          // z coordinate of 1st step
	//information recorded by ec
	TTree* tree_solid_ec= (TTree*) file->Get("solid_ec");
	TTree* tree_solid_ec_ps= (TTree*) file->Get("solid_ec_ps");
	setup_tree_solid_ec(tree_solid_ec);	
	setup_tree_solid_ec_ps(tree_solid_ec_ps);	
	//information recorded by spd
	TTree* tree_solid_spd= (TTree*) file->Get("solid_spd");
	setup_tree_solid_spd(tree_solid_spd);	 
	//information recorded by spd
	//TTree* tree_solid_gem = (TTree*) file->Get("solid_gem");
	//setup_tree_solid_gem(tree_solid_gem);	
	TRandom3 rand;
	rand.SetSeed(0);

	int sensor_good=0;
	int event_good=0,event_trig_good=0;
	// 	long int N_events = (long int)tree_header->GetEntries();
	long int N_events = (long int)tree_generated->GetEntries();	

	cout << "total number of events : " << N_events << endl;	

	//----------------------------
	//      loop trees
	//---------------------------
	double Ek=0;
	double Ec=0;
	double trigger_ec=0;
	int hit_id=-1;
	double px_max=0,py_max=0,pz_max=0,p_max=0;
	double SC0_p_max=0;
	double SC1_p_max=0;
	double SPD_p_max=0;
	double LASPD_p_max=0;
	int GEM00_index_max=0;
	int GEM10_index_max=0;
	int pid_gen1=0;
	double hit_pf=0;
	//	double theta_gen1=0,p_gen1=0,px_gen1=0,py_gen1=0,pz_gen1=0,vx_gen1=0,vy_gen1=0,vz_gen1=0;      
	bool writeFlag = false;
	for(long int i=0;i<N_events;i++){
		
		// cout<<"pid_size="<<gen_pid->size()<<endl;
		if (evgen==2){
			tree_header->GetEntry(i);
			rate=var6->at(0)/numberOfFile; // new eDIS and eAll generator
		}
		else if (evgen==0) {
			rate=1e-6/1.6e-19/event_actual;  //beamOnTarget  file
		}
		else if (evgen==4) rate=1; //even event file
		else if (evgen==3){         // bggen file
			tree_header->GetEntry(i);
			rate=var10->at(0)/numberOfFile; 
		}
		else {
			cout << "Not right filemode" << endl;    
			return 0; 
		}
		tree_generated->GetEntry(i);
		for (int j=0;j<gen_pid->size();j++) {
			pid_gen= gen_pid->at(j);
			px_gen=gen_px->at(j)/1e3;
			py_gen=gen_py->at(j)/1e3;
			pz_gen=gen_pz->at(j)/1e3;
			vx_gen=gen_vx->at(j)*0.1;
			vy_gen=gen_vy->at(j)*0.1;
			vz_gen=gen_vz->at(j)*0.1;
			p_gen=sqrt(px_gen*px_gen+py_gen*py_gen+pz_gen*pz_gen); 
		}
		//---
		tree_flux->GetEntry(i);		  
		int sec_hgc=0;		
		bool Is_trig=false;					  
		hit_id=-1;
		double Eec=0,Eec_photonele=0,Eec_ele=0,Edepsc1=0,Edepsc2=0;
		px_max = 0;
		py_max = 0;
		pz_max = 0;
		p_max = 0;
		GEM00_n=0;
		GEM10_n=0;
		SC0_p_max=0;
		SC1_p_max=0;
		SPD_p_max=0;
		LASPD_p_max=0;
		GEM00_index_max=0;
		GEM10_index_max=0;
		for (Int_t j=0;j<flux_hitn->size();j++) {
			//check tid tid==1 prime particle
			//	if(flux_tid->at(j) !=1) continue;
			hit_pf=sqrt(flux_px->at(j)*flux_px->at(j)+flux_py->at(j)*flux_py->at(j)+flux_pz->at(j)*flux_pz->at(j))/1e3;  //MeV to GeV
			double E=flux_trackE->at(j)/1e3;		  
			if(flux_id->at(j)==1) hit_id=0; // SC0 front
			else if(flux_id->at(j)==2) hit_id=1; // SC1 front
			else if(flux_id->at(j)==3) hit_id=2; // EC front
			else if(flux_id->at(j)==9) hit_id=9; // EC shower front
			else if(flux_id->at(j)==4) hit_id=3; // EC back
			else if(flux_id->at(j)==5) hit_id=4; // GEM 1
			else if(flux_id->at(j)==6) hit_id=5; // GEM 2	  
			else if(flux_id->at(j)==7) hit_id=7; // spd 1
			else if(flux_id->at(j)==8) hit_id=8; // spd 2	  
			else if(flux_id->at(j)==10) hit_id=6; // Cherenkov front window
			else cout << "wrong flux_id" << flux_id->at(j) << endl;
			if(flux_tid->at(j)==1 ){
				if(hit_id==2){
					p_max=hit_pf;
					px_max=flux_px->at(j)/1e3;
					py_max=flux_py->at(j)/1e3;
					pz_max=flux_pz->at(j)/1e3;
				}
				if(hit_id==0 ){
					SC0_p_max=hit_pf;
				}
				if(hit_id==1){
					SC1_p_max=hit_pf;
				}
				if(hit_id==7){
					SPD_p_max=hit_pf;
				}
				if(hit_id==8){
					LASPD_p_max=hit_pf;
				}
				if(hit_id==4){
					GEM00_np=j;
				}
				if(hit_id==5){
					GEM10_np=j;
				}
			}// end tid==1
			if(hit_id==4){
				GEM00_x[GEM00_n]=flux_avg_lx->at(j)*0.1;
				GEM00_y[GEM00_n]=flux_avg_ly->at(j)*0.1;
				GEM00_vx[GEM00_n]=flux_avg_x->at(j)*0.1;
				GEM00_vy[GEM00_n]=flux_avg_y->at(j)*0.1;
				GEM00_n++;
			}
			if(hit_id==5){
				GEM10_x[GEM10_n]=flux_avg_lx->at(j)*0.1;
				GEM10_y[GEM10_n]=flux_avg_ly->at(j)*0.1;
				GEM10_vx[GEM10_n]=flux_avg_x->at(j)*0.1;
				GEM10_vy[GEM10_n]=flux_avg_y->at(j)*0.1;
				GEM10_n++;
			}
			//cout<<"event="<<i<<"j="<<j<<" GEM00_n="<<GEM00_n<<"GEM10_n="<<GEM10_n<<endl;
		}	// end of flux		

		// process ec
		tree_solid_ec->GetEntry(i);
		tree_solid_ec_ps->GetEntry(i);
		//if(hit_preshowerN>0){
		double Eend_ec_sum=0;
		double Eend_ec_ps_sum=0;	
		double Eend_ec[4]={0};
		double Eend_ec_ps[4]={0};
		process_tree_solid_ec(tree_solid_ec,tree_solid_ec_ps,Eend_ec_sum,Eend_ec_ps_sum,Eend_ec,Eend_ec_ps);

		//if(Eend_ec_ps_sum>0){ 
		PreShSum=Eend_ec_ps_sum;
		PreSh_l=Eend_ec_ps[2];
		PreSh_r=Eend_ec_ps[3];
		PreSh_t=Eend_ec_ps[1];
		ShowerSum=Eend_ec_sum;
		Shower_l=Eend_ec[2];
		Shower_r=Eend_ec[3];
		Shower_t=Eend_ec[1];
		PreShP=p_max;
		PreShPx=px_max;
		PreShPy=py_max;
		PreShPz=pz_max;
	//	writeFlag = true;
		//cout<<"event="<<i<<"  pid_gen="<<pid_gen<<endl;
		//if(Eend_ec_ps_sum>0){
		//cout<<"event="<<i<<"   "<<"Eend_ec_ps_sum="<<Eend_ec_ps_sum<<"PreSh_l="<<PreSh_l<<"PreSh_r="<<PreSh_r<<"PreSh_t="<<PreSh_t<<endl;
		//cout<<"event="<<i<<"   "<<"Eend_ec_sum="<<Eend_ec_sum<<"Shower_l="<<Shower_l<<"Shower_r="<<Shower_r<<"Shower_t="<<Shower_t<<endl;
		//}
		// process spd
		tree_solid_spd->GetEntry(i);
		double Edep_sc1=0, Edep_sc2=0, Edep_spd1=0, Edep_spd2=0;
		process_tree_solid_spd_simple(tree_solid_spd,Edep_sc1,Edep_sc2,Edep_spd1,Edep_spd2);
		//SC0
		SC0_P=SC0_p_max;
		SC0_Eendsum = Edep_sc1;
		if(SC0_P>0){
			SC0_Eend = Edep_sc1;
			//cout<<"event="<<i<<"SC0_P="<<SC0_P<<"SC0_Eendsum="<<SC0_Eendsum<<endl;
		}
		//SC0
		SC1_P=SC1_p_max;
		SC1_Eendsum = Edep_sc2;
		if(SC1_P>0){
			SC1_Eend = Edep_sc2;
		}
		//SPD
		SPD_P=SPD_p_max;
		SPD_Eendsum = Edep_spd1;
		if(SPD_P>0){
			SPD_Eend = Edep_spd1;
		}
		//LASPD
		LASPD_P=LASPD_p_max;
		LASPD_Eendsum = Edep_spd2;
		if(LASPD_P>0){
			LASPD_Eend = Edep_spd2;
		}
		//GEM00
	//	if (writeFlag){
			outTree->Fill();
	//		outfile1<<pid_gen<<","<<vx_gen<<","<<vy_gen<<","<<vz_gen<<","<<px_gen<<","<<py_gen<<","<<pz_gen<<","<<p_gen<<","<<SC0_P<<","<<SC0_Eendsum<<","<<SC1_P<<","<<SC1_Eendsum<<","<<SPD_P<<","<<SPD_Eendsum<<","<<LASPD_P<<","<<LASPD_Eendsum<<","<<PreShPx<<","<<PreShPy<<","<<PreShPz<<","<<PreShP<<","<<PreSh_l<<","<<PreSh_r<<","<<PreSh_t<<","<<PreShSum<<","<<Shower_l<<","<<Shower_r<<","<<Shower_t<<","<<ShowerSum<<endl;               }		
		if(((i+1)%10000)==0) {
			cout<<i+1<<" / "<<N_events<<" events processed \r"<<std::flush;
		}

	} //end loop
	//do outputs
	cout<<endl;
	outFile->cd();
	outTree->Write();
	outFile->Close();
}
