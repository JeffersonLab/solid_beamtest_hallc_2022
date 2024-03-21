import argparse
import os
import json
import ROOT
import random
import numpy as np
from ROOT import gROOT, gInterpreter
from plot_utils import prepare_canvas, my_style, nthreads

ROOT.ROOT.EnableImplicitMT(nthreads)

if __name__ == "__main__":
    # batch mode
    gROOT.SetBatch(True)

    # make sure the relative path for root loading is correct
    owd = os.getcwd()
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(os.path.join(script_dir, '..'))
    print('working directory is ' + os.getcwd())

    # signal layouts
    layout_json = os.path.join(script_dir, '..', 'database', 'channels_layout.json')
    with open(layout_json, 'r') as f:
        channels_layout = json.load(f)
    signal_types = {
        'LAPPD': [
            ('Pixels', 'lappd_all', np.arange(5, 21), 'Peak Height (ADC Channel)', (200, 0, 2000))
        ],
        'MAPMT': [
            ('PMTs', 'mapmt_sums_all', np.arange(1, 10), 'Peak Height (ADC Channel)', (200, 0, 6000)),
            ('Quads', 'mapmt_quads_all', np.arange(5, 21), 'Peak Height (ADC Channel)', (200, 0, 6000)),
        ]
    }

    # argument parser
    parser = argparse.ArgumentParser('draw events')
    parser.add_argument('stype', help='signal type, supporting: {}'.format(list(signal_types)))
    parser.add_argument('root_file', help='a root file of waveform data')
    parser.add_argument('-o', '--plot-dir', dest='out_dir', default="./plots", help='output directory')
    args = parser.parse_args()

    if args.stype not in signal_types:
        print('signal type must be in {}'.format(list(signal_types)))
        exit(-1)

    # recover paths
    for attr in ['root_file', 'out_dir']:
        setattr(args, attr, os.path.join(owd, getattr(args, attr)))

    os.makedirs(args.out_dir, exist_ok=True)

    # root macros
    gROOT.Macro('rootlogon.C')
    # declare root graph code
    func_code = open(os.path.join(script_dir, 'root', 'dataframe_analysis.C')).read()
    gInterpreter.Declare(func_code)

    # style
    my_style.cd()

    # root dataframe
    rdf = ROOT.RDataFrame('EvTree', args.root_file)

    for name, signal, nhits, x_label, x_hrange in signal_types[args.stype]:
        nh_label = 'N_{}'.format(name)
        sname = '{} {}'.format(args.stype.replace('_', ' '), name)

        c = ROOT.TCanvas('signal_sum', '', 1920, 1080)
        title = '{} Signal Sums'.format(sname)
        pads = prepare_canvas(c, [[nh] for nh in nhits], title, x_label, 'Counts')
        hists = []
        for i, nh in enumerate(nhits):
            pads[nh].cd()
            hist = rdf.Filter('{}.pos == {}'.format(signal, nh))\
                      .Histo1D(ROOT.RDF.TH1DModel('hsum{}'.format(nh), '{} = {}'.format(nh_label, nh), *x_hrange),
                              '{}.height'.format(signal))
            hist.SetLineColor(1)
            hist.SetFillColorAlpha(i + 1, 0.5)
            hist.Draw('hist & same')
            hists.append(hist)
        c.SaveAs(os.path.join(args.out_dir, '{}_sum_nhits.png'.format(sname.replace(' ', '_').lower())))

        c = ROOT.TCanvas('total_signal', '', 1920, 1080)
        minhit = 0
        title = '{} Signal Sum ({} > {})'.format(sname, nh_label, minhit)
        pads = prepare_canvas(c, [[0]], title, x_label, 'Normalized Counts')
        pads[0].cd()
        pads[0].SetLogy()
        hist = rdf.Filter('{}.pos > {}'.format(signal, minhit))\
                  .Histo1D(ROOT.RDF.TH1DModel('hsum', '', *x_hrange), '{}.height'.format(signal))
        hist.SetLineColor(1)
        hist.SetFillColorAlpha(38, 0.5)
        chist = hist.DrawNormalized('hist & same')
        leg = ROOT.TLegend(0.5, 0.8, 0.9, 0.9)
        leg.SetHeader('Total Entries {}'.format(rdf.Count().GetValue()), 'C')
        leg.Draw()
        c.SaveAs(os.path.join(args.out_dir, '{}_sum.png'.format(sname.replace(' ', '_').lower())))

