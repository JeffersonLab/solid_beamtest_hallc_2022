#!/bin/bash

source ~/apps/setup_rh8.sh

if [-z $HallCBeamtestDir]
then
  export HallCBeamtestDir=/$HOME/apps/cbeamtest_decoder
fi

mkdir -pv $HallCBeamtestDir
if [! -d $HallCBeamtestDir] 
then
  echo '$HallCBeamtestDir  does not exit, I quit ...';
  exit 0;
fi

mkdir -p $HallCBeamtestDir/tracking
cmake -S. -B$HallCBeamtestDir/build
cmake --build $HallCBeamtestDir/build
cmake --install $HallCBeamtestDir/build --prefix $HallCBeamtestDir
#todo: update CMakeList.txt to include the next 2 lines into "make install"
cp -f ./bin/*csh ${HallCBeamtestDir}/bin/
cp -fr ./database ./config ./CreateLevel1Tree rootlogon.C setup*sh ${HallCBeamtestDir}/
cp -fr tracking/gemped tracking/tracking_config  ${HallCBeamtestDir}/tracking/
