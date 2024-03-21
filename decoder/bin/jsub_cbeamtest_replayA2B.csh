#!/bin/csh -fb
# csh script to submit batch farm jobs for replaying hallc beamtest runs in /cache
# one job for each run, about 15 minutes per file
########################################################################
set DEBUG = ( echo );
set DEBUG = ("");

if (! $?HallCBeamtestDir) setenv HallCBeamtestDir /work/halla/solid/jixie/ecal_beamtest_hallc/decoder
set replaydir = /work/halla/solid/$user/ecal_beamtest_hallc/ROOTFILE

if ($#argv < 1) then
    echo "Submit jobs to replaying hallc beamtest runs" 
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
#create the $template
set template = ($jobfiledir/job_hallc_beamtest_T)
echo "create template file $template"
# there has to be '\!' in the next line
echo "#\!/bin/bash" >&! $template
echo "#SBATCH --partition=production" >> $template
echo "#SBATCH --account=hallc" >> $template
echo "#SBATCH --mem-per-cpu=4096" >> $template
echo "#SBATCH --time=72:00:00" >> $template
echo "#SBATCH --gres=disk:25000" >> $template

############################################################
#submit jobs for each run

set mssdir = /mss/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw
set datadir = /cache/halla/solid/subsystem/ec/ecal_beamtest_hallc/raw

@ run = $startrun - 1
while ($run < $endrun)

  @ run = $run + 1
  
  set nfile = (0)  
  (ls -1 $datadir/hallc_fadc_ssp_${run}.evio.*| wc | awk '{print " " $1}' >! ~/.tmp_$$) >&! /dev/null  
  set nfile = (`cat  ~/.tmp_$$`)
  rm -fr ~/.tmp_$$  >&! /dev/null   
  if ($nfile < 1) continue
   
  set jobfile = (${jobfiledir}/job_replay_cbeamtest_${run})
  
  cp -f $template $jobfile
  echo "#SBATCH --job-name=C_${run}_replay" >> $jobfile  
  echo "$HallCBeamtestDir/bin/replayA2B.csh ${run} ${run} ${overwrite} ${replaydir}" >> $jobfile
 
  cat $jobfile
  $DEBUG sbatch $jobfile

##$DEBUG sbatch << END_OD_SBATCH
#!/bin/bash
#SBATCH --job-name=replay_cbeamtest_${run}
#SBATCH --account=hallc
#SBATCH --partition=production
#SBATCH --mem-per-cpu=4096
#SBATCH --time=72:00:00
#SBATCH --gres=disk:25000
##$HallCBeamtestDir/bin/replayA2B.csh ${run} ${run} ${overwrite} ${replaydir}

##END_OF_SBATCH

end #end of while loop


