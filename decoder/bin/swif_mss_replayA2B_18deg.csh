#!/bin/csh -fb
# csh script to submit batch farm jobs for replaying hallc beamtest files in /mss
# one job for each file, about 15 minutes per file
# this script will cook all files in /mss and /cache with given run number
########################################################################
set DEBUG = ( echo );
set DEBUG = ("");

if (! $?HallCBeamtestDir) setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
set decoderdir = ${HallCBeamtestDir}

#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/replay/pass1
#set replaydir = /volatile/halla/solid/jixie/ecal_beamtest_hallc/82deg/pass1

#set datadir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_cosmic_hallc/replay/pass0
#set replaydir = /volatile/halla/solid/jixie/ecal_beamtest_hallc/cosmic/pass0

#set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_7deg/replay/pass1
#set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/7deg/pass1

set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/raw
#set replaydir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc_18deg/replay/pass0
set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/18deg/GEMROOTFILE
#set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/18deg/thre3

if ($#argv < 1) then
	echo "Submit jobs to replaying hallc beamtest files in mss" 
	echo "Usage: $0:t <startrun> [endrun=0] [overwrite=0] [outputdir=$replaydir]"
	echo "example 1: '$0 1000'  will replay for run number 1000"
	echo "example 2: '$0 1000 1005'  will replay for run number from 1000 to 1005(included)"
	echo "example 3: '$0 1000 1005 ./outdir'  will replay for run number from 1000 to 1005(included) and output file is at ./outdir"
	$DEBUG exit 0
endif

########################################################################

set startrun = ($1)
set endrun = ($1)
if ($#argv > 1) set endrun = ($2)
if ( $endrun < $startrun ) set endrun = $startrun

set overwrite = (0)
if ($#argv >= 3) set overwrite = ($3)

if ($#argv >= 4) set replaydir = ($4)

#check for write permission
mkdir -p $replaydir/.check
if !(-d $replaydir/.check) then
	echo "You have no write permission in $replaydir, I quit ..."
	exit 0
else
	rm -fr $replaydir/.check
endif

#the template loate at ./job_hallc_beamtest_T
#
#create the job script template
set jobfiledir = (`pwd`/JobFiles)
mkdir -pv $jobfiledir;
############################################################
#create the $jobfile
set jobfile = ($jobfiledir/swif_${startrun}_to_${endrun})

set workflow = replay_tracking
#echo "create workflow $workflow"
echo "swif2 create -workflow $workflow" >&! $jobfile
$DEBUG swif2 create -workflow $workflow

############################################################
#add job for each run into workflow

set  mssdir = `echo $datadir| sed -e "s/cache/mss/"`

@ njob = 0
@ run = $startrun - 1
while ($run < $endrun)

	@ run = $run + 1
	
	set nfile = (0)  
	(ls -1 $mssdir/hallc_fadc*_${run}.evio.* $datadir/hallc_fadc*_${run}.evio.*| wc | awk '{print " " $1}' >! ~/.tmp_$$) >&! /dev/null  
	set nfile = (`cat  ~/.tmp_$$`)
	rm -fr ~/.tmp_$$  >&! /dev/null   
	if ($nfile < 1) continue  
	 
	foreach infile ($mssdir/hallc_fadc*_${run}.evio.* $datadir/hallc_fadc*_${run}.evio.*)
		
		set infilename = (`basename $infile`)
		
		#check if this file exist in /cache, if yes, skip it to avoid repeating cooking
		set parentdir = (`dirname $infile`)
		if ( "$parentdir" =~ */mss/* ) then
			if ( -f $datadir/$infilename )  continue;
		endif
		
		set subrun = (${infile:e})
		set outfilename = beamtest_hallc_${run}_${subrun}.root
		set outfile = $replaydir/$outfilename
 
		#####################################################
		
		#replayFiles.csh will always overwrite the output file in its own output dir ${HallCBeamtestDir}/../ROOTFILE
		#which is the same as the default $replaydir, therefore do not add job if the output file exists
		if ($overwrite == 0 && -f $outfile) then
			echo "$outfile exist, skip replaying this file ......"
			continue
		endif
		
		#add '"' to escape the command string
		set cmd = ($HallCBeamtestDir/bin/replayFiles_18deg.csh "'-x 1 -t 6'" $datadir/$infilename)
		echo "adding one job for file $infilename"
		if (! -f $datadir/$infilename) then
			#do job from /mss
			echo  "swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -time 96h -input $datadir/$infilename mss:$mssdir/$infilename $cmd " >> $jobfile
			$DEBUG swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -time 96h -input $datadir/$infilename mss:$mssdir/$infilename $cmd  
			#copy the file from /mss to /cache
			echo "jcache get $infile"
			$DEBUG jcache get $infile
		else
			#do job from /cache, 
			echo  "swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -time 96h $cmd " >> $jobfile
			$DEBUG swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -time 96h $cmd  
		endif
		@ njob = $njob + 1
		
	end #end foreach loop

end #end of while loop

if ($njob > 0) then
	echo "swif2 run $workflow " >> $jobfile
	$DEBUG swif2 run $workflow 
	echo "$njob added. Now start running the job in batch farm ..."
endif
