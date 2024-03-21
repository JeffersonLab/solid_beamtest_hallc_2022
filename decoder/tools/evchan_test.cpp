/*  A program to test the consistency between Evchannel and Etchannel
 *  One should start this program first, connecting to an ET system (et_start with JLab ET library)
 *  And then use the et_feeder to feed the same evio file to the same ET system
 *  The intervals should be set large enough so that the two channels are synchronized
 */

#include "ConfigArgs.h"
#include "EtConfigWrapper.h"
#include "EvChannel.h"
#include "EtChannel.h"
#include <csignal>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

#define PROGRESS_COUNT 100

using namespace std::chrono;


volatile std::sig_atomic_t gSignalStatus;


void signal_handler(int signal) {
    gSignalStatus = signal;
}


int main(int argc, char* argv[])
{
    // setup input arguments
    ConfigArgs arg_parser;
    arg_parser.AddHelp("--help");
    arg_parser.AddPositional("evio_file", "input evio file");
    arg_parser.AddArg<std::string>("-h", "host", "host address of the ET system", "localhost");
    arg_parser.AddArg<int>("-p", "port", "port to connect ET system", 11111);
    arg_parser.AddArg<std::string>("-f", "et_file", "path to the memory mapped et file", "/tmp/et_feeder");
    arg_parser.AddArg<int>("-i", "interval", "interval in milliseconds to write data", 100);

    auto args = arg_parser.ParseArgs(argc, argv);

    // et channel reader
    evc::EtChannel et_chan;
    if (et_chan.Connect(args["host"].String(), args["port"].Int(), args["et_file"].String()) != evc::status::success ||
        et_chan.Open("MONITOR") != evc::status::success) {
        std::cerr << "Failed to open ET channel" << std::endl;
        return -1;
    }

    // evio file reader
    evc::EvChannel ev_chan;
    if (ev_chan.Open(args["evio_file"].String()) != evc::status::success) {
        std::cerr << "Failed to open coda file \"" << args["evio_file"] << "\"." << std::endl;
        return -1;
    }

    // install signal handler
    std::signal(SIGINT, signal_handler);
    int count = 0;
    bool loop = true;
    std::cout << "Listening to ET system, now you can feed the data to it." << std::endl;
    while (loop) {
        if (gSignalStatus == SIGINT) {
            std::cout << "Received control-C, exiting..." << std::endl;
            break;
        }
        switch (et_chan.Read()) {
        case evc::status::success:
            break;
        case evc::status::empty:
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        default:
            loop = false;
            continue;
        }
        ev_chan.Read();
        system_clock::time_point start(system_clock::now());
        system_clock::time_point next(start + std::chrono::milliseconds(args["interval"].Int()));

        if (++count % PROGRESS_COUNT == 0) {
            std::cout << "Received " << count << " events to ET.\r" << std::flush;
        }
        auto et_buf = et_chan.GetRawBufferVec();
        auto ev_buf = ev_chan.GetRawBuffer();
        std::cout << "New Event: " << et_buf.size() << ", " << et_buf[0] + 1 << ", " << ev_buf[0] + 1 << std::endl;
        std::cout << std::hex << std::setfill('0');
        for (size_t i = 0; (i < ev_buf[0] + 1) || (i < et_buf[0] + 1); ++i) {
            std::cout << "0x" << std::setw(8);
            if (i < et_buf.size()) { std::cout << et_buf[i]; }
            if (i < ev_buf[0] + 1) { std::cout << ", 0x" << std::setw(8) << ev_buf[i]; }
            std::cout << "\n";
        }
        std::cout << std::dec;
        std::this_thread::sleep_until(next);
    }
    std::cout << "Received " << count << " events to ET" << std::endl;

    et_chan.Close();
    et_chan.Disconnect();
    ev_chan.Close();
    return 0;
}

