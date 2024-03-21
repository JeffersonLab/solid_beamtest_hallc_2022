#!/bin/csh -fb
#  script to run the decoder, this script will always overwrite the output files
#
set DEBUG = ( echo );
set DEBUG = ("");

set host = (`hostname -s`)
if (! $?HallCBeamtestDir) then
	if ("$host" != "uvasolid2") then
		setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
	else
		setenv HallCBeamtestDir /home/solid/apps/cbeamtest_decoder
	endif
endif
set decoderdir = ${HallCBeamtestDir}
#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/replay/pass1
#set replaydir = /volatile/halla/solid/jixie/ecal_beamtest_hallc/82deg/pass1

#set datadir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/replay/pass0
#set replaydir = /volatile/halla/solid/jixie/ecal_beamtest_hallc/cosmic/pass0

set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/replay/pass1
set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/7deg/pass1

#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/replay/pass0
#set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/18deg/ROOTFILE

set monitordir = /volatile/halla/solid/$user/ecal_beamtest_hallc/monitor_7deg
if ("$host" == "uvasolid2")  then
	set datadir = /home/solid/data
	set replaydir = /home/solid/replay/ROOTFILE
	set monitordir = /home/solid/replay/monitor
endif


if ($#argv < 2) then
	echo "error: you need to provide at least 2 arguments"
	echo "usage: $0 <'replay_option'> <File_List>"
	echo "example 1: $0:t '-n -1' hallc_fadc_ssp_3049.evio.?"
	echo "example 2: $0:t '-m $decoderdir/database/modules/mapmt_module_highrate.json -n 50000' hallc_fadc_ssp_3049.evio.2 hallc_fadc_ssp_3049.evio.3"
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
if ( "$host" == "uvasolid2" ) then
	setenv ROOTSYS /u/apps/root/6.22.08/root
	setenv PATH ${HallCBeamtestDir}/bin:$ROOTSYS/bin:${PATH}
	setenv LD_LIBRARY_PATH ${HallCBeamtestDir}/lib64:$ROOTSYS/lib:${LD_LIBRARY_PATH}
else
	setenv ROOTSYS /u/home/pcrad/apps/root-6.22.02
	setenv PATH ${HallCBeamtestDir}/bin:$ROOTSYS/bin:/group/jpsi-007/local/stow/gcc_8.2.0/bin:${PATH}
	setenv LD_LIBRARY_PATH ${HallCBeamtestDir}/lib64:$ROOTSYS/lib:/group/jpsi-007/local/stow/gcc_8.2.0/lib64:${LD_LIBRARY_PATH}
endif

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

####create WORKDIR, do not put it into /cache###
if ( $?SLURM_JOBID ) then
	set WORKDIR = /scratch/slurm/$SLURM_JOBID/replay_${HOST:r:r}_$$
else if ("$replaydir" =~ "*/cache/*") then
	set WORKDIR = $monitordir/../job/replay_${HOST:r:r}_$$
else
	set WORKDIR = $replaydir/../job/replay_${HOST:r:r}_$$
endif
mkdir -p $monitordir $WORKDIR/graph
if (! -d $WORKDIR) then
	echo "can not create WORKDIR $WORKDIR, I quit ..."
	exit -1
endif
cd $WORKDIR;set WORKDIR = (`pwd`);cd $curdir
##################################################


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

	#if filename contains _ssp, tell the decoder to decode ssp data
	set useSSP = (`echo $filename | grep _ssp | awk '{print length($0);}'`)

	#set the file to proper group
	$DEBUG chown jixie.12gev_solid $infile

	set run = (`echo $filename:t:r:r | awk -F'_' '{print $NF}'`)
	set firstchar = `echo ${run} | awk '{print substr($0,0,1)}'`
	set lastchar = `echo ${run} | awk '{print substr($0,length($0),1)}'`
	if (("$firstchar" !~ [0-9]) || ("$lastchar" !~ [0-9])) then
		echo "could not extract run number from file name $filename, skip ..."
		continue
	endif
	set subrun = (${infile:e})
	set outfilename = beamtest_hallc_${run}_${subrun}.root
	set outfile = $replaydir/$outfilename
	set outlevel1file = $replaydir/beamtest_level1_${run}_${subrun}.root

	#lowrate: <= 3683
	#cosmic1: 3684 - 3860, PreSh left and right swapped, Shower has 120deg rotation
	#cosmic2: 3861 - 3885, fixed the above
	#cosmic3: 3886 - 3896, HV scan for Preshower and Shower
	#cosmic4: 3897 - 3905, dedicated PreShower calibration runs, 3 preshower stacked to one pile, no GEM
	#highrate: >= 3906

	set jsonfile = "mapmt_module_2FADC_beamtest.json"
	set cscriptfile = "run.C"
	if ($run <= 3683) then
		set jsonfile = "mapmt_module_2FADC_beamtest.json"
	set cscriptfile = "run.C"
	else if ($run >= 3684 && $run <= 3860) then
		set jsonfile = "mapmt_module_cosmic1.json"
		set cscriptfile = "run_cosmic1.C"
	else if ($run >= 3861 && $run <= 3885) then  #cosmic2 and cosmic3 has GEM
		set jsonfile = "mapmt_module_cosmic4.json"
		set cscriptfile = "run_cosmic4.C"
	else if ($run >= 3886 && $run <= 3905) then  #cosmic4 has NO  GEM
		set jsonfile = "mapmt_module_cosmic4.json"
		set cscriptfile = "run_cosmic4.C"
	else if ($run >= 3906 && $run <= 4125) then
		set jsonfile = "mapmt_module_highrate_ssp.json"   #//with GEM
		if ($useSSP < 1) set jsonfile = "mapmt_module_highrate.json"   #//without GEM
		set cscriptfile = "run_highrate_jx.C"
	else #if ($run >= 4126) then
		set jsonfile = "mapmt_module_highrate_ssp.json"   #//with GEM
		if ($useSSP < 1) set jsonfile = "mapmt_module_highrate.json"   #//without GEM
		set cscriptfile = "run_highrate_jx.C"
	endif

	set replayoption = ("-m $decoderdir/database/modules/$jsonfile $1")
	echo "replayoption='$replayoption'"

	#echo "start replaying $infile  --> $outfile"
	#remove the local output file if exists, will move it to $outfile when finish
	if (-f $outfilename) $DEBUG rm -f $outfilename

	#due to bin/analyze require ./config and ./database to run
	# I have to copy them  into $WORKDIR
	cd $WORKDIR
	$DEBUG cp -fr $decoderdir/config $decoderdir/database $WORKDIR
	$DEBUG cp -fr $decoderdir/rootlogon.C $WORKDIR

	#this will set pedestal files for GEM
	if ($run <= 3319) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_10m $WORKDIR/config/gem.conf
	else if ($run >= 3320 && $run <= 3349) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_20m $WORKDIR/config/gem.conf
	else if ($run >= 3350 && $run <= 3683) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_30m $WORKDIR/config/gem.conf
	else if ($run >= 3684 && $run <= 3905) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_10m $WORKDIR/config/gem.conf
	else if ($run >= 3906 && $run <= 4296) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_highrate $WORKDIR/config/gem.conf
	else if ($run >= 4297) then
		$DEBUG cp -f $WORKDIR/config/gem.conf_highrate2 $WORKDIR/config/gem.conf
	endif

	$DEBUG $decoderdir/bin/analyze $replayoption $infile $outfilename

	#create level1 tree and plot monitoring histo
	if (-f $outfilename) then
		$DEBUG mv -f $outfilename $outfile
		#$DEBUG ln -sf $decoderdir/rootlogon.C .
		$DEBUG root -b -q $outfile $decoderdir/CreateLevel1Tree/${cscriptfile}+

		if ($subrun == 0) then
			if (-f Pedastal.inc) $DEBUG cat Pedastal.inc >> $monitordir/Pedastal.inc
			$DEBUG root -b -q $outfile $decoderdir/CreateLevel1Tree/FindTriggerPeak.C+
			if (-f Peak.inc) $DEBUG cat Peak.inc >> $monitordir/Peak.inc
			if (-f $outlevel1file) then
				$DEBUG root -b -q $outlevel1file $decoderdir/CreateLevel1Tree/plot_level1.C+
				$DEBUG cp -fr gr_monitor $monitordir/.
		endif
		$DEBUG cp -fr graph $monitordir/.
	endif
	cd $curdir

end #end of loop

rm -fr $WORKDIR

