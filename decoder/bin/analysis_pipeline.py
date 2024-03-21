#!/bin/python3

import os
import argparse
import fnmatch
from collections import OrderedDict


#ANALYZE_BIN = 'build/src/analyze_tracking'
ANALYZE_BIN = 'bin/analyze_tracking'

# get split number of raw data
def get_split_number(fname):
    try:
        return int(fname.split('.')[-1])
    except:
        return -1


# find data
def search_raw_data(dirname, pattern, nmax):
    files = []
    for f in os.listdir(dirname):
        if fnmatch.fnmatch(f, pattern):
            files.append(os.path.join(dirname, f))
        if nmax > 0 and len(files) >= nmax:
            break;
    return sorted(files, key=get_split_number)


def analysis_pipeline(raw_data, proc_data, modules, layouts, **kwargs):
    run = kwargs['run']
    # format commands
    commands = OrderedDict()

    # analyze data
    binary = ANALYZE_BIN
    arguments = [
        '--module=database/modules/{}_module.json'.format(modules.lower()),
        '-r {res} -t {thres} -p {npeds} -f {flat}'.format(**kwargs),
    ]
    commands['analyze'] = [
        '{} {} {} {}'.format(binary, raw, proc, ' '.join(arguments))
        for raw, proc in zip(raw_data, proc_data)
    ]

    # draw raw event samples
    binary = 'python3 scripts/draw_events.py'
    samples_dir = 'plots/run{}/samples'.format(run)
    arguments = [
        '--plot-dir={}'.format(samples_dir),
        '-r {res} -t {thres} -p {npeds} -f {flat}'.format(**kwargs),
    ]
    commands['draw_samples'] = [
        '{} {} {} {}'.format(binary, layout.upper(), proc_data[0], ' '.join(arguments))
        for layout in layouts
    ]

    # raw distributions
    binary = 'python3 scripts/plot_dist.py'
    json_file = 'database/{}_raw_dist.json'.format(modules.lower())
    rawdist_dir = 'plots/run{}/raw'.format(run)
    arguments = [
        '--plot-dir={}'.format(rawdist_dir),
        '--root-plots',
    ]
    commands['raw_dist'] = [
        '{} {} {} {}'.format(binary, proc_data[0], json_file, ' '.join(arguments))
    ]

    # trigger timing cut
    binary = 'python3 scripts/trigger_timing_cuts.py'
    tcuts_json = os.path.join('database', 'run{}_timing.json'.format(run))
    trg_data = os.path.join('processed_data', 'run{}_trg.root'.format(run))
    tcuts_dir = 'plots/run{}/tcuts'.format(run)
    arguments= [
        '--files={}'.format(','.join(proc_data)),
        '--snapshot={}'.format(trg_data),
        '--plot-dir={}'.format(tcuts_dir),
        '-b',
    ]
    commands['trg_tcuts'] = [
        '{} {} {} {}'.format(binary, modules.upper(), tcuts_json, ' '.join(arguments))
    ]

    # signal timing cut
    binary = 'python3 scripts/signal_timing_cuts.py'
    tcuts_data = os.path.join('processed_data', 'run{}_tcuts.root'.format(run))
    arguments= [
        '--snapshot={}'.format(tcuts_data),
        '--plot-dir={}'.format(tcuts_dir),
        '-b',
    ]
    commands['signal_tcuts'] = [
        '{} {} {} {} {}'.format(binary, modules.upper(), trg_data, tcuts_json, ' '.join(arguments))
    ]

    # signal sums
    binary = 'python scripts/plot_sum.py'
    res_dir = 'plots/run{}/results'.format(run)
    arguments = [
        '--plot-dir={}'.format(res_dir),
    ]
    commands['signal_sum'] = [
        '{} {} {} {}'.format(binary, modules.upper(), tcuts_data, ' '.join(arguments))
    ]

    return commands, [res_dir, tcuts_dir, rawdist_dir, samples_dir]


# execute the script
if __name__  == '__main__':
    owd = os.getcwd()
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(os.path.join(script_dir, '..'))
    print('working directory is ' + os.getcwd())

    signal_types = {
        'LAPPD': ['CALORIMETER', 'LAPPD'],
        'MAPMT': ['CALORIMETER', 'MAPMT_SUMS', 'MAPMT_QUADS'],
    }

    # argument parser
    parser = argparse.ArgumentParser('analyze and post plots')
    parser.add_argument('stype', help='signal type')
    parser.add_argument('run', help='run number to analyze')
    parser.add_argument('--post', action='store_true', help='make a post to the logbook for the analysis')
    parser.add_argument('--raw-dir', dest='raw_dir', default='./raw_data', help='raw data directory')
    parser.add_argument('--proc-dir', dest='proc_dir', default='./processed_data', help='processed data directory')
    parser.add_argument('-n', '--nsplit', dest='nsplit', type=int, default=-1,
                        help='maximum number of split evio files')
    parser.add_argument('-r', type=int, default=1, dest='res',
                        help='spectrum resolution for analyzer')
    parser.add_argument('-t', type=float, default=10.0, dest='thres',
                        help='spectrum peak height threshold resolution for analyzer')
    parser.add_argument('-p', type=int, default=10, dest='npeds',
                        help='number of samples to check pedestal')
    parser.add_argument('-f', type=float, default=2.0, dest='flat',
                        help='upper limit of the pedestal error (flatness requirement)')
    args = parser.parse_args()

    if args.stype not in signal_types:
        print('signal type {} must be in {}'.format(args.stype, list(signal_types)))
        quit()

    # recover paths
    for attr in ['raw_dir', 'proc_dir']:
        setattr(args, attr, os.path.join(owd, getattr(args, attr)))

    # search raw data files
    raw_data = search_raw_data(args.raw_dir, 'hallc_fadc_ssp_{}.evio.*'.format(args.run), args.nsplit)

    if not len(raw_data):
        print('no data found!')
        quit()

    # match processed data files
    proc_data = [os.path.join(args.proc_dir, 'run{}_{}.root'.format(args.run, i)) for i, _ in enumerate(raw_data)]

    commands, plot_dirs = analysis_pipeline(raw_data, proc_data, args.stype, signal_types[args.stype], **vars(args))

    steps = [
        'analyze',
        #'draw_samples',
        #'raw_dist',
        #'trg_tcuts',
        #'signal_tcuts',
        #'signal_sum',
    ]

    for step in steps:
        print('STEP - {}'.format(step))
        for command in commands[step]:
            print(command)
            if os.system(command) != 0:
                print('Failed at STEP - {}'.format(step))
                exit(-1)

    # make a post of the results
    if args.post:
        binary = '/site/ace/certified/apps/bin/logentry'
        if not os.path.exists(binary):
            print('Cannot find executable for posting logentry, abort posting.')
            quit()
        body_temp = 'body_temp.txt.tmp'
        body = '<pre>\n'\
               '{} split data files \n'\
               'resolution: -r {res}\n'\
               'threshold: -t {thres}\n'\
               'pedestal window: -p {npeds}\n'\
               'pedestal flatness: -f {flat}\n'\
               '</pre>'.format(len(raw_data), **vars(args))

        arguments = [
            '-t \"Analysis plots: Run {}, Split 0 - {}\"'.format(args.run, len(raw_data) - 1),
            '-b {}'.format(body_temp),
            '-g \"Analysis\"',
            '-l \"SOLID\"',
            '--html',
            '--noqueue',
            # '--nosubmit',
        ]

        # find all plots
        for plot_dir in plot_dirs:
            for (dirpath, dirnames, filenames) in os.walk(plot_dir):
                for fname in filenames:
                    arguments.append('-a {}'.format(os.path.join(plot_dir, fname)))

        os.system('echo \"{}\" > {}'.format(body, body_temp))
        command = '{} {}'.format(binary, ' '.join(arguments))
        print(command)
        os.system(command)

