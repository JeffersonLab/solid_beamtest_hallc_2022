// a script for analyze the data with root dataframe

#include "Fadc250Data.h"


using namespace fdec;
using namespace ROOT::VecOps;

// for reducing the data size
Fadc250Data get_peaks(const Fadc250Data &data)
{
    Fadc250Data res;
    res.peaks = data.peaks;
    return res;
}

std::vector<double> unpack_time(const std::vector<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.time - ref); }
    return res;
}

std::vector<double> unpack_time(const RVec<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.time - ref); }
    return res;
}

std::vector<double> unpack_height(const std::vector<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.height - ref); }
    return res;
}

std::vector<double> unpack_height(const RVec<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.height - ref); }
    return res;
}

std::vector<double> unpack_integral(const std::vector<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.integral - ref); }
    return res;
}

std::vector<double> unpack_integral(const RVec<Peak> &peaks, bool cut_overflow = true, double ref = 0.)
{
    std::vector<double> res;
    for (auto &p : peaks) { if (!p.overflow || !cut_overflow) res.push_back(p.integral - ref); }
    return res;
}

// get peak in time window
bool time_cut(const Peak &p, double tmin, double tmax)
{
    return (p.time < tmax) && (p.time >= tmin);
}

bool time_cut_rel(const Peak &p, double tmin, double tmax, double ref)
{
    return ((p.time - ref) < tmax) && ((p.time - ref) >= tmin);
}

Peak get_maximum_peak(const std::vector<Peak> &peaks, bool cut_overflow = true)
{
    Peak res(0., 0., 0.);

    for (size_t i = 0; i < peaks.size(); ++i) {
        auto &p = peaks[i];
        if ((!p.overflow || !cut_overflow) && p.height > res.height) {
            res = p;
            res.pos = i;
        }
    }
    return res;
}

// get the maximum peak with a filter
template<class Filter, typename... Args>
Peak get_maximum_peak(const std::vector<Peak> &peaks, Filter &&f, Args&&... args)
{
    Peak res(0., 0., 0.);

    for (size_t i = 0; i < peaks.size(); ++i) {
        auto &p = peaks[i];
        if (!p.overflow && f(p, args...) && p.height > res.height) {
            res = p;
            res.pos = i;
        }
    }

    return res;
}

// get the maximum peak with a filter
template<class Filter, typename... Args>
Peak get_maximum_peak(const RVec<Peak> &peaks, Filter &&f, Args&&... args)
{
    Peak res(0., 0., 0.);

    for (size_t i = 0; i < peaks.size(); ++i) {
        auto &p = peaks[i];
        if (!p.overflow && f(p, args...) && p.height > res.height) {
            res = p;
            res.pos = i;
        }
    }

    return res;
}

Peak peak_sum(const std::vector<Peak> &peaks, double thres = 0.)
{
    Peak res(0., 0., 0., 0);

    for (auto &peak : peaks) {
        if (peak.height > thres) {
            res.time += peak.time;
            res.integral += peak.integral;
            res.height += peak.height;
            res.pos ++;
        }
    }
    res.time /= (double)res.pos;
    return res;
}

