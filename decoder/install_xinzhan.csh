#!/bin/csh

source setup_xinzhan.csh

setenv HallCBeamtestDir ${PWD}

cmake -S. -B$HallCBeamtestDir/build
cmake --build $HallCBeamtestDir/build
cmake --install $HallCBeamtestDir/build --prefix $HallCBeamtestDir
