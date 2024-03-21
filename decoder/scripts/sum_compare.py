#!/bin/python3

import argparse
import os
import json
import ROOT
import random
import numpy as np
from ROOT import gROOT, gInterpreter
from plot_utils import prepare_canvas, my_style, nthreads


if __name__ == "__main__":
    ROOT.ROOT.EnableImplicitMT(nthreads)
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
    parser.add_argument('--min', dest='min', default=1, type=int, help='minimum number of hits')
    parser.add_argument('--max', dest='max', default=10, type=int, help='maximum number of hits')
    parser.add_argument('-o', '--plot-dir', dest='out_dir', default="./plots", help='output directory')
    parser.add_argument('-t', type=float, default=0.0, dest='thres', help='peak height threshold to sum')
    args = parser.parse_args()

    # recover paths
    for attr in ['out_dir']:
        setattr(args, attr, os.path.join(owd, getattr(args, attr)))

    os.makedirs(args.out_dir, exist_ok=True)

    # batch mode
    gROOT.SetBatch(True)
    # root macros
    gROOT.Macro('rootlogon.C')
    # declare root graph code
    func_code = open(os.path.join(script_dir, 'root', 'dataframe_analysis.C')).read()
    gInterpreter.Declare(func_code)

    # style
    my_style.cd()

    # root dataframes
    runs = [
        ('CO_{2}', {'SetLineColor': [38], 'SetFillColorAlpha': [38, 0.5], 'SetFillStyle': [3004]}, [
            'processed_data/run427_tcuts.root',
        ]),

        ('C_{4}F_{8} (80%)', {'SetLineColor': [46], 'SetFillColorAlpha': [46, 0.5], 'SetFillStyle': [3005]}, [
            'processed_data/run450_tcuts.root',
        ]),

        ('C_{4}F_{8} (100%)', {'SetLineColor': [30], 'SetFillColorAlpha': [30, 0.5], 'SetFillStyle': [3006]}, [
            'processed_data/run459_tcuts.root',
        ]),
    ]

    xcol, xlbl, hrange = 'sum_height', 'ADC Value', (300, 0, 3000)
    c = ROOT.TCanvas('Total Sum', '', 1920, 1080)
    pad = prepare_canvas(c, [[0]], 'Signal Sums Total', xlbl, 'Normalized Counts')[0]
    
    channels = ['{}_peak'.format(c) for chs in layout['LAPPD'] for c in chs]

    pad.cd()
    pad.SetLogy()
    hists = []
    hs = ROOT.THStack
    for i, (name, settings, root_files) in enumerate(runs):
        chain = ROOT.TChain('EvTree')
        for rfile in root_files:
            chain.Add(rfile)
        rdf = ROOT.RDataFrame(chain)
        rdf = rdf.Define('sum_peak', 'peak_sum({}, {})'.format('{' + ','.join(channels) + '}', args.thres))\
                 .Define('sum_height', 'sum_peak.height')\
                 .Filter('sum_height > 0 && sum_peak.pos > 0')

        hist = rdf.Histo1D(ROOT.RDF.TH1DModel(name, '', *hrange), xcol)
        for method, inputs in settings.items():
            method_to_call = getattr(hist, method)
            method_to_call(*inputs)
        chist = hist.DrawNormalized('same')
    pad.BuildLegend(0.5, 0.8, 0.9, 0.9)
    c.SaveAs(os.path.join(args.out_dir, 'lappd_sum_comp.png'))

