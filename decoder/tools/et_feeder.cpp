/*  A program to feed evio file to an ET system event-by-event
 *  ET system can be opened with et_start with the JLab ET library and its executables
 */
#include "ConfigArgs.h"
#include "EtConfigWrapper.h"
#include "EvChannel.h"
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

    et_sys_id et_id;
    et_att_id att_id;

    // open ET system
    et_wrap::OpenConfig conf;
    conf.set_cast(ET_DIRECT);
    conf.set_host(args["host"].c_str());
    conf.set_serverport(args["port"].Int());

    char *fname = strdup(args["et_file"].c_str());
    auto status = et_open(&et_id, fname, conf.configure().get());
    free(fname);

    if (status != ET_OK) {
        std::cerr << "Cannot open ET at " << args["host"] << ":" << args["port"] << " with "
                  << args["et_file"] << std::endl;
        return -1;
    }

    // attach to GRAND CENTRAL
    status = et_station_attach(et_id, ET_GRANDCENTRAL, &att_id);
    if (status != ET_OK) {
        std::cerr << "Failed to attach to the ET Grand Central Station." << std::endl;
        return -1;
    }

    // evio file reader
    evc::EvChannel chan;
    if (chan.Open(args["evio_file"].String()) != evc::status::success) {
        std::cerr << "Failed to open coda file \"" << args["evio_file"] << "\"." << std::endl;
        return -1;
    }

    // install signal handler
    std::signal(SIGINT, signal_handler);
    int count = 0;
    et_event *ev;
    while ((chan.Read() == evc::status::success) && et_alive(et_id)) {
        if (gSignalStatus == SIGINT) {
            std::cout << "Received control-C, exiting..." << std::endl;
            break;
        }
        system_clock::time_point start(system_clock::now());
        system_clock::time_point next(start + std::chrono::milliseconds(args["interval"].Int()));

        if (++count % PROGRESS_COUNT == 0) {
            std::cout << "Read and fed " << count << " events to ET.\r" << std::flush;
        }

        uint32_t *buf = chan.GetRawBuffer();
        size_t nbytes = (buf[0] + 1)*sizeof(uint32_t);

        /*
        std::cout << std::hex << std::setfill('0');
        for (size_t i = 0; i < buf[0] + 1; ++i) {
            std::cout << "0x" << std::setw(8) << buf[i] << "\n";
        }
        std::cout << std::dec;
        */

        status = et_event_new(et_id, att_id, &ev, ET_SLEEP, nullptr, nbytes);
        if (status != ET_OK) {
            std::cerr << "Failed to add new event to the ET system." << std::endl;
            return -1;
        }
        // build et event
        void *data;
        et_event_getdata(ev, &data);
        memcpy((void *) data, (const void *)buf, nbytes);
        et_event_setlength(ev, nbytes);

        // put back the event
        status = et_event_put(et_id, att_id, ev);
        if (status != ET_OK) {
            std::cerr << "Failed to put event back to the ET system." << std::endl;
            return -1;
        }

        std::this_thread::sleep_until(next);
    }
    std::cout << "Read and fed " << count << " events to ET" << std::endl;

    chan.Close();
    return 0;

}

