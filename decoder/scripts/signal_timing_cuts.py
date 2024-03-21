import ROOT
import os
import numpy as np
import pandas as pd
import argparse
import json
from matplotlib import pyplot as plt
from ROOT import gROOT, gSystem, gInterpreter, gStyle, gPad, kRed, kAzure
from plot_utils import prepare_canvas, prepare_figure, my_style, get_hist_contents, nthreads


ROOT.ROOT.EnableImplicitMT(nthreads)

# make sure the relative path for root scripts is correct
owd = os.getcwd()
script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(os.path.join(script_dir, '..'))
print('working directory is ' + os.getcwd())

# rootlogon macro
gROOT.Macro('rootlogon.C')

# the functions that process data are defined in scripts/root/dataframe_analysis.C
func_code = open(os.path.join(script_dir, 'root', 'dataframe_analysis.C')).read()
gInterpreter.Declare(func_code)

# global style
my_style.cd()
bprops = dict(boxstyle='round', facecolor='wheat', alpha=0.2)

# signal layouts
layout_json = os.path.join(script_dir, '..', 'database', 'channels_layout.json')
with open(layout_json, 'r') as f:
    channels_layout = json.load(f)
get_chs = lambda lyt : [c for chs in lyt for c in chs]

# default tcuts peak search range
lappd_layout = channels_layout['LAPPD']
lappd_tcuts = {c: {'cut': (-100, -50), 'ref': 'ecal.time'} for c in get_chs(lappd_layout)}

mapmts_layout = channels_layout['MaPMT Sums']
mapmts_tcuts = {c: {'cut': (-100, -50), 'ref': 'ecal.time'} for c in get_chs(mapmts_layout)}
mapmts_tcuts['Cer43_5']['cut'] = (-150, -100)

mapmtq_layout = channels_layout['MaPMT Quads']
mapmtq_tcuts = {c: {'cut': (-80, 20), 'ref': c.split('_')[0] + '_5_peak.time', 'group': c.split('_')[0]}
                for c in get_chs(mapmtq_layout)}

# {type: [(name, layout, tcuts_search_range, tcuts_width, plots), ...]}
# plots: [(attribute, title, xlabel, xhist_range, take_reference), ...]
signal_types = {
    'LAPPD': [
        ('LAPPD', lappd_layout, lappd_tcuts, 10, [
            ('time', 'LAPPD Time (tcut)', 'Time Rel. to ECal. (ns)', (80, -160, 160), True),
            ('height', 'LAPPD Peak Height (tcut)', 'Peak Height (ADC Channel)', (100, 0, 100), False),
        ]),
    ],
    'MAPMT': [
        ('MaPMT Sums', mapmts_layout, mapmts_tcuts, 8, [
            ('time', 'MaPMT Sums Time (tcut)', 'Time Rel. to ECal. (ns)', (80, -160, 160), True),
            ('height', 'MaPMT Sums Peak Height (tcut)', 'Peak Height (ADC Channel)', (100, 0, 1000), False),
        ]),
        ('MaPMT Quads', mapmtq_layout, mapmtq_tcuts, 5, [
            ('time', 'MaPMT Quads Time (tcut)', 'Time Rel. to Sum (ns)', (80, -160, 160), True),
            ('height', 'MaPMT Quads Peak Height (tcut)', 'Peak Height (ADC Channel)', (100, 0, 1000), False),
        ]),
    ],
}

# argument parser
parser = argparse.ArgumentParser('timing cuts study')
parser.add_argument('stype', help='signal type, supporting: {}'.format(list(signal_types)))
parser.add_argument('root_file', help='root file output from analyzer')
parser.add_argument('json_file', help='output timing cut in json')
parser.add_argument('-b', dest='batch', action='store_true', help='set to batch mode')
parser.add_argument('--read-cuts', dest='read', action='store_true', help='directly use cuts defined in json file.')
parser.add_argument('--snapshot', dest='snap', default='', type=str, help='path to save the data with trigger cuts')
parser.add_argument('--plot-dir', dest='out_dir', default='.', type=str, help='directory to save the plot')
parser.add_argument('--cut-height', dest='hcut', default=0, type=float, help='cut on the minimum peak height')
args = parser.parse_args()

if args.stype not in signal_types:
    print('signal type must be in {}'.format(list(signal_types)))
    exit(-1)

# batch mode
gROOT.SetBatch(args.batch)

# recover paths
for attr in ['root_file', 'json_file', 'snap', 'out_dir']:
    setattr(args, attr, os.path.join(owd, getattr(args, attr)))

os.makedirs(args.out_dir, exist_ok=True)

# root dataframe
rdf = ROOT.RDataFrame('EvTree', args.root_file)

# read json
json_data = dict()
if os.path.exists(args.json_file):
    with open(args.json_file, 'r') as f:
        json_data.update(json.load(f))


# main function
def timing_cuts_search(df, cuts_dict, cut_width, hbins=100):
    for ch, cuts in cuts_dict.items():
        search = cuts['cut']
        ref = cuts['ref']
        hist = df.Define('peak_time', 'unpack_time({}_p.peaks, true, {})'.format(ch, ref))\
                 .Histo1D(ROOT.RDF.TH1DModel('time_hist1', '', hbins, *search), 'peak_time')
        pos = hist.GetBinCenter(hist.GetMaximumBin())
        height = hist.GetMaximum()
        if height > 0:
            cuts_dict[ch]['cut'] = (pos - cut_width, pos + cut_width)
    return cuts_dict


# plot distributions with cuts
def plot_dist_tcuts(df, canvas, pads_dict, cuts_dict, attr, hrange=(128, -256, 256), use_ref=False, hthres=0):
    stacks, hists = [], []
    for ch, pad in pads_dict.items():
        cut = cuts_dict[ch]['cut']
        ref = cuts_dict[ch]['ref'] if use_ref else '0'
        stacks.append(ROOT.THStack('hs_' + ch, ch))
        attr_name = '{}_{}'.format(ch, attr)
        hists.append(
            df.Define(attr_name, 'unpack_{}({}_p.peaks, true, {})'.format(attr, ch, ref))\
              .Histo1D(ROOT.RDF.TH1DModel(attr_name + '_h', '', *hrange), attr_name)
        )
        hists[-1].SetFillColorAlpha(38, 0.7)
        hists[-1].SetFillStyle(3000)
        hists[-1].SetLineStyle(2)
        hists[-1].SetLineColor(kAzure - 6)
        stacks[-1].Add(hists[-1].GetPtr())

        # cut peaks
        hists.append(
            df.Filter('{}_peak.height > {}'.format(ch, hthres))\
              .Define(attr_name + '_cut', '{}_peak.{} - {}'.format(ch, attr, ref))\
              .Histo1D(ROOT.RDF.TH1DModel(attr_name + '_hcut', '', *hrange), attr_name + '_cut')
        )
        hists[-1].SetFillColorAlpha(46, 0.7)
        hists[-1].SetFillStyle(3000)
        hists[-1].SetLineStyle(2)
        hists[-1].SetLineColor(kRed + 3)
        stacks[-1].Add(hists[-1].GetPtr())

        pad.cd()
        stacks[-1].Draw('nostack')
        canvas.Update()
    return stacks, hists



# =============================================================================
# Signal timing cuts
# =============================================================================
save_list = ['ecal', 'ecal_ch']
sum_cols = []
for name, layout, tcut_search_range, tcut_width, plots in signal_types[args.stype]:
    print('Study timing cuts for {} channels...'.format(name))

    # tcuts
    if args.read:
        tcuts = {c: json_data[c] for c in get_chs(layout)}
    else:
        tcuts = timing_cuts_search(rdf, tcut_search_range, tcut_width)
        json_data.update(tcuts)

    # apply cuts
    new_cols = []
    for ch, cut in tcuts.items():
        col = '{}_peak'.format(ch)
        rdf = rdf.Define(col, 'get_maximum_peak({}_p.peaks, time_cut_rel, {}, {}, {})'\
                              .format(ch, *cut['cut'], cut.get('ref', '0')))
        new_cols.append(col)
    save_list += new_cols

    # signal sums
    vec_str = '{' + ','.join(new_cols) + '}'
    sum_col = '{}_all'.format(name.lower().replace(' ', '_'))
    rdf = rdf.Define(sum_col, 'peak_sum({}, {})'.format(vec_str, args.hcut))
    sum_cols.append(sum_col)

    # plot distributions with time cuts
    for pattr, title, xlbl, xhrange, ref in plots:
        c = ROOT.TCanvas(pattr, '', 1920, 1080)
        pads = prepare_canvas(c, layout, pad_label=title, x_label=xlbl, y_label='Counts')
        res = plot_dist_tcuts(rdf, c, pads, tcuts, pattr, xhrange, hthres=args.hcut, use_ref=ref)
        c.SaveAs(os.path.join(args.out_dir, 'tcuts_{}_{}.png'.format(name.lower().replace(' ', '_'), pattr)))

# filter zero sums
fil_codes = ['{}.height > {}'.format(s, args.hcut) for s in sum_cols]
rdf = rdf.Filter('||'.join(fil_codes), 'Signal Sum Cut')
save_list += sum_cols


# =============================================================================
# Ending
# =============================================================================
rdf.Report().Print()
# save json
if not args.read:
    with open(args.json_file, 'w') as f:
        json.dump(json_data, f, indent=2)

# save data
if args.snap:
    print('Saving data snapshot to \"{}\", with columns:\n{}'.format(args.snap, save_list))
    rdf.Snapshot('EvTree', args.snap, save_list)

# this keeps the canvas
if not args.batch:
    print("press enter to quit")
    input()

