#!/bin/bash

alias ll='ls -lh'

MY_APPS=/$HOME/apps

# set these to make sure cmake is using the correct C/C++ compilers
export CC=`which gcc`
export CXX=`which g++`

#python already install in  /usr
#PythonDir=/apps/python3/3.9.7
#export PATH=${PythonDir}/bin:${PATH}
#export LD_LIBRARY_PATH=${PythonDir}/lib:${LD_LIBRARY_PATH}

# load qt libraries
#QT_DIR=${MY_APPS}/qt-5.15.1
#QT_DIR=/usr
#export PATH=${QT_DIR}/bin:${PATH}
#export LD_LIBRARY_PATH=${QT_DIR}/lib:${LD_LIBRARY_PATH}

#decoder
export HallCBeamtestDir=$HOME/apps/cbeamtest_decoder

export PATH=${HallCBeamtestDir}/bin:${PATH}
export LD_LIBRARY_PATH=.:${HallCBeamtestDir}/lib64:${LD_LIBRARY_PATH}

# load root
source /apps/root/6.22.08/setroot_CUE.bash #too much print out
#echo "load root at $ROOTSYS"

echo '$HallCBeamtestDir'=${HallCBeamtestDir}
#echo '$PATH'=${PATH}
