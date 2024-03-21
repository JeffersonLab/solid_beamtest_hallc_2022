import ROOT
import os
import json
import numpy as np
import pandas as pd
import argparse
from matplotlib import pyplot as plt
from ROOT import gROOT, gSystem, gInterpreter, gStyle, gPad
from plot_utils import prepare_canvas, prepare_figure, my_style, get_hist_contents


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
parser.add_argument('-o', '--plot-dir', dest='out_dir', default='./plots', help='output directory')
parser.add_argument('--layout', dest='layout', default=os.path.join(script_dir, '..', 'database/channels_layout.json'),
                    help='json data for channels layout')
args = parser.parse_args()

# recover paths
for attr in ['root_file', 'out_dir', 'layout']:
    setattr(args, attr, os.path.join(owd, getattr(args, attr)))

os.makedirs(args.out_dir, exist_ok=True)

# root dataframe
rdf = ROOT.RDataFrame('EvTree', args.root_file)

# read json files
with open(args.layout, 'r') as f:
    channels_layout = json.load(f)

channels = channels_layout['LAPPD']

c1 = ROOT.TCanvas('c1', '', 2560, 1440)
p1 = prepare_canvas(c1, channels, 'LAPPD Peak Height Distribution', 'ADC Value', 'Counts')


def get_neighbors(channels, i, j):
    neighbors = []
    ni = [i - 1, i + 1]
    nj = [j - 1, j + 1]
    for ii in ni:
        for jj in nj:
            if ii >= 0 and ii < 8 and jj >= 0 and jj < 8:
                neighbors.append(channels[0][ii*8 + jj])
    return neighbors

hists = []
hrange = (100, 0, 100)
ymax = 0
for i in np.arange(8, dtype=int):
    for j in np.arange(8, dtype=int):
        ch = channels[0][i*8 +j]
        p1[ch].cd()
        neighbors = get_neighbors(channels, i, j)
        filter_code = '||'.join(['{}_peak.height > 0'.format(n) for n in neighbors])
        hists.append(
            rdf.Filter('{}_peak.height > 0'.format(ch))\
               .Filter(filter_code)
               .Histo1D(ROOT.RDF.TH1DModel('h{}'.format(ch), '', *hrange), '{}_peak.height'.format(ch))
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
for j in np.arange(64, dtype=int):
    hists[j].GetYaxis().SetRangeUser(0, ymax*1.1)

c1.SaveAs(os.path.join(args.out_dir, 'lappd_neighbor_cuts.png'))

