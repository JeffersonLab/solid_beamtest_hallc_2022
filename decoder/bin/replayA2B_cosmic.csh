#!/bin/csh -fb
#  script to run the decoder
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

set datadir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/replay/pass0
set replaydir = /volatile/halla/solid/jixie/ecal_beamtest_hallc/cosmic/pass0

#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/replay/pass0
#set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/7deg/pass0

#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/replay/pass0
#set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/18deg/pass0

if ("$host" == "uvasolid2")  then
	set datadir = /home/solid/data/cosmic
	set replaydir = /home/solid/replay/ROOTFILE/cosmicX
endif


if ($#argv < 1) then
	echo "error: you need to provide at least 1 argument"
	echo "usage: $0 <startrun> [endrun=0] [overwrite=0] [replayoption='-n -1'] [outdir=$replaydir]"
	echo "replayoption must be single-quoted, here is a list of them:"
	echo "usage of build/src/analyze:"
	echo "    build/src/analyze [-n NEV] [-m, --module MODULE] [-r RES] [-t THRES] [-p NPEDS] [-f FLAT] RAW_DATA ROOT_FILE"
	echo "         -h, --help: print help message"
	echo "         -n [NEV (default: -1)]: number of events to process (< 0 means all)"
	echo "         -k [NSKIP (default: 0)]: number of events to skip"
	echo "         -m, --module [MODULE (default: database/modules/mapmt_module.json)]: json file for module configuration"
	echo "         -r [RES (default: 3)]: resolution for waveform analysis"
	echo "         -t [THRES (default: 20.000000)]: peak threshold for waveform analysis"
	echo "         -p [NPEDS (default: 8)]: sample window width for pedestal searching"
	echo "         -f [FLAT (default: 1.000000)]: flatness requirement for pedestal searching"
	echo "         -x [USEFIXEDPED (default: 0)]: whether or not to use fixed FADC pedestals from database"
	echo ""
	echo "example 1: [$0 1000]  will replay for run number 1000"
	echo "example 2: [$0 1000 1005]  will replay for run number from 1000 to 1005(included)"
	echo "example 3: [$0 1000 1005 '-x 1']  will replay for run number from 1000 to 1005(included) with option '-x 1'"
	$DEBUG exit 0
endif


set startrun = ($1)
set endrun = ($1)
if ($#argv > 1) set endrun = ($2)
if ( $endrun < $startrun ) set endrun = $startrun

set overwrite = (0)
if ($#argv >= 3) set overwrite = ($3)

set replayoption = '-n -1'
if ($#argv >= 4) set replayoption = ($4)

set outdir = $replaydir
if ($#argv >= 5) set outdir = ($5)

#get absolute path for $replaydir and $outdir
set curdir = (`pwd`)
cd $replaydir; set replaydir = (`pwd`);
cd $outdir; set outdir = (`pwd`);
cd $curdir


#####check for write permission#######
mkdir -p $replaydir/.check
if !(-d $replaydir/.check) then
	echo "You have no write permission in $replaydir, I quit ..."
	exit 0
else
	rm -fr $replaydir/.check
endif
mkdir -p $outdir/.check
if !(-d $outdir/.check) then
	echo "You have no write permission in $replaydir, I quit ..."
	exit 0
else
	rm -fr $outdir/.check
endif


#############Loop over runs ###########
@ run = $startrun - 1
while ($run < $endrun)

	@ run = $run + 1

	set nfile = (0)

	(ls -1 $datadir/hallc_fadc*_${run}.evio.*| wc | awk '{print " " $1}' >! ~/.tmp_$$) >&! /dev/null
	set nfile = (`cat  ~/.tmp_$$`)
	rm -fr ~/.tmp_$$  >&! /dev/null
	if ($nfile < 1) continue

	foreach infile ($datadir/hallc_fadc*_${run}.evio.*)

		set subrun = (${infile:e})
		set outfilename = beamtest_hallc_${run}_${subrun}.root
		set outfile = $replaydir/$outfilename
	set outlevel1file = $replaydir/beamtest_level1_${run}_${subrun}.root

		if ($overwrite == 0 && -f $outdir/$outfilename) then
			echo "$outfile exist, skip replaying this file ......"
			continue
		endif

		#echo "start replaying $infile  --> $outfile"
		#remove the local output file if exists, will move it to $outfile when finish
		if (-f $outfilename) $DEBUG rm -f $outfilename

		$DEBUG $decoderdir/bin/replayFiles_cosmic.csh "$replayoption" $infile

	#moving files from $replaydir to $outfir
	if ("$replaydir" != "$outdir") then
		$DEBUG mv $outfile $outlevel1file $outdir/.
	endif
	end #end of for loop

end #end of while loop
