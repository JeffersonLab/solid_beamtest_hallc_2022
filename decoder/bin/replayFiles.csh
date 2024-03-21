#!/bin/csh -fb
#  script to run the decoder, this script will always overwrite the output files
#
set DEBUG = ( echo ); 
set DEBUG = (""); 

if (! $?HallCBeamtestDir) setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
set decoderdir = ${HallCBeamtestDir}
set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
#set replaydir = $decoderdir/../ROOTFILE
set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/pass1/ROOTFILE

if ($#argv < 2) then
	echo "error: you need to provide at least 2 arguments"
	echo "usage: $0 <'replay_option'> <File_List>"
	echo "example 1: $0:t '-n -1' hallc_fadc_ssp_3049.evio.?"
	echo "example 2: $0:t '-m $decoderdir/database/modules/mapmt_module_2FADC_beamtest.json -n 5000' hallc_fadc_ssp_3049.evio.2 hallc_fadc_ssp_3049.evio.3"
	echo "replayoption must be single-quoted, here is a list of them:"
	echo "usage of build/src/analyze:"
	echo "build/src/analyze [-n NEV] [-m, --module MODULE] [-r RES] [-t THRES] [-p NPEDS] [-f FLAT] RAW_DATA ROOT_FILE"
	echo "         -h, --help: print help message"
	echo "         -n [NEV (default: -1)]: number of events to process (< 0 means all)"
	echo "         -k [NSKIP (default: 0)]: number of events to skip"
	echo "         -m, --module [MODULE (default: database/modules/mapmt_module.json)]: json file for module configuration"
	echo "         -r [RES (default: 3)]: resolution for waveform analysis"
	echo "         -t [THRES (default: 10.000000)]: peak threshold for waveform analysis"
	echo "         -p [NPEDS (default: 8)]: sample window width for pedestal searching"
	echo "         -f [FLAT (default: 1.000000)]: flatness requirement for pedestal searching"
	echo "         -x [USEFIXEDPED (default: 0)]: whether or not to use fixed FADC pedestals from database"
	$DEBUG exit 0
endif

set jsonfile = "-m $decoderdir/database/modules/mapmt_module_2FADC_beamtest.json "
set replayoption = ("$jsonfile $1")
echo "replayoption='$replayoption'"


#get absolute path for replaydir
set curdir = (`pwd`);
cd $replaydir;set replaydir = (`pwd`);cd $curdir

#check for write permission
mkdir -p $replaydir/.check
if !(-d $replaydir/.check) then
	echo "You have no write permission in $replaydir, I quit ..."
	exit 0
else
	rm -fr $replaydir/.check
endif

#For some reason, this error message was spit out: "module: Command not found" 
#source $HallCBeamtestDir/setup.csh
setenv ROOTSYS /u/home/pcrad/apps/root-6.22.02
setenv PATH ${HallCBeamtestDir}/bin:$ROOTSYS/bin:/group/jpsi-007/local/stow/gcc_8.2.0/bin:${PATH}
setenv LD_LIBRARY_PATH ${HallCBeamtestDir}/lib64:$ROOTSYS/lib:/group/jpsi-007/local/stow/gcc_8.2.0/lib64:${LD_LIBRARY_PATH}

###################################################################################
#build/src/analyze -m database/modules/mapmt_module_2FADC.json  \
#  /cache/halla/solid/subsystem/hgc/2020benchtest/coda/solidhgc_${run}.evio.0 \
#  /w/halla-scshelf2102/solid/cc_pro/benchtest/coda/root/solidhgc_${run}.root
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
if ( $?SLURM_JOBID ) then
	set WORKDIR = /scratch/slurm/$SLURM_JOBID/replay_${HOST:r:r}_$$
else if ("$replaydir" =~ "*/cache/*") then
	set WORKDIR = $monitordir/../job/replay_${HOST:r:r}_$$
else
	set WORKDIR = $replaydir/../job/replay_${HOST:r:r}_$$
endif
mkdir -p $WORKDIR/graph
if (! -d $WORKDIR) then
	echo "can not create WORKDIR $WORKDIR, I quit ..."
	exit -1
endif
cd $WORKDIR;set WORKDIR = (`pwd`);cd $curdir


foreach infile0 ($argv[2-$#argv])
	if !(-f $infile0)  then
		echo "file $infile0 not exist, skipped ..."
		continue
	endif

	#get the absolute path of infile
	set filename = (`basename $infile0`)
	set infiledir0 = (`dirname $infile0`)
	cd $infiledir0
	set infiledir = (`pwd`) 
	set infile = $infiledir/$filename
	 
	#set the file to proper group
	$DEBUG chown jixie.12gev_solid $infile
	
	set run = (`echo $infile:t:r:r | awk -F'ssp_' '{print $2}'`)
	if ("$run" == "") then
		set run = (`echo $infile:t:r:r | awk -F'ti_' '{print $2}'`)
	endif
	set subrun = (${infile:e})
	set outfilename = beamtest_hallc_${run}_${subrun}.root
	set outfile = $replaydir/$outfilename
	
	#echo "start replaying $infile  --> $outfile"
	#remove the local output file if exists, will move it to $outfile when finish
	if (-f $outfilename) $DEBUG rm -f $outfilename

	#due to bin/analyze require ./config and ./database to run
	# I have to copy them  into $WORKDIR
	cd $WORKDIR
	$DEBUG cp -fr $decoderdir/config $decoderdir/database $WORKDIR
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
	
end #end of loop

rm -fr $WORKDIR

