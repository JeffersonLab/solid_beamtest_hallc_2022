#!/bin/python3

import argparse
import os
import json


with open('database/channels_layout.json', 'r') as f:
    channels_layout = json.load(f)

# format keys
layout = {key.replace(' ', '_').upper(): channels for key, channels in channels_layout.items()}

# lappd channels
layout.update({
    'LAPPD_{}'.format(''.join(grp)): [['LAPPD_{}{}'.format(g, i)] for i in range(1, 9) for g in grp]
for grp in [['A', 'B'], ['C', 'D'], ['E', 'F'], ['G', 'H']]})


# two-dimensional array
def ch_literal(chs):
    subs = []
    for grp in chs:
        subs.append('{\\\"' + '\\\",\\\"'.join(grp) + '\\\"}')
    return '{' + ','.join(subs) + '}'


if __name__ == "__main__":
    parser = argparse.ArgumentParser('draw events')

    parser.add_argument('root_file', help='a root file of waveform data')
    parser.add_argument('channels', help='channels to plot, supports {}'.format(list(layout)))
    parser.add_argument('-o', dest='odir', default="./plots", help='output directory')
    parser.add_argument('-e', default='', dest='events',
                        help='list of events to draw, example: 100,200,300')
    parser.add_argument('-n', dest='nev', type=int, default=5,
                        help='number of events randomly selected to draw, if no list of events provided')
    parser.add_argument('-r', type=int, default=3, dest='res',
                        help='spectrum resolution for analyzer')
    parser.add_argument('-t', type=float, default=20.0, dest='thres',
                        help='spectrum peak height threshold resolution for analyzer')
    parser.add_argument('-p', type=int, default=5, dest='npeds',
                        help='number of samples to check pedestal')
    parser.add_argument('-f', type=float, default=1.0, dest='flat',
                        help='upper limit of the pedestal error (flatness requirement)')

    args = parser.parse_args()

    if args.channels not in layout:
        print('Unsupported channel layout {}, please choose one of {}'.format(args.channels, list(layout)))
        exit(-1)

    # make sure the relative path for root loading is correct
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(os.path.join(script_dir, '..'))
    print('working directory is ' + os.getcwd())

    script = r'scripts/root/draw_events.cxx'
    script_args = '\\\"{}\\\",\\\"{}\\\",{},{{{}}},{},{},{},{},{},\\\"{}\\\"'\
                  .format(args.root_file, args.odir, ch_literal(layout[args.channels]),
                          args.events, args.nev,
                          args.res, args.thres, args.npeds, args.flat,
                          args.channels.lower() + '_')
    # for long argument
    os.system('ulimit -s 65536')
    os.system('root -b -q \"{}+({})\"'.format(script, script_args))

