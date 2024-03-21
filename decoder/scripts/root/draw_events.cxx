#include "Fadc250Data.h"
#include "WfRootGraph.h"
#include "TChain.h"
#include "TSystem.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>


#define PROGRESS_COUNT 10

struct TestChannel {
    std::string name;
    fdec::Fadc250Data data;
};

struct TestEvent {
    std::string name;
    std::vector<std::vector<TestChannel>> channels;

    size_t Nsamples() const
    {
        size_t nsamples = 0;
        for (auto &chs : channels) {
            for (auto &ch : chs) {
                nsamples += ch.data.raw.size();
            }
        }
        return nsamples;
    }
};

// get events from root file
std::vector<TestEvent> get_events(const std::string &path, const std::vector<std::vector<std::string>> &channels,
                                  std::vector<int> indices = {}, int nev = 5)
{
    std::vector<TestEvent> res;
    // root tree
    auto t = new TChain("EvTree");
    t->Add(path.c_str());
    int maxn = t->GetEntries();

    // fetch tree data
    std::vector<std::vector<fdec::Fadc250Data*>> data(channels.size());
    for (size_t i = 0; i < channels.size(); ++i) {
        data[i].resize(channels[i].size());
        for (size_t j = 0; j < channels[i].size(); ++j) {
            auto &ch = channels[i][j];
            t->SetBranchAddress(ch.c_str(), &data[i][j]);
        }
    }

    auto get_event = [t, maxn, &data, &channels] (int idx) {
        TestEvent event{"event_" + std::to_string(idx), std::vector<std::vector<TestChannel>>(channels.size())};
        if (idx >= maxn) { return event; }
        t->GetEntry(idx);
        for (size_t i = 0; i < channels.size(); ++i) {
            event.channels[i].resize(channels[i].size());
            for (size_t j = 0; j < channels[i].size(); ++j) {
                auto &ch = channels[i][j];
                event.channels[i][j].name = ch;
                event.channels[i][j].data = *data[i][j];
            }
        }
        return event;
    };

    // if event indices provided
    if (indices.size()) {
        for (int idx : indices) {
            res.push_back(get_event(idx));
        }
        return res;
    }

    // randomly pick events if no indices provided
    nev = std::min(maxn, nev);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> udist(0, maxn - 1);
    int tries = 0;
    while (static_cast<int>(indices.size()) < maxn && static_cast<int>(res.size()) < nev) {
        tries ++;
        int idx = udist(rng);
        if(std::find(indices.begin(), indices.end(), idx) == indices.end()) {
            indices.push_back(idx);
            auto event = get_event(idx);
            // only save if this event is not an empty one
            if (event.Nsamples()) {
                res.emplace_back(std::move(event));
            }
        }
        if((tries % PROGRESS_COUNT) == 0) {
            std::cout << "Tried " << tries << " events, "
                      << res.size() << " non-empty (for the given channels) events found..."
                      << "\r" << std::flush;
        }
    }
    std::cout << "Tried " << tries << " events, "
              << res.size() << " non-empty (for the given channels) events found..."
              << std::endl;
    return res;
}


void draw_events(const std::string &path = "", const std::string &dir = "./plots",
                 const std::vector<std::vector<std::string>> &channels = {{"Cer11_1", "Cer11_2", "Cer11_3", "Cer11_4"}},
                 const std::vector<int> &indices = {}, int nev = 5,
                 size_t res = 3, double thres = 10.0, size_t npeds = 5, double ped_flatness = 1.0,
                 const std::string &prefix = "")
{
    gSystem->Exec(("mkdir -p " + dir).c_str());
    gStyle->SetOptStat(0);
    fdec::Analyzer ana(res, thres, npeds, ped_flatness);

    auto events = get_events(path, channels, indices, nev);

    for (auto &event : events) {
        auto c = new TCanvas(event.name.c_str(), event.name.c_str(), 1920, 1080);
        c->DivideSquare(event.channels.size());
        for (size_t i = 0; i < event.channels.size(); ++i) {
            auto &chs = event.channels[i];
            c->cd(i + 1);
            int ndiv = (chs.size() > 1) ? int(std::sqrt(chs.size()) - 1e-3) + 1 : 1;
            gPad->Divide(ndiv, ndiv, 0., 0.);
            for (size_t j = 0; j < chs.size(); ++j) {
                auto &ch = chs[j];
                auto graph = get_waveform_graph(ana, ch.data.raw);
                c->cd(i + 1)->cd(j + 1);
                graph.mg->SetTitle((ch.name + "; Sample Number; ADC Values").c_str());
                graph.mg->GetXaxis()->SetRangeUser(0, ch.data.raw.size() - 1);
                graph.mg->Draw("A");
            }

        }
        c->SaveAs((dir + "/" + prefix + event.name + ".png").c_str());
    }
}

