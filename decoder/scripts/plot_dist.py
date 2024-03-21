import ROOT
import os
import json
import numpy as np
import pandas as pd
import argparse
from matplotlib import pyplot as plt
from ROOT import gROOT, gSystem, gInterpreter, gStyle, gPad
from plot_utils import prepare_canvas, prepare_figure, my_style, get_hist_contents, nthreads

ROOT.ROOT.EnableImplicitMT(nthreads)

# make sure the relative path for root loading is correct
owd = os.getcwd()
script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(os.path.join(script_dir, '..'))
print('working directory is ' + os.getcwd())

# batch mode
gROOT.SetBatch(True)
# rootlogon macro
gROOT.Macro('rootlogon.C')

# the functions that process data are defined in scripts/root/dataframe_analysis.C
func_code = open(os.path.join(script_dir, 'root', 'dataframe_analysis.C')).read()
gInterpreter.Declare(func_code)

# global style
my_style.cd()
bprops = dict(boxstyle='round', facecolor='wheat', alpha=0.2)

# argument parser
parser = argparse.ArgumentParser('plot distributions for raw data')
parser.add_argument('root_file', help='root file output from analyzer')
parser.add_argument('json_command', help='json file of plot commands')
parser.add_argument('-o', '--plot-dir', dest='out_dir', default='./plots', help='output directory')
parser.add_argument('--root-plots', dest='root_plots', action='store_true',
                    help='use root instead of matplotlib to plot')
parser.add_argument('--layout', dest='layout', default=os.path.join(script_dir, '..', 'database/channels_layout.json'),
                    help='json data for channels layout')
args = parser.parse_args()

# recover paths
for attr in ['root_file', 'json_command', 'out_dir', 'layout']:
    setattr(args, attr, os.path.join(owd, getattr(args, attr)))

os.makedirs(args.out_dir, exist_ok=True)

# root dataframe
rdf = ROOT.RDataFrame('EvTree', args.root_file)

# read json files
with open(args.json_command) as f:
    plot_data = json.load(f)

with open(args.layout, 'r') as f:
    channels_layout = json.load(f)


# get histogram from dataframe
def get_hist(root_df, fcode, bins, xmin, xmax, filcode):
    df_temp = root_df.Define('my_temp', fcode)
    if filcode:
        df_temp = df_temp.Filter(filcode)
    return df_temp.Histo1D(ROOT.RDF.TH1DModel('my_temp', '', bins, xmin, xmax), 'my_temp')


# plot with root canvas
def root_plots(df, channels, func_code, title, xlabel, ylabel, hist_range, path, filter_code='', refs={}):
    c = ROOT.TCanvas(title, '', 2560, 1440)
    pads = prepare_canvas(c, channels, title, xlabel, ylabel)
    hists = []
    for i, chans in enumerate(channels):
        ymax = 0
        for j, ch in enumerate(chans):
            kw = refs.get(ch, {})
            pads[ch].cd()
            hists.append(
                get_hist(df, func_code.format(ch=ch, **kw), *hist_range, filter_code.format(ch=ch, **kw))
            )
            hists[-1].SetTitle('{}'.format(ch))
            hists[-1].SetLineStyle(2)
            hists[-1].SetLineWidth(1)
            hists[-1].SetLineColor(46)
            hists[-1].SetFillColorAlpha(38, 0.7)
            hists[-1].SetFillStyle(3000)
            hists[-1].Draw('LF')
            new_ymax = hists[-1].GetMaximum()
            ymax = new_ymax if new_ymax > ymax else ymax
        # shared y-axis among sub-groups
        for j in np.arange(0, len(chans)):
            hists[-1 - j].GetYaxis().SetRangeUser(0, ymax*1.1)
    c.Update()
    c.SaveAs(path)


# plot with matplotlib
def mat_plots(df, channels, func_code, title, xlabel, ylabel, hist_range, path, filter_code='', refs={}):
    fig = plt.figure(figsize=(16, 9), dpi=160)
    axs = prepare_figure(fig, channels, title=title, xlabel=xlabel, ylabel=ylabel)

    for i, chans in enumerate(channels):
        for j, ch in enumerate(chans):
            kw = refs.get(ch, {})
            ax = axs[ch]
            x, y = get_hist_contents(
                get_hist(df, func_code.format(ch=ch, **kw), *hist_range, filter_code.format(ch=ch, **kw))
            )
            ax.bar(x, y, width=np.min(np.diff(x)))
            ax.text(0.95, 0.95, ch, transform=ax.transAxes, fontsize=14, bbox=bprops,
                    verticalalignment='top', horizontalalignment='right')

    fig.savefig(path)


# generate plots
for group, data in plot_data.items():
    # get default layout if no channels provided
    channels = data.get('channels', channels_layout[group])
    refs = data.get('refs', {})
    # path
    if isinstance(refs, str):
        cmd_dir = os.path.dirname(os.path.realpath(args.json_command))
        with open(os.path.join(cmd_dir, refs), 'r') as f:
            refs = json.load(f)

    for plot_kw in data['plots']:
        # update a few keywords
        plot_kw['path'] = os.path.join(args.out_dir, plot_kw['path'])
        if 'refs' not in plot_kw:
            plot_kw['refs'] = refs
        # plot
        if args.root_plots:
            root_plots(rdf, channels, **plot_kw)
        else:
            mat_plots(rdf, channels, **plot_kw)
        print('Saved plot to {}'.format(plot_kw['path']))

