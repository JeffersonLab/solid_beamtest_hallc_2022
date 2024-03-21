#!/bin/csh -fb
#  script to run the fadc250 decoder
#  remember to source env setup file $decoderdir/env/setup_env.csh before execute this script
#
#set DEBUG = ( echo ); 
set DEBUG = (""); 

if (! $?HallCBeamtestDir) setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
set decoderdir = ${HallCBeamtestDir}
set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
#set replaydir = $decoderdir/../ROOTFILE
set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/pass1/ROOTFILE

if($#argv < 1) then
    echo "error: you need to provide at least 1 argument"
    echo "usage: $0 <run> [overwrite=0]"
    $DEBUG exit 0
endif

set run = ($1)

set overwrite = (0)
if ($#argv >= 2) set overwrite = ($2)

set replayoption = "-m $decoderdir/database/modules/mapmt_module_2FADC_beamtest.json "

#check for write permission
mkdir -p $replaydir/.check
if !(-d $replaydir/.check) then
  echo "You have no write permission in $replaydir, I quit ..."
  exit 0
else
  rm -fr $replaydir/.check
endif

#set the file to proper group
$DEBUG chown jixie.12gev_solid $datadir/hallc_fadc_ssp_${run}.evio.*


#For some reason, this error message was spit out: "module: Command not found" 
#source $HallCBeamtestDir/setup.csh
setenv ROOTSYS /u/home/pcrad/apps/root-6.22.02
setenv PATH ${HallCBeamtestDir}/bin:$ROOTSYS/bin:/group/jpsi-007/local/stow/gcc_8.2.0/bin:${PATH}
setenv LD_LIBRARY_PATH ${HallCBeamtestDir}/lib64:$ROOTSYS/lib:/group/jpsi-007/local/stow/gcc_8.2.0/lib64:${LD_LIBRARY_PATH}

###################################################################################
#build/src/analyze -m database/modules/mapmt_module_2FADC.json  /cache/halla/solid/subsystem/hgc/2020benchtest/coda/solidhgc_${run}.evio.0 /w/halla-scshelf2102/solid/cc_pro/benchtest/coda/root/solidhgc_${run}.root
#
# First argument is reads the json file for the FADC slot to be decoded
# IN above examle we have 2 slots 6 and slot 7 "database/modules/mapmt_module_2FADC.json" 
# The other argument gives the raw evio file to be decoded:
#"/cache/halla/solid/subsystem/hgc/2020benchtest/coda/solidhgc_${run}.evio.0"
#
#3) Gives the path where the decoded root file is kept 
#"/w/halla-scshelf2102/solid/cc_pro/benchtest/coda/root/solidhgc_${run}.root"
###################################################################################

set curdir = (`pwd`) 
if (! $?WORKDIR) setenv WORKDIR $curdir/replay_${HOST:r:r}_${run}_$$
mkdir -p $WORKDIR/graph
if (! -d $WORKDIR) then
  echo "can not create WORKDIR $WORKDIR, I quit ..."
  exit -1
endif
cd $WORKDIR;set WORKDIR = (`pwd`);cd $curdir


foreach infile ($datadir/hallc_fadc_ssp_${run}.evio.*)
  set subrun = (${infile:e})
  set outfilename = beamtest_hallc_${run}_${subrun}.root
  set outfile = $replaydir/$outfilename

  if ($overwrite == 0 && -f $outfile) then
    echo "$outfile exist, skip replaying this file ......"
    continue
  endif
    
  #echo "start replaying $infile  --> $outfile"
  #remove the local output file if exists, will move it to $outfile when finish
  if (-f $outfilename) $DEBUG rm -f $outfilename
  
  #due to bin/analyze require ./config and ./database to run
  # I have to copy them  into $WORKDIR
  cd $WORKDIR
  $DEBUG cp -fr $decoderdir/config $decoderdir/database $WORKDIR/
  if ($run >= 3350) then
    $DEBUG cp -f $WORKDIR/config/gem.conf_30m $WORKDIR/config/gem.conf
  else if ($run >= 3320) then
    $DEBUG cp -f $WORKDIR/config/gem.conf_20m $WORKDIR/config/gem.conf
  else      
    $DEBUG cp -f $WORKDIR/config/gem.conf_10m $WORKDIR/config/gem.conf
  endif  
  $DEBUG $decoderdir/bin/analyze $replayoption $infile $outfilename
  
  #extract pedestal and create level1 tree
  if (-f $outfilename) then
    $DEBUG mv -f $outfilename $outfile
    $DEBUG root -b -q $outfile $decoderdir/CreateLevel1Tree/run.C+
  endif
  cd $curdir
  
end

rm -fr $WORKDIR


