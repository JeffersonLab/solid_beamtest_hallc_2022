#include <algorithm>
#include <utility>

#include <TFile.h>
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

#include <unordered_map>
#include <iostream>
using namespace std; 

//#define SBS_UV_CHAMBER

#define N_LAYERS 2
#define N_CHAMBERS_PER_LAYER 2

#ifndef MAX_CLUSTERS_PER_PLANE
#define MAX_CLUSTERS_PER_PLANE 150  
#endif
/*
fail to fill branch GEM00_y_adc[MAX_CLUSTERS_PER_PLANE]  if MAX_CLUSTERS_PER_PLANE <= 128, will work if it >=150
Fatal in <TBufferFile::AutoExpand>: Request to expand to a negative size, likely due to an integer overflow: 0xc0c13dc0 for a max of 0x7ffffffe.
*/
#define VerboseLevel 3

////////////////////////////////////////////////////////////////////////////////
// uv coordinate to xy coordinate transform
//
// uv_angle is the strip angle between u and v in the upper plane, 60 degree in sbs uv chambers
//
// viewing from the top of the chamber, with the chamber short side (has 8 APVs) on you right hand
//     u channel connects the upper half APV connectors
//     v channel connects the lower half APV connectors

template<typename T1, typename T2>
std::pair<T1, T1> convert_uv_to_xy(const T2& _u, const T2& _v, T2 uv_angle)
{
    T1 u = static_cast<T1>(_u);
    T1 v = static_cast<T1>(_v);

    T1 angle = static_cast<T1>(uv_angle) * TMath::Pi() / 180.;

    T1 x, y;

    // 1558, 406
    // 1766.4, 406

    u += 20;
    v -= 406 * TMath::Sin(angle/2.);

    y = -0.5 * ( (u-v)/TMath::Tan(angle/2.) - 406);
    x = 0.5*(u+v);

    return std::make_pair<T1&, T1&>(x, y);
}


////////////////////////////////////////////////////////////////////////////////
// gem cluster struct

struct GEMCluster
{
    int layerID, detID, layerPosIndex, plane, size;
    float adc, pos;

    GEMCluster():
        layerID(-1), detID(-1), layerPosIndex(-1), plane(-1), size(-1),
        adc(0), pos(-9999)
    {
    }

    GEMCluster(const GEMCluster &that):
        layerID(that.layerID), detID(that.detID), layerPosIndex(that.layerPosIndex),
        plane(that.plane), size(that.size), adc(that.adc), pos(that.pos)
    {
    }

    GEMCluster(GEMCluster &&that):
        layerID(that.layerID), detID(that.detID), layerPosIndex(that.layerPosIndex),
        plane(that.plane), size(that.size), adc(that.adc), pos(that.pos)
    {
    }

    // copy assignment
    GEMCluster& operator=(const GEMCluster &rhs)
    {
        if(this == &rhs)
            return *this;
        GEMCluster c(rhs);
        *this = move(c);
        return *this;
    }

    // move assignment
    GEMCluster & operator=(GEMCluster &&rhs) {
        if(this == &rhs)
            return *this;
        layerID = rhs.layerID;
        detID = rhs.detID;
        layerPosIndex = rhs.layerPosIndex;
        plane = rhs.plane;
        size = rhs.size;
        adc = rhs.adc;
        pos = rhs.pos;
        return *this;
    }
};

// print cluster
ostream& operator<<(ostream& out, const GEMCluster& c) {
    out<<" layer id: "<<c.layerID
        <<" det id: "<<c.detID
        <<" layer pos index: "<<c.layerPosIndex
        <<" plane id: "<<c.plane
        <<" size: "<<c.size
        <<" adc: "<<c.adc
        <<" pos: "<<c.pos
        <<endl;
    return out;
}

// reconstructed gem 2d hit point
struct GEM2DHit 
{
    GEMCluster xc, yc;
    GEM2DHit()
    {}

    GEM2DHit(const GEMCluster &a, const GEMCluster &b)
        :xc(a), yc(b)
    {}
};


// clusters on one plane
struct PlaneCluster
{
    int planeID;
    vector<GEMCluster> clusters;

    PlaneCluster():
        planeID(-1)
    {
        clusters.clear();
    }

    void addCluster(const GEMCluster& c)
    {
        clusters.push_back(c);
    }

    // must be in desending order
    void Sort() {
        sort(clusters.begin(), clusters.end(), 
                [](const GEMCluster &c1, const GEMCluster &c2)
                {
                return c1.adc > c2.adc; // desending order
                });
    }

    void Clear(){
        clusters.clear();
    }
};

// clusters on one chamber
struct ChamberCluster
{
    int chamberPosIndex;
    PlaneCluster plane_cluster[2]; // 2 planes: x/y, u/v

    vector<GEM2DHit> Hits2D;

    ChamberCluster():
        chamberPosIndex(-1)
    {
    }

    void addCluster(const GEMCluster &c)
    {
        int id = c.plane;
        if(id < 0 || id > 1) {
            cout<<"plane index is incorrect: "<<id<<endl;
            return;
        }
        plane_cluster[id].addCluster(c);
        plane_cluster[id].planeID=id;
    }

    void Sort() {
        plane_cluster[0].Sort();
        plane_cluster[1].Sort();
    }

    void Match() 
    {
        Sort();

        // get 2d hits
        size_t nCluster = plane_cluster[0].clusters.size();
        if(nCluster > plane_cluster[1].clusters.size())
            nCluster = plane_cluster[1].clusters.size();

        for(size_t i=0;i<nCluster;i++) {
            GEM2DHit hit(plane_cluster[0].clusters[i],
                    plane_cluster[1].clusters[i]);
            Hits2D.push_back(hit);
        }

        // clear this event, get ready for next event
        for(auto &i: plane_cluster)
            i.Clear();
    }
};

// clusters on one layer
struct LayerCluster
{
    int layerID;
    ChamberCluster chamber_cluster[N_CHAMBERS_PER_LAYER];

    LayerCluster():
        layerID(-1)
    {
    }

    void addCluster(const GEMCluster &c) 
    {
        int id = c.layerPosIndex;
        if(id < 0 || id > 3) {
            cout<<"incorrect chamber pos index: "<< id<<" on layer: "<<layerID<<endl;
            return;
        }
        chamber_cluster[id].addCluster(c);
        chamber_cluster[id].chamberPosIndex=id;
        layerID=c.layerID;
    }

    void Match() {
        for(auto &i: chamber_cluster)
            i.Match();
    }
};

// show 2d cluster map for 1 layer
TH2F* GetLayerCluster2DMap(const LayerCluster &layer_cluster, const int &nlayer)
{
    TH2F *hClusterMap = new TH2F(Form("hClusterMap%d", nlayer), 
            Form("cluster 2d map layer %d", nlayer), 
            120, -60, 60,
            120, -60, 60);

    for(auto &chamber : layer_cluster.chamber_cluster) {
        for(auto &c: chamber.Hits2D) {
#ifdef SBS_UV_CHAMBER
            // convert uv to xy
            std::pair<float, float> res = convert_uv_to_xy<float, float>(c.xc.pos, c.yc.pos, 60);
            hClusterMap -> Fill(res.first, res.second);
#else
            hClusterMap -> Fill(c.xc.pos, c.yc.pos);
#endif
        }
    }

    hClusterMap -> GetXaxis() -> SetTitle("X [mm]");
    hClusterMap -> GetXaxis() -> CenterTitle();
    hClusterMap -> GetYaxis() -> SetTitle("Y [mm]");
    hClusterMap -> GetYaxis() -> CenterTitle();
    hClusterMap -> GetYaxis() -> SetTitleOffset(0.8);

    return hClusterMap;
}

// show results for each layer after matching
// chamber by chamber, one layer has N_CHAMBERS_PER_LAYER chambers
// contents include (for each chamber):
//      cluster size(x and y), adc(x and y), cluster_pos(x and y)
//      charge correlation (x vs y), size correlation (x vs y), cluster 2d map (x vs y)
void ShowLayerResults(const LayerCluster &layer_cluster, const int &nlayer)
{
    TCanvas *c[9];

    int chamber_id = 0;
    for(auto &chamber: layer_cluster.chamber_cluster) 
    {
        // get y position
        float x_size = 102.4; // mm
        float x_min = -x_size/2.;
        float x_max =  x_size/2.;
        float y_size = 102.4; // mm
        float y_min = -y_size/2.;
        float y_max = y_size/2.;
        // hists
        TH1F *cluster_size_x = new TH1F(Form("x_cluster_size_chamber%d/%d", chamber_id, nlayer),
                Form("x cluser size chamber %d", chamber_id),
                20, -0.5, 19.5);
        cluster_size_x -> GetXaxis() -> SetTitle("cluster size");
        cluster_size_x -> GetXaxis() -> CenterTitle();
        TH1F *cluster_size_y = new TH1F(Form("y_cluster_size_chamber%d/%d", chamber_id, nlayer),
                Form("y cluser size chamber %d", chamber_id),
                20, -0.5, 19.5);
        cluster_size_y -> GetXaxis() -> SetTitle("cluster size");
        cluster_size_y -> GetXaxis() -> CenterTitle();
 
        TH1F *cluster_adc_x = new TH1F(Form("x_cluster_adc_chamber%d/%d", chamber_id, nlayer),
                Form("x cluser adc chamber %d", chamber_id),
                100, 0, 2500);
        cluster_adc_x -> GetXaxis() -> SetTitle("U channel ADC");
        cluster_adc_x -> GetXaxis() -> CenterTitle();
 
        TH1F *cluster_adc_y = new TH1F(Form("y_cluster_adc_chamber%d/%d", chamber_id, nlayer),
                Form("y cluser adc chamber %d", chamber_id),
                100, 0, 2500);
        cluster_adc_y -> GetXaxis() -> SetTitle("V channel ADC");
        cluster_adc_y -> GetXaxis() -> CenterTitle();
 
        TH1F *cluster_pos_x = new TH1F(Form("x_cluster_pos_chamber%d/%d", chamber_id, nlayer),
                Form("x cluser pos chamber %d", chamber_id),
                100, x_min, x_max);
        cluster_pos_x -> GetXaxis() -> SetTitle("strip index");
        cluster_pos_x -> GetXaxis() -> CenterTitle();
 
        TH1F *cluster_pos_y = new TH1F(Form("y_cluster_pos_chamber%d/%d", chamber_id, nlayer),
                Form("y cluser pos chamber %d", chamber_id),
                100, x_min, x_max);
        cluster_pos_y -> GetXaxis() -> SetTitle("strip index");
        cluster_pos_y -> GetXaxis() -> CenterTitle();
 
        TH2F *charge_correlation = new TH2F(Form("charge_correlation_chamber%d/%d", chamber_id, nlayer),
                Form("charge_correlation chamber %d", chamber_id),
                1000, 0, 2500, 1000, 0, 2500);
        charge_correlation -> GetXaxis() -> SetTitle("U channel cluster charge");
        charge_correlation -> GetXaxis() -> CenterTitle();
        charge_correlation -> GetYaxis() -> SetTitle("V channel cluster charge");
        charge_correlation -> GetYaxis() -> CenterTitle();
 
        TH2F *size_correlation = new TH2F(Form("size_correlation_chamber%d/%d", chamber_id, nlayer),
                Form("size_correlation chamber %d", chamber_id),
                20, -0.5, 19.5, 20, -0.5, 19.5);
        TH2F *position_correlation = new TH2F(Form("position_correlation_chamber%d/%d", chamber_id, nlayer),
                Form("pos_correlation chamber %d", chamber_id),
                120, -60, 60, 120, -60, 60);

        for(auto &hit2d: chamber.Hits2D) 
        {
            cluster_size_x -> Fill(hit2d.xc.size);
            cluster_size_y -> Fill(hit2d.yc.size);
            cluster_adc_x ->  Fill(hit2d.xc.adc);
            cluster_adc_y ->  Fill(hit2d.yc.adc);
            cluster_pos_x ->  Fill(hit2d.xc.pos);
            cluster_pos_y ->  Fill(hit2d.yc.pos);
            charge_correlation -> Fill(hit2d.xc.adc, hit2d.yc.adc);
            size_correlation   -> Fill(hit2d.xc.size, hit2d.yc.size);
            position_correlation -> Fill(hit2d.xc.pos, hit2d.yc.pos);
        }
        // show histos
        gStyle -> SetTitleFontSize(0.1);

        c[0] = new TCanvas(Form("c_layer%d_0", nlayer), Form("layer %d", nlayer), 900, 600);
        position_correlation->Draw("col");

        c[1] = new TCanvas(Form("c_layer%d_1", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_size_x -> Draw();

        c[2] = new TCanvas(Form("c_layer%d_2", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_size_y -> Draw();

        c[3] = new TCanvas(Form("c_layer%d_3", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_adc_x -> Draw(); 
        
        c[4] = new TCanvas(Form("c_layer%d_4", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_adc_y -> Draw(); 
       
        c[5] = new TCanvas(Form("c_layer%d_5", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_pos_x -> Draw(); 
      
        c[6] = new TCanvas(Form("c_layer%d_6", nlayer), Form("layer %d", nlayer), 900, 600);
        cluster_pos_y -> Draw(); 
     
        c[7] = new TCanvas(Form("c_layer%d_7", nlayer), Form("layer %d", nlayer), 900, 600);
        charge_correlation -> Draw("col");
    
        c[8] = new TCanvas(Form("c_layer%d_8", nlayer), Form("layer %d", nlayer), 900, 600);
        size_correlation -> Draw("col");

        chamber_id++;
    }

    for(int i=0; i<9; i++) {
        if(i== 0) c[i] -> Print(Form("GEM%d.pdf(", nlayer));
        else if(i==8) c[i] -> Print(Form("GEM%d.pdf)", nlayer));
        else c[i] -> Print(Form("GEM%d.pdf", nlayer));
    }

    //return c;
}

// main 
void GEM_Cluster_Matching(const char* path="beamtest_hallc_3153_0.root")
{
    
    std::unordered_map<int, LayerCluster> layer_cluster_map;

    const int Max = 1000; // max 2000 clusters per event
    
    int nCluster;

    int layerID[Max];
    int detID[Max];
    int layerPosIndex[Max];
    int plane[Max];
    int size[Max];
    float adc[Max];
    float pos[Max];
    TFile *f = new TFile(path);
    TTree *EvTree = (TTree*)f->Get("EvTree");
    
    EvTree->SetBranchAddress("GEM_nCluster", &nCluster);
    EvTree->SetBranchAddress("GEM_planeID", layerID);
    EvTree->SetBranchAddress("GEM_prodID", detID);
    EvTree->SetBranchAddress("GEM_moduleID", layerPosIndex);
    EvTree->SetBranchAddress("GEM_axis", plane);
    EvTree->SetBranchAddress("GEM_size", size);
    EvTree->SetBranchAddress("GEM_adc", adc);
    EvTree->SetBranchAddress("GEM_pos", pos);

    int Entries = EvTree->GetEntries();
    cout<<"total enries: "<<Entries<<endl;

    TFile *OutFile = new TFile("gem.root","RECREATE");
    TTree* T = new TTree("T","GEM cluster hits");

    int GEM00_n,GEM01_n,GEM10_n,GEM11_n;
    float GEM00_x[MAX_CLUSTERS_PER_PLANE],GEM00_y[MAX_CLUSTERS_PER_PLANE];
    float GEM01_x[MAX_CLUSTERS_PER_PLANE],GEM01_y[MAX_CLUSTERS_PER_PLANE];
    float GEM10_x[MAX_CLUSTERS_PER_PLANE],GEM10_y[MAX_CLUSTERS_PER_PLANE];
    float GEM11_x[MAX_CLUSTERS_PER_PLANE],GEM11_y[MAX_CLUSTERS_PER_PLANE];
    
    float GEM00_x_adc[MAX_CLUSTERS_PER_PLANE],GEM00_y_adc[MAX_CLUSTERS_PER_PLANE];
    float GEM01_x_adc[MAX_CLUSTERS_PER_PLANE],GEM01_y_adc[MAX_CLUSTERS_PER_PLANE];
    float GEM10_x_adc[MAX_CLUSTERS_PER_PLANE],GEM10_y_adc[MAX_CLUSTERS_PER_PLANE];
    float GEM11_x_adc[MAX_CLUSTERS_PER_PLANE],GEM11_y_adc[MAX_CLUSTERS_PER_PLANE];
    
    //reset the tree leavies values
    GEM00_n=GEM01_n=GEM10_n=GEM10_n=0;
    for(int i=0;i<MAX_CLUSTERS_PER_PLANE;i++) {
        GEM00_x[i]=GEM00_y[i]=-9999.0;
        GEM01_x[i]=GEM01_y[i]=-9999.0;
        GEM10_x[i]=GEM10_y[i]=-9999.0;
        GEM11_x[i]=GEM11_y[i]=-9999.0;
        
        GEM00_x_adc[i]=GEM00_y_adc[i]=0.0;
        GEM01_x_adc[i]=GEM01_y_adc[i]=0.0;
        GEM10_x_adc[i]=GEM10_y_adc[i]=0.0;
        GEM11_x_adc[i]=GEM11_y_adc[i]=0.0;
    }
    
    T->Branch("GEM_nCluster", &nCluster,"GEM_nCluster/I");
    
    T->Branch("GEM00_n", &GEM00_n,"GEM00_n/I");
    T->Branch("GEM00_x", &GEM00_x[0],"GEM00_x[GEM00_n]/F");
    T->Branch("GEM00_y", &GEM00_y[0],"GEM00_y[GEM00_n]/F");
    T->Branch("GEM00_x_adc", &GEM00_x_adc[0],"GEM00_x_adc[GEM00_n]/F");
    T->Branch("GEM00_y_adc", &GEM00_y_adc[0],"GEM00_y_adc[GEM00_n]/F");

    T->Branch("GEM01_n", &GEM01_n,"GEM01_n/I");
    T->Branch("GEM01_x", &GEM01_x[0],"GEM01_x[GEM01_n]/F");
    T->Branch("GEM01_y", &GEM01_y[0],"GEM01_y[GEM01_n]/F");
    T->Branch("GEM01_x_adc", &GEM01_x_adc[0],"GEM01_x_adc[GEM01_n]/F");
    T->Branch("GEM01_y_adc", &GEM01_y_adc[0],"GEM01_y_adc[GEM01_n]/F");

    T->Branch("GEM10_n", &GEM10_n,"GEM10_n/I");
    T->Branch("GEM10_x", &GEM10_x[0],"GEM10_x[GEM10_n]/F");
    T->Branch("GEM10_y", &GEM10_y[0],"GEM10_y[GEM10_n]/F");
    T->Branch("GEM10_x_adc", &GEM10_x_adc[0],"GEM10_x_adc[GEM10_n]/F");
    T->Branch("GEM10_y_adc", &GEM10_y_adc[0],"GEM10_y_adc[GEM10_n]/F");

    T->Branch("GEM11_n", &GEM11_n,"GEM11_n/I");
    T->Branch("GEM11_x", &GEM11_x[0],"GEM11_x[GEM11_n]/F");
    T->Branch("GEM11_y", &GEM11_y[0],"GEM11_y[GEM11_n]/F");
    T->Branch("GEM11_x_adc", &GEM11_x_adc[0],"GEM11_x_adc[GEM11_n]/F");
    T->Branch("GEM11_y_adc", &GEM11_y_adc[0],"GEM11_y_adc[GEM11_n]/F");

    // decode
    for(int entry = 0;entry<Entries;entry++) 
    {
        if(entry > 10000) break;
        if((entry%1000)==0 && entry) std::cout<<entry<<" events processed \r"<<std::flush;

        EvTree->GetEntry(entry);
        
        if(nCluster<2)  continue;

        if(VerboseLevel>=3) cout<<"debug entry "<<entry<<": nCluster="<<nCluster<<"\n";
        // loop for each cluster
        for(int i=0; i<nCluster; i++) {
            GEMCluster cluster;
            cluster.layerID = layerID[i];
            cluster.detID = detID[i];
            cluster.layerPosIndex = layerPosIndex[i];
            cluster.plane = plane[i];
            cluster.size = size[i];
            cluster.adc = adc[i];
            cluster.pos = pos[i];

            if(VerboseLevel>=3) cout<<i<<":  layerID="<<layerID[i]<<", layerPosIndex="<<layerPosIndex[i]<<", x-y-plan="<<plane[i]<<", adc="<<adc[i]<<"\n";
            layer_cluster_map[layerID[i]].addCluster(cluster);
        }

        // match clusters
        for(auto &i: layer_cluster_map) {
            i.second.Match();
        }

        // extract the result
        for(auto &layer: layer_cluster_map) {
            if(VerboseLevel>=3) cout<<"\t debug layer "<<layer.first<<"\n";
            for(auto &chamber : layer.second.chamber_cluster) {
                if(chamber.Hits2D.size()<=0) continue;
                if(VerboseLevel>=3) cout<<"\t \t debug chamber "<<chamber.chamberPosIndex<<"\n";
                for(auto &hit: chamber.Hits2D) {
                   
                    int gem_id = hit.xc.layerID*10 + hit.xc.layerPosIndex;  
                    if(VerboseLevel>=3) cout<<"\t \t \t debug hit, gem_id = "<<gem_id<<", x_adc="<<hit.xc.adc<<", y_adc="<<hit.yc.adc<<" \n";
                    switch(gem_id) {
                    case 0:
                        {
                            if(GEM00_n>=MAX_CLUSTERS_PER_PLANE) break;
                            GEM00_x[GEM00_n] = hit.xc.pos;
                            GEM00_y[GEM00_n] = hit.yc.pos;
                            GEM00_x_adc[GEM00_n] = hit.xc.adc;
                            GEM00_y_adc[GEM00_n] = hit.yc.adc;
                            GEM00_n++;
                        }   
                        break;
                    case 1:
                        {
                            if(GEM01_n>=MAX_CLUSTERS_PER_PLANE) break;
                            GEM01_x[GEM01_n] = hit.xc.pos;
                            GEM01_y[GEM01_n] = hit.yc.pos;
                            GEM01_x_adc[GEM01_n] = hit.xc.adc;
                            GEM01_y_adc[GEM01_n] = hit.yc.adc;
                            GEM01_n++;
                        }   
                        break;
                    case 10:
                        {
                            if(GEM10_n>=MAX_CLUSTERS_PER_PLANE) break;
                            GEM10_x[GEM10_n] = hit.xc.pos;
                            GEM10_y[GEM10_n] = hit.yc.pos;
                            GEM10_x_adc[GEM10_n] = hit.xc.adc;
                            GEM10_y_adc[GEM10_n] = hit.yc.adc;
                            GEM10_n++;
                        }   
                        break;
                    case 11:
                        {
                            if(GEM11_n>=MAX_CLUSTERS_PER_PLANE) break;
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

        T->Fill();
        
        //reset the tree leavies values
        GEM00_n=GEM01_n=GEM10_n=GEM11_n=0;
        for(int i=0;i<MAX_CLUSTERS_PER_PLANE;i++) {
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
    T->Write(0,TObject::kOverwrite);
    OutFile->Close();
    cout<<"\ndone!\n";
}
