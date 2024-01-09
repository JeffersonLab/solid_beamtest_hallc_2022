#! /bin/csh 

set usrnam = `whoami`
set run_list = (4680 4778 4779)
#set run_list = (4680)

set output_dir = "/volatile/halla/solid/xbai/replayed_root_files/"
set data_dir = "/volatile/halla/solid/xbai/"

set evio_file = "hallc_fadc_ssp_"#4778.evio.0

set ana_dir = "/work/halla/solid/xbai/hallc_beamtest/beamtest_hallc_decoder"

# create jsubs for each run
foreach rnum ($run_list)
    set jsub = jsc/run$rnum.sh

    if (-f $jsub) then
        rm $jsub
    endif
    echo -n > $jsub

    echo "#! /bin/csh \n" >> $jsub
    echo "#SBATCH --job-name=replay_run$rnum" >> $jsub
    echo "#SBATCH --account=hallb" >> $jsub
    echo "#SBATCH --partition=production" >> $jsub
    echo "#SBATCH --output=/farm_out/$usrnam/sim$rnum-%j-%N.out" >> $jsub
    echo "#SBATCH --error=/farm_out/$usrnam/sim$rnum-%j-%N.err" >> $jsub
    echo "#SBATCH --mail-user=$usrnam@jlab.org" >> $jsub
    echo "#SBATCH --time=24:00:00" >> $jsub
    echo "#SBATCH --mem-per-cpu=600" >> $jsub

    echo "\n" >> $jsub
    echo 'cd /scratch/slurm/$SLURM_JOBID' >> $jsub
    echo "cp $data_dir$evio_file$rnum.evio.* ./" >> $jsub
    echo "cp $ana_dir/setup_xinzhan.csh ./" >> $jsub
    echo "cp -r $ana_dir/config ./" >> $jsub
    echo "cp -r $ana_dir/database ./" >> $jsub
    echo "cp -r $ana_dir/lib64 ./" >> $jsub
    echo "cp -r $ana_dir/bin ./" >> $jsub
    echo "source setup_xinzhan.csh" >> $jsub
    echo "python3 bin/analysis_pipeline.py --raw-dir ./ --proc-dir ./ MAPMT $rnum" >> $jsub
    echo "mv run$rnum*.root $output_dir" >> $jsub
end

foreach rnum ($run_list)
    sbatch jsc/run$rnum.sh
end
