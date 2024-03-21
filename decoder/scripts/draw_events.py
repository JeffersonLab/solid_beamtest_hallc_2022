#!/bin/python3

import argparse
import os
import json
import ROOT
import random
import numpy as np
from ROOT import gROOT, gInterpreter
from plot_utils import prepare_canvas, my_style


# has raw events
def has_raw(t, chans):
    for ch in chans:
        if getattr(t, ch).raw.size() > 0:
            return True
    return False


# randomly select events
def random_select(t, chans, nev, ntests = 10000):
    max_evs = t.GetEntries()
    nev = np.min([nev, max_evs])
    tries = []
    events = []

    print('Randomly select {} events from the file'.format(nev))
    count = 0
    while len(events) < nev and len(tries) < max_evs and count < ntests:
        iev = np.random.randint(0, max_evs)
        count += 1
        if iev not in tries:
            tries.append(iev)
            t.GetEntry(iev)
            if has_raw(t, chans):
                events.append(iev)
        if not len(tries) % 10:
            print('Tried {} events, {} events have non-empty data for the given channels'\
                  .format(len(tries), len(events)), end='\r', flush=True)

    print('Tried {} events, {} events have non-empty data for the given channels'.format(len(tries), len(events)))
    return events


if __name__ == "__main__":
    # make sure the relative path for root loading is correct
    owd = os.getcwd()
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(os.path.join(script_dir, '..'))
    print('working directory is ' + os.getcwd())

    # get layouts
    with open(os.path.join(script_dir, '..', 'database', 'channels_layout.json'), 'r') as f:
        channels_layout = json.load(f)
    # format keys
    layout = {key.replace(' ', '_').upper(): channels for key, channels in channels_layout.items()}

    # argument parser
    parser = argparse.ArgumentParser('draw events')
    parser.add_argument('channels', help='channels layout to plot, supports {}'.format(list(layout)))
    parser.add_argument('root_file', help='a root file of waveform data')
    parser.add_argument('-o', '--plot-dir', dest='out_dir', default="./plots", help='output directory')
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
        print('Cannot find layout {}, please choose one of {}'.format(args.channels, list(layout)))
        exit(-1)

    # recover paths
    for attr in ['root_file', 'out_dir']:
        setattr(args, attr, os.path.join(owd, getattr(args, attr)))

    os.makedirs(args.out_dir, exist_ok=True)

    # batch mode
    gROOT.SetBatch(True)
    # root macros
    gROOT.Macro('rootlogon.C')
    # declare root graph code
    func_code = open(os.path.join(script_dir, '..', 'decoder', 'WfRootGraph.h')).read()
    gInterpreter.Declare(func_code)

    # style
    my_style.cd()

    # root file
    f = ROOT.TFile(args.root_file, 'r')
    tree = f.EvTree
    channels = layout[args.channels]

    # define analyzer
    analyzer = ROOT.fdec.Analyzer(args.res, args.thres, args.npeds, args.flat)

    events = []
    for ev in args.events.split(','):
        try:
            events.append(int(ev))
        except:
            pass

    if not len(events):
        flat_chans = [ch for chans in channels for ch in chans]
        events = random_select(tree, flat_chans, args.nev)

    for iev in events:
        tree.GetEntry(iev)
        title = '{} Event {}'.format(args.channels, iev)
        c = ROOT.TCanvas(title, '', 2560, 1440)
        pads = prepare_canvas(c, channels, title, 'Sample Number', 'ADC Value')
        grs = []
        for chs in channels:
            ymin, ymax = 4000, 0
            for ch in chs:
                pads[ch].cd()
                grs.append(ROOT.fdec.get_waveform_graph(analyzer, getattr(tree, ch).raw))
                grs[-1].mg.SetTitle(ch)
                grs[-1].mg.Draw('A')
                if getattr(tree, ch).raw.size() > 0:
                    new_ymax = grs[-1].mg.GetHistogram().GetMaximum()
                    new_ymin = grs[-1].mg.GetHistogram().GetMinimum()
                    ymax = new_ymax if new_ymax > ymax else ymax
                    ymin = new_ymin if new_ymin < ymin else ymin
                    grs[-1].mg.GetXaxis().SetRangeUser(0, getattr(tree, ch).raw.size() - 1)
            for j in np.arange(0, len(chs)):
                grs[-1 - j].mg.GetYaxis().SetRangeUser(ymin, ymax + (ymax - ymin)*0.1)
                c.Update()

        c.SaveAs(os.path.join(args.out_dir, '{}_event_{}.png'.format(args.channels.lower(), iev)))

