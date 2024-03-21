
//By Jixie: function to read a database file

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "ReadDatabase.h"

using namespace std;

extern double gPed7[16];
extern double gPed8[16];

void InitDB()
{
	char* HallCBeamtestDir = getenv("HallCBeamtestDir");
	db::DetMap.clear();
	if(strlen(HallCBeamtestDir)>1) {
		char filename[255];
		sprintf(filename, "%s/database/FADCPedestal.txt", HallCBeamtestDir);
		db::ReadFile(filename);
		sprintf(filename, "%s/database/FADCPeak.txt", HallCBeamtestDir);
		db::ReadFile(filename);
	} else {
		std::string USER = getenv("USER");
		if(USER.find("jixie") != std::string::npos) {
			db::ReadFile("/work/halla/solid/jixie/ecal_beamtest_hallc/decoder/database/FADCPedestal.txt");
			db::ReadFile("/work/halla/solid/jixie/ecal_beamtest_hallc/decoder/database/FADCPeak.txt");
		}
		else {
			db::ReadFile("/home/solid/replay/database/FADCPedestal.txt");
			db::ReadFile("/home/solid/replay/database/FADCPeak.txt");
		}
	}
	db::IsInitialized = 1;
	//db::PrintMap();
	//db::dbBlock *dbB = db::FindBlock("FADCPedestal",4125);
	//if(dbB) dbB->Print();
}

void GetPed(int run, double ped7[], double ped8[], int verbose_level=0)
{
	if(!db::IsInitialized) InitDB();
	db::dbBlock *dbB = db::FindBlock("FADCPedestal",run);
	if(!dbB) return;
	for(int i=0;i<16;i++) {
		ped7[i] = dbB->Data[i];
		ped8[i] = dbB->Data[16+i];
	}
	if(verbose_level) {
		cout<<"gPed7[] = { "<< ped7[0];
		for (int i=1;i<16;i++) cout<<", "<<ped7[i];
		cout<<"}; \n";
		cout<<"gPed8[] = { "<< ped8[0];
		for (int i=1;i<16;i++) cout<<", "<<ped8[i];
		cout<<"}; \n";
	}
}

void GetPeakPos(int run, double peak7[][16], double peak8[][16], int verbose_level=0)
{
	if(!db::IsInitialized) InitDB();
	db::dbBlock *dbB = db::FindBlock("FADCPeakPos",run);
	if(!dbB) return;
	for(int i=0;i<6;i++) {
		for(int j=0;j<16;j++) {
			peak7[i][j] = dbB->Data[16*i+j];
			peak8[i][j] = dbB->Data[96+16*i+j];
		}
	}

	if(verbose_level) {
		cout<<"gSlot7PeakPos_4[] = { "<< peak7[4][0];
		for (int i=1;i<16;i++) cout<<", "<<peak7[4][i];
		cout<<"}; \n";
		cout<<"gSlot8PeakPos_4[] = { "<< peak8[4][0];
		for (int i=1;i<16;i++) cout<<", "<<peak8[4][i];
		cout<<"}; \n";
	}
}

void GetPedNPeakPos(int run, double ped7[], double ped8[], double peak7[][16], double peak8[][16], int verbose_level=1)
{
	GetPed(run,ped7,ped8,verbose_level);
	GetPeakPos(run,peak7,peak8,verbose_level);
}

void GetPed(int run, int verbose_level=1)
{
	GetPed(run,gPed7,gPed8,verbose_level);
}

void GetPedNPeakPos(int run, double peak7[][16], double peak8[][16], int verbose_level=1)
{
	GetPedNPeakPos(run,gPed7,gPed8,peak7,peak8,verbose_level);
}
