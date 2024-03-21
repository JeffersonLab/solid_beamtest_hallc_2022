import os
import numpy as np
from collections import OrderedDict
from ROOT import TCanvas, TText, TStyle, kBird, kWhite
from matplotlib import pyplot as plt
import matplotlib.gridspec as gridspec


nthreads = int(os.cpu_count()/2)

# style
my_style = TStyle("Default", "My Root Style")
my_style.SetOptStat(0)
my_style.SetFrameLineWidth(2)
my_style.SetTitleSize(0.1, 'p')
my_style.SetTitleW(0.35)
my_style.SetTitleX(0.6)
my_style.SetTitleAlign(13)
my_style.SetTitleColor(2, 'xyz')
my_style.SetTitleSize(0.08, 'xyz')
my_style.SetTitleOffset(0.5, 'xyz')
my_style.SetTitleStyle(0)
my_style.SetTitleBorderSize(0)
my_style.SetLabelSize(0.06, 'xyz')
my_style.SetPalette(kBird)
my_style.SetFrameFillColor(kWhite)
my_style.SetPadGridX(0)
my_style.SetPadGridY(0)
my_style.SetPadBorderMode(0)
my_style.SetCanvasColor(0)
my_style.SetPadColor(0)


def divide_square(n):
    test = int(np.sqrt(n))
    if np.square(test) >= n:
        return test, test
    elif (test + 1)*test >= n:
        return test + 1, test
    else:
        return test + 1, test + 1


def prepare_canvas(c, channels, pad_label=None, x_label=None, y_label=None):
    pads = dict()
    # for axis labels
    c.Divide(1, 1, 0.03, 0.03)
    c.cd(1).Divide(*divide_square(len(channels)))
    for i, chans in enumerate(channels):
        ncol, nrow = divide_square(len(chans))
        c.cd(1).cd(i + 1).Divide(nrow, ncol, 0., 0.)
        # share y axis in the sub-group
        ymax = 0
        for j, ch in enumerate(chans):
            pads[ch] = c.cd(1).cd(i + 1).cd(j + 1)
    # labels
    c.cd(0)
    htext, vtext = TText(), TText()
    htext.SetTextSize(0.03)
    htext.SetTextAlign(22)
    vtext.SetTextSize(0.03)
    vtext.SetTextAlign(22)
    vtext.SetTextAngle(90)

    if pad_label:
        htext.DrawText(0.5, 0.982, pad_label)
    if x_label:
        htext.DrawText(0.5, 0.015, x_label)
    if y_label:
        vtext.DrawText(0.015, 0.5, y_label)
    return pads


def prepare_figure(fig, channels, title=None, xlabel=None, ylabel=None):
    axs = dict()
    ncol1, nrow1 = divide_square(len(channels))
    gs1 = gridspec.GridSpec(nrow1, ncol1, figure=fig, left=0.07, right=0.95, bottom=0.07, top=0.95)

    for i, chans in enumerate(channels):
        gs = gs1[i // ncol1, i % ncol1]
        ncol2, nrow2 = divide_square(len(chans))
        gs2 = gs.subgridspec(nrow2, ncol2, wspace=0., hspace=0.)

        # ax0 for label sharing
        ax0 = None
        for j, ch in enumerate(chans):
            if ax0:
                ax = fig.add_subplot(gs2[j // ncol2, j % ncol2], sharey=ax0, sharex=ax0)
            else:
                ax = fig.add_subplot(gs2[j // ncol2, j % ncol2])
                ax0 = ax
            ax.tick_params(labelsize=12, axis='both', direction='in')
            axs[ch] = ax

    # only show the outer tick labels
    for ax in fig.axes:
        try:
            ax.label_outer()
        except:
            pass

    if title:
        fig.text(0.5, 0.97, title, ha='center', fontsize=18)
    if xlabel:
        fig.text(0.5, 0.02, xlabel, ha='center', fontsize=16)
    if ylabel:
        fig.text(0.02, 0.5, ylabel, va='center', rotation=90, fontsize=16)
    return axs


def get_hist_contents(root_hist):
    xfunc = np.vectorize(root_hist.GetBinCenter, otypes=[float])
    yfunc = np.vectorize(root_hist.GetBinContent, otypes=[float])
    bins = root_hist.GetNbinsX()
    return xfunc(np.arange(1, bins + 1, dtype=int)), yfunc(np.arange(1, bins + 1, dtype=int))


