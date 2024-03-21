#!/bin/csh -fb
# csh script to submit batch farm jobs for replaying hallc beamtest files in /mss
# one job for each file, about 15 minutes per file
########################################################################
set DEBUG = ( echo );
set DEBUG = ("");

if (! $?HallCBeamtestDir) setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
set decoderdir = ${HallCBeamtestDir}
set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
#set replaydir = $decoderdir/../ROOTFILE
set replaydir = /volatile/halla/solid/$user/ecal_beamtest_hallc/pass1/ROOTFILE

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

set workflow = replay_cbeamtest
#echo "create workflow $workflow"
echo "swif2 create -workflow $workflow" >&! $jobfile
$DEBUG swif2 create -workflow $workflow

############################################################
#add job for each run into workflow

set mssdir = /mss/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw

@ njob = 0
@ run = $startrun - 1
while ($run < $endrun)

  @ run = $run + 1
  
  set nfile = (0)  
  (ls -1 $mssdir/hallc_fadc_ssp_${run}.evio.*| wc | awk '{print " " $1}' >! ~/.tmp_$$) >&! /dev/null  
  set nfile = (`cat  ~/.tmp_$$`)
  rm -fr ~/.tmp_$$  >&! /dev/null   
  if ($nfile < 1) continue
   
  foreach infile ($mssdir/hallc_fadc_ssp_${run}.evio.*)
    
    set subrun = (${infile:e})
    set infilename = (`basename $infile`)
    set outfilename = beamtest_hallc_${run}_${subrun}.root
    set outfile = $replaydir/$outfilename
 
    #####################################################
    #set jgetonly to +1 if only copy files not in /cache
    #set jgetonly to  0 if cook all files /mss, will automaticaly copy files from  /mss to /cache
    #set jgetonly to -1 if only cook files not in /cache
    set jgetonly = (0)
    if ( $jgetonly > 0 ) then
      if (! -f $datadir/$infilename) then
        $DEBUG jcache get $infile
      else
        echo "$datadir/$infilename exist, skip jget for this file ......"
      endif
      continue
    endif
    if ((-f $datadir/$infilename) && $jgetonly < 0) then
      continue;
    endif
    #####################################################
    
    #replayFiles.csh will always overwrite the output file in its own output dir ${HallCBeamtestDir}/../ROOTFILE
    #which is the same as the default $replaydir, therefore do not add job if the output file exists
    if ($overwrite == 0 && -f $outfile) then
      echo "$outfile exist, skip replaying this file ......"
      continue
    endif
    
    #TODO: 
    #1) when "-input localfile remotefile" option is used, what if the localfile exists, does the process still 
    #   copy the remotefile to overwrite the localfile?
    #2) When a file in /cache is updated, does the tape will update that file already in tape?
    
    #add '"' to escape the command string
    set cmd = ($HallCBeamtestDir/bin/replayFiles.csh "'-n -1'" $datadir/$infilename)
    echo "adding one job for file $infilename"
    if (! -f $datadir/$infilename) then
      #do job from /mss
      echo  "swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -input $datadir/$infilename mss:$mssdir/$infilename $cmd " >> $jobfile
      $DEBUG swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 -input $datadir/$infilename mss:$mssdir/$infilename $cmd  
    else
      echo  "swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 $cmd " >> $jobfile
      $DEBUG swif2 add-job $workflow -account halla -name ${run}_${subrun}_replay -partition production -ram 2g -phase 1 $cmd  
    endif
    @ njob = $njob + 1
    
  end #end foreach loop

end #end of while loop

if ($njob > 0) then
  echo "swif2 run $workflow " >> $jobfile
  $DEBUG swif2 run $workflow 
  echo "$njob added. Now start running the job in batch farm ..."
endif
