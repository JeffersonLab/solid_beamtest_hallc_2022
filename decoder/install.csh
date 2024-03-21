#!/bin/csh

source setup.csh

if (! $?HallCBeamtestDir) then
  setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
endif

mkdir -pv $HallCBeamtestDir
if (! -d $HallCBeamtestDir) then
  echo '$HallCBeamtestDir  does not exit, I quit ...';
  exit 0;
endif

mkdir -p $HallCBeamtestDir/tracking
cmake -S. -B$HallCBeamtestDir/build
cmake --build $HallCBeamtestDir/build
cmake --install $HallCBeamtestDir/build --prefix $HallCBeamtestDir
#todo: update CMakeList.txt to include the next 2 lines into "make install"
cp -f ./bin/*csh ${HallCBeamtestDir}/bin/
cp -fr ./database ./config ./CreateLevel1Tree rootlogon.C setup*sh ${HallCBeamtestDir}/
cp -fr tracking/gemped tracking/tracking_config  ${HallCBeamtestDir}/tracking/

