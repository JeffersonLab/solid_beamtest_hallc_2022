#include "Fadc250Data.h"
#include "WfRootGraph.h"
#include "TSystem.h"
#include "TChain.h"
#include <algorithm>
#include <string>
#include <vector>
#include <random>


// some pre-built events
static const std::vector<std::vector<uint32_t>> _test_events = {
    {
        129, 132, 131, 128, 128, 127, 130, 130, 126, 126, 128, 126, 126,
        // 127, 128, 125, 130, 130, 131, 143, 146, 149, 143, 142, 139, 136,
        127, 128, 125, 123, 120, 121, 113, 109, 102, 109, 110, 119, 126,
        134, 135, 132, 132, 132, 128, 139, 167, 188, 189, 180, 171, 164,
        156, 150, 145, 143, 140, 136, 137, 136, 132, 131, 132, 132, 132,
        130, 129, 131, 130, 131, 131, 128, 130, 129, 127, 128, 127
    },
    {
        567, 484, 414, 359, 320, 291, 271, 275, 279, 278, 271, 271, 277,
        277, 265, 252, 237, 228, 213, 207, 201, 195, 188, 183, 179, 176,
        173, 191, 217, 231, 298, 392, 436, 423, 383, 341, 302, 265, 237,
        215, 198, 181, 171, 160, 151, 152, 144, 137, 136, 134, 133, 130,
        125, 125, 124, 123, 120, 122, 117, 117, 116, 115, 116, 113
    },
    {
        131, 133, 131, 134, 131, 132, 131, 132, 132, 132, 131, 129, 131,
        132, 130, 131, 129, 130, 131, 131, 131, 133, 133, 137, 147, 152,
        151, 147, 146, 139, 141, 138, 135, 135, 132, 134, 134, 136, 134,
        131, 134, 133, 130, 130, 132, 129, 128, 132, 132, 130, 130, 133,
        132, 131, 131, 131, 132, 132, 130, 129, 127, 129, 131, 132
    },
    {
        162, 158, 157, 159, 159, 160, 159, 161, 159, 160, 158, 159, 183,
        236, 263, 271, 267, 254, 240, 228, 213, 205, 192, 186, 181, 177,
        // 236, 1263, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 2271, 1177,
        174, 170, 165, 170, 165, 165, 164, 162, 161, 163, 163, 162, 159,
        159, 161, 159, 161, 160, 158, 157, 159, 158, 156, 160, 159, 158,
        160, 158, 158, 156, 158, 157, 159, 157, 159, 158, 156, 157
    },
    {
        152, 150, 153, 151, 151, 154, 152, 151, 151, 151, 150, 154, 151,
        154, 151, 155, 152, 152, 150, 152, 153, 153, 151, 150, 153, 151,
        151, 150, 151, 153, 192, 244, 267, 266, 254, 238, 220, 206, 198,
        188, 181, 174, 171, 167, 164, 161, 160, 160, 156, 157, 156, 156,
        155, 155, 156, 154, 152, 153, 153, 190, 246, 269, 263, 250
    },
};

// channel look-up table
static const std::vector<std::string> _channels = {
    "Cer11_5", "Cer12_5", "Cer13_5", "Cer14_5",
    "Cer21_5", "Cer22_5", "Cer23_5", "Cer24_5",
    "Cer31_5", "Cer32_5", "Cer33_5", "Cer34_5",
    "Cer41_5", "Cer42_5", "Cer43_5", "Cer44_5",
};

struct TestEvent {
    std::string name;
    fdec::Fadc250Data data;
};

// get events from root file
// if empty <path> provided, the pre-built events will be returned
// if empty <indices> provided, <nev> events will be randomly selected
std::vector<TestEvent> get_events(const std::string &path, std::vector<int> indices = {}, int nev = 5)
{
    std::vector<TestEvent> res;
    if (path.empty()) {
        for (size_t i = 0; i < _test_events.size(); ++i) {
            fdec::Fadc250Data data;
            data.raw = _test_events[i];
            res.emplace_back(TestEvent{"Pre-built Event " + std::to_string(i + 1), data});
        }
        return res;
    }

    // root tree
    auto t = new TChain("EvTree");
    t->Add(path.c_str());
    int maxn = t->GetEntries();

    // fetch tree data
    std::vector<fdec::Fadc250Data*> events(_channels.size());
    for (size_t i = 0; i < _channels.size(); ++i) {
        auto &ch = _channels[i];
        t->SetBranchAddress(_channels[i].c_str(), &events[i]);
    }

    // random samples if no event indices given
    if (indices.empty()) {
        if (maxn > nev) {
            std::random_device rd;
            std::mt19937 rng(rd());
            std::uniform_int_distribution<int> udist(0, maxn - 1);
            while (static_cast<int>(indices.size()) < nev) {
                int idx = udist(rng);
                if(std::find(indices.begin(), indices.end(), idx) == indices.end()) {
                    indices.push_back(idx);
                }
            }
        } else {
            for (int i = 0; i < maxn; ++i) { indices.push_back(i); }
        }
    }

    // get event samples
    for (int idx : indices) {
        if (idx >= maxn) { continue; }
        t->GetEntry(idx);

        for (size_t i = 0; i < _channels.size(); ++i) {
            if (events[i]->raw.empty()) { continue; }
            auto &ch = _channels[i];
            res.emplace_back(TestEvent{"Event " + std::to_string(idx) + ", " + ch, *events[i]});
        }
    }
    return res;
}


void test_wfanalyzer(const std::string &path = "", const std::string &dir = "./plots",
                     const std::vector<int> &indices = {}, int nev = 5,
                     size_t res = 3, double thres = 10.0)
{
    gSystem->Exec(("mkdir -p " + dir).c_str());
    gStyle->SetOptStat(0);
    fdec::Analyzer ana(res, thres);

    auto events = get_events(path, indices, nev);

    auto c = new TCanvas("test", "test", 1920, 1080);
    c->DivideSquare(events.size() + 1);
    auto legend = new TLegend(0.1, 0.9, 0.9, 0.1);

    int count = 0;
    for (auto &event : events) {
        auto graph = get_waveform_graph(ana, event.data.raw);
        graph.mg->SetTitle((event.name + "; Sample Number; ADC Values").c_str());
        graph.mg->GetXaxis()->SetRangeUser(0, event.data.raw.size() - 1);
        count ++;
        c->cd(count);
        graph.mg->Draw("a");

        // only add entries once
        if (count > 1) { continue; }
        for (auto &entry : graph.entries) {
            legend->AddEntry(entry.obj, entry.label.c_str(), entry.option.c_str());
        }
    }
    c->cd(events.size() + 1);
    legend->Draw();
    c->SaveAs((dir + "/wfa_test.png").c_str());
}

