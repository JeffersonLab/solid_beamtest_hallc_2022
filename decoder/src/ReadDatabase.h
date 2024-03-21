
//By Jixie: function to read a database file
//The database file looks like this
/*
# Pedestal database for solid hallc beamtest
# use '#' to comment, anything behind it in the same line will be ignored
# This file contains multiple blocks, each block starts with 'BlockStart={' and end with 'BlockEnd=}'
# Multiple lines of Data will be appended into the memory
# Lines without'=' will be ignored, characters after ';' will also be ignored

BlockStart={
DetName = FADCPedestal
RunMin = 1000
RunMax = 1000
NChannel = 32
Data = 106.0  101.8   99.6  100.4   99.8   98.4  101.4   94.3  107.2  100.1  106.9  108.0  103.3  100.9  101.3   98.8
Data = 107.7  107.9  104.8  117.0  103.1  104.0  106.7  106.6  106.3  101.9  100.0  102.5  103.2  107.1  108.3  101.6
BlockEnd=}
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#ifndef _ReadDataBase_h_
#define _ReadDataBase_h_


// data structures
namespace db
{
//verbose level
static int verbose = 0;

class dbBlock {
public:
    string DetName;
    int RunMin, RunMax, NChannel;
    vector<double> Data;

    dbBlock(){;};

    dbBlock(string name, int nch, double *data, int min, int max)
        : DetName(name), NChannel(nch), RunMin(min), RunMax(max)
    {
        for(int i=0;i<nch;i++) Data.push_back(data[i]);
    }

    dbBlock(string name, vector<double> data, int min, int max)
        : DetName(name), RunMin(min), RunMax(max)
    {
        NChannel = int(data.size());
        for(int i=0;i<NChannel;i++) Data.push_back(data[i]);
    }

    ~dbBlock() {Data.clear();}

    dbBlock& Clone(dbBlock &b)
    {
        DetName = b.DetName;
        RunMin = b.RunMin;
        RunMax = b.RunMax;
        NChannel = b.NChannel;
        Data.clear();
        for(int i=0;i<NChannel;i++) Data.push_back(b.Data[i]);
        return *this;
    }

    void SetData(vector<double> &data)
    {
        NChannel = int(data.size());
        Data.clear();
        for(int i=0;i<NChannel;i++) Data.push_back(data[i]);
    }

    void SetData(int nch, double *data)
    {
        NChannel = nch;
        Data.clear();
        for(int i=0;i<NChannel;i++) Data.push_back(data[i]);
    }

    void Clear() {RunMin=RunMax=NChannel=0;Data.clear();}

    dbBlock& operator=(const dbBlock& b)
    {
        if (this == &b) return *this;
        DetName = b.DetName;
        RunMin = b.RunMin;
        RunMax = b.RunMax;
        NChannel = b.NChannel;
        Data.clear();
        for(int i=0;i<NChannel;i++) Data.push_back(b.Data[i]);
        return *this;
    }

    void Print()
    {
        cout<<"\nBlockStart={\n";
        cout<<"  DetName = "<<DetName<<"\n  RunMin = "<<RunMin<<"\n  RunMax = "<<RunMax<<"\n  Nchannel = "<<NChannel<<endl;
        std::cout.setf(std::ios::fixed, std::ios::floatfield); // floatfield set to fixed
        streamsize oldprecision = std::cout.precision(1);
        for(int i=0;i<NChannel;i++) {
            if ( (i+1)%16 == 1 ) cout<<"  Data = ";
            cout<<setw(6)<<Data[i]<<" ";
            if ( (i+1)%16 == 0 ) cout<<"\n";
        }
        std::cout.precision(oldprecision);
        std::cout.unsetf(std::ios::floatfield);  // floatfield not set
        cout<<"BlockEnd=}\n";
    }
};


static map<string,vector<dbBlock>> DetMap;

void PrintMap()
{
    cout<<"\n==================Start of Detector Map====================\n";
    cout<<"db::PrintMap(): this is the whole detector map:\n";
    map<string,vector<dbBlock>>::iterator it;
    for (it = DetMap.begin(); it != DetMap.end(); it++)
    {
        for (auto & dbB : it->second) {
            dbB.Print();
        }
    }
    cout<<"======================End of Detector Map====================\n";
}

void FillBlock(dbBlock &b)
{
    string name = b.DetName;
    dbBlock dbB = b;
    if(verbose>=5) dbB.Print();

    if(DetMap.find(name) != DetMap.end()) {
        //find the detector, check if the run range is already inside
        //if yes, update the data, otherwise add this block
        bool found = false;
        vector<dbBlock> *ptr = &DetMap[name];
        vector<dbBlock>::iterator it;
        if(verbose>=2) cout<<"Det="<<name<<":\n";
        for(it=ptr->begin();it!=ptr->end();it++) {
            if(it->RunMin == b.RunMin && it->RunMax == b.RunMax) {
                if(verbose>=2) cout<<"\tFound an existing block has the same range, update the content \n";
                *it = b;
                found = true;
                break;
            }
            else if(it->RunMin <= b.RunMin && it->RunMax >= b.RunMax) {
                cout<<"\tThe new block is inside an existing block, split the existing one into 2-3 ranges\n";
                if(it->RunMin == b.RunMin) {
                    it->RunMin = b.RunMax + 1;
                    ptr->insert(it,dbB);
                    if(verbose>=2) cout<<"\tRunMin is the same, change the RunMin of the existing block to "<<it->RunMin<<" then insert the new block in front of it \n";
                } else if(it->RunMax == b.RunMax) {
                    it->RunMax = b.RunMin - 1;
                    ptr->insert(it+1,dbB);
                    if(verbose>=2) cout<<"\tRunMax is the same, change the RunMax of the existing block to "<<it->RunMax<<" then insert the new block behind it \n";
                } else {
                    if(verbose>=2) cout<<"\tSplit the existing block into 2 parts, insert the new block in between them \n";
                    dbBlock dbB2(*it);
                    it->RunMin = b.RunMax + 1;
                    if(verbose>=3) cout<<"\tChanged the 1st splited block RunMin to "<< it->RunMin<<endl;
                    dbB2.RunMax = b.RunMin - 1;
                    if(verbose>=3) cout<<"\tChanged the 2nd splited block RunMax to "<< dbB2.RunMax <<endl;

                    //There are 2 dbBlocks need to be inserted, the iterator 'it' will be invalid after calling insert()
                    //ptr->insert(it,dbB);ptr->insert(it,dbB2); will not work
                    //therefore a vector is created here, the whole vector will be inserted at one call
                    vector<dbBlock> aVec;
                    aVec.push_back(dbB2);
                    aVec.push_back(dbB);
                    if(verbose>=3) cout<<"\tInsert the 2nd splited block and the new block into a NEW vector"<<endl;
                    ptr->insert(it,aVec.begin(),aVec.end());
                    if(verbose>=3) cout<<"\tInsert the newly built vector into the main vector"<<endl;
                }
                found = true;
                break;
            }
        }
        //this reange is not found, insert this block
        if(!found) {
            DetMap[name].push_back(dbB);
        }
    } else {
        //this detector is not found, add it into the map
        vector<dbBlock> pVec;
        pVec.push_back(dbB);
        DetMap[name] = pVec;
    }
}

dbBlock* FindBlock(string name, int run)
{
    if(DetMap.find(name) != DetMap.end()) {
        //find the detector, check if the run range is already inside
        //if yes, take the data, otherwise get the last entry
        /*
        for(auto it=DetMap[name].rbegin();it!=DetMap[name].rend();it++) {  //in decreasing order
            if(it->RunMin <= run && it->RunMax >= run) {
                return static_cast<dbBlock*>(&(*it));
            }
        }
        */

        for(auto & it : DetMap[name]) {  //in increasing order
            if(it.RunMin <= run && it.RunMax >= run) {
                return &it;
            }
        }
        cout<<"Warning: count not find data of detector '"<<name<<"' for run "<<run<<", use the last entry instead...\n";
        return &(DetMap[name].back());
    } else {
        //not found
        cout<<"No detector '"<<name<<"' found in the database...\n";
        return NULL;
    }
}


//remove space or tab \t at both end of the string
void  Trim(std::string &astr)
{
    if(astr.length()<1) return;
    size_t i=astr.length()-1;
    while(astr[i]==' ' || astr[i]=='\t' || astr[i]=='\r' )
    {
        astr.erase(astr.length()-1,1);
        i=astr.length()-1;
    }
    i=0;
    while(astr[i]==' ' || astr[i]=='\t' || astr[i]=='\r' )
    {
        astr.erase(0,1);
    }
}

//Input: filename c string
//Output: return false if file can not open succesfully, otherwise return true
//        all parameters written in the given file will be stored and
//        achieved by calling bool GetParameter<typename>(char *name, T &value);
//        Please NOTE that the typename could be size_t,int,float,double,bool,char*,string only
//feature:
//1)The parameter files must in a format as "parameter=value", allow space ' ' beside '='.
//2)The value is ended by comma ',', semi-comma ';' or space ' ', care should be taken for string parameters
//2)Use '#' to comment out lines or part of lines. Everything after # will be ignored.
//3)A line without '=' will not be understood. All non-understood lines will be printed out and ignored.
//4)The maximum length of each line is 1024.
//5)Lines without'=' will be ignored

bool ReadFile(const char *filename)
{
    ifstream dbfile(filename,ios_base::in);
    if(!dbfile.good())
    {
        cout<<"\n***Error: Can not open database file \""<<filename<<"\"!"<<endl;
        return false;
    }
    else  cout<<"\nReading database file \""<<filename<<"\""<<endl;

    int nline=0;
    size_t found=0;
    static const int MaxLineSize=1024;
    char line[MaxLineSize];
    std::string strLine;
    std::string strName;
    std::string strValue;

    dbBlock dbB;
    while(!dbfile.eof())
    {
        dbfile.getline(line,MaxLineSize);
        nline++;
        //Search forward. for the '#' to skip the afterwards
        strLine=line;
        found=strLine.find('#');
        if(found!=string::npos) strLine=strLine.substr(0,found);

        //trim the string
        Trim(strLine);
        if(strLine.length()==0) continue; //this is a comment line whose first char is '#'

        //try to find '=', if not find, ignore this line
        found=strLine.find('=');
        if (found==string::npos || strLine.length()<3)
        {
            cout<<"Warning! line "<<nline<<" is not recognized: \""<<line<<"\" \n";
            continue;
        }
        strName=strLine.substr(0,found);
        Trim(strName);
        strValue=strLine.substr(found+1);

        found=strValue.find(';');
        if (found!=string::npos) strValue=strValue.substr(0,found);
        Trim(strValue);

        if(verbose>=3) cout<<setw(15)<<strName<<" = "<<strValue<<endl;

        //store the value into the block
        if(strName.compare("DetName")== 0)  dbB.DetName = strValue;
        else if(strName.compare("RunMin")== 0)  dbB.RunMin = atoi(strValue.c_str());
        else if(strName.compare("RunMax")== 0)  dbB.RunMax = atoi(strValue.c_str());
        else if(strName.compare("NChannel")== 0)  dbB.NChannel = atoi(strValue.c_str());
        else if(strName.compare("Data")== 0)  {
            stringstream s(strValue);
            double val=0.0;
            while (s >> val)  dbB.Data.push_back(val);
        }
        else if(strName.compare("BlockStart")== 0)  dbB.Clear();
        else if(strName.compare("BlockEnd")== 0) {
            FillBlock(dbB);
            dbB.Clear(); //reset it for next block
        } else {
            cout<<"Warning: nknown line in database file: \""<<strLine<<"\"\n";
        }

    }
    dbfile.close();

    return true;
}

}; // namespace db


void ReadDatabase(const char *filename="../database/FADCPedestal.txt")
{
    db::DetMap.clear();
    db::ReadFile(filename);
    //db::PrintMap();
    //db::dbBlock *dbB = db::FindBlock("FADCPedestal",4125);
    //if(dbB) dbB->Print();
}

#endif  //_ReadDataBase_h_
