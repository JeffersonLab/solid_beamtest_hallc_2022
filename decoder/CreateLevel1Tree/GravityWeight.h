/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicing energy deposit in transverse direction
 *
 *  Author: Zhenyu Ye (UIC), 02/24/2021
 */

#ifndef _EcalEvent_H_
#define _EcalEvent_H_

#include <algorithm>

using namespace std;

// From the o
double m_logWeightBase=3.6;
//double m_depthCorrection=0.0;

// origin is the corner where all 3 modules contact. unit is cm
float Shower_XCenter[3]={-5.28,0.00,5.28};
float Shower_YCenter[3]={-3.05,6.10,-3.05};
float Shower_ZCenter[3]={3,3,3};
float PreSh_XCenter[3]={0.00,-5.28,5.28};
float PreSh_YCenter[3]={6.10,-3.05,-3.05};
float PreSh_ZCenter[3]={0,0,0};

class EcalEvent
{
public:
    EcalEvent() { }
    virtual ~EcalEvent(){ };

    float fAdc[16];
    float Shower_Cluster_AdcSum;
    float Shower_Cluster_x,Shower_Cluster_y,Shower_Cluster_z;
    float PreSh_Cluster_AdcSum,PreSh_Cluster_x,PreSh_Cluster_y,PreSh_Cluster_z;
    
    void Shower_Clustering() {
        float tw = 0., x = 0., y = 0., z = 0.;

        Shower_Cluster_AdcSum=fAdc[2]+fAdc[3]+fAdc[4];
        for (Int_t i=2;i<=4;i++) {
            // suppress low energy contributions
            float w = std::max(0., double(m_logWeightBase + log(fAdc[i]/Shower_Cluster_AdcSum)));
            tw += w;
            x += Shower_XCenter[i-2] * w;
            y += Shower_YCenter[i-2] * w;
            z += Shower_ZCenter[i-2] * w;
        }
        Shower_Cluster_x=x/tw;
        Shower_Cluster_y=y/tw;
        Shower_Cluster_z=z/tw;
    }

    void PreSh_Clustering() {
        float tw = 0., x = 0., y = 0., z = 0.;

        PreSh_Cluster_AdcSum=fAdc[5]+fAdc[6]+fAdc[7];
        for (Int_t i=5;i<=7;i++) {
            // suppress low energy contributions
            float w = std::max(0., double(m_logWeightBase + log(fAdc[i]/Shower_Cluster_AdcSum)));
            tw += w;
            x += PreSh_XCenter[i-5] * w;
            y += PreSh_YCenter[i-5] * w;
            z += PreSh_ZCenter[i-5] * w;
        }
        PreSh_Cluster_x=x/tw;
        PreSh_Cluster_y=y/tw;
        PreSh_Cluster_z=z/tw;
    }
    
    void DoWeighting(float PreSh_l,float PreSh_r,float PreSh_t,float Shower_l,float Shower_r,float Shower_t) {
        fAdc[2]=Shower_l;
        fAdc[3]=Shower_t;
        fAdc[4]=Shower_r;
        Shower_Clustering();
        
        fAdc[5]=PreSh_t;
        fAdc[6]=PreSh_l;
        fAdc[7]=PreSh_r;
        PreSh_Clustering();
    }
    
    void Reset() {
        
        Shower_Cluster_AdcSum=PreSh_Cluster_AdcSum=0.0;
        Shower_Cluster_x=Shower_Cluster_y=Shower_Cluster_z=-100.0;
        PreSh_Cluster_x=PreSh_Cluster_y=PreSh_Cluster_z=-100.0;
    }

    //ClassDef(EcalEvent,1)
};

//ClassImp(EcalEvent)
#endif //_EcalEvent_H_
