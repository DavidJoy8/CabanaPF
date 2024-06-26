#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <CabanaPF_Runner.hpp>
#include <getopt.h>

namespace CabanaPF {

struct CommandLineInput {
    inline static const std::string USAGE =
        " --grid <grid points> --dt <timestep size> --endtime <end time> [--majoroutputs <count>] [--minoroutputs "
        "<count>] [--startoutput <time>] [--endoutput <time>] [--log] [--outputatzero]";
    inline static const std::string HELP =
        "--grid <grid points>\n\tNumber of grid points in each dimension\n"
        "--dt <timestep size>\n\tSize of each timestep\n"
        "--endtime <end time>\n\tLength of simulation. Defaults to endoutput, but at least one must be specified\n"
        "[--majoroutputs <count>]\n\tOptional number of (Runner-defined) significant outputs (e.g. file write) to "
        "perform\n"
        "[--minoroutputs <count>]\n\tOptional number of (Runner-defined) lesser outputs (e.g. free energy calculation) "
        "to perform\n"
        "[--startoutput <time>]\n\tSimulation time to start outputting. Inclusive, so an output will happen at this "
        "time. Defaults to 0\n"
        "[--endoutput <time>]\n\tSimulation time to stop outputting. Inclusive, so an output will happen at this time. "
        "Defaults to endtime\n"
        "[--log]\n\tFlag to space outputs logarithmically. If not specified, spacing is linear\n"
        "[--outputatzero]\n\tFlag to include an output at t=0, independent of the other options\n";

    int grid_points = 0;
    double dt = 0;
    double end_time = 0;
    double start_output = 0;
    double end_output = 0;
    int major_outputs = 0;
    int minor_outputs = 0;
    bool log_scale = false;
    bool output_at_zero = false;

    // add outputs to the runner
    template <class Runner>
    void add_outputs_to_runner(Runner& runner) {
        if (output_at_zero && start_output > 0) {
            runner.add_output(0, true);
            runner.add_output(0, false);
        }

        double low = log_scale ? std::log10(start_output) : start_output;
        double high = log_scale ? std::log10(end_output) : end_output;
        for (int i = 0; i < major_outputs; i++) {
            double time = low + i * (high - low) / (major_outputs - 1);
            if (log_scale)
                time = std::pow(10, time);
            runner.add_output(time, true);
        }
        for (int i = 0; i < minor_outputs; i++) {
            double time = low + i * (high - low) / (minor_outputs - 1);
            if (log_scale)
                time = std::pow(10, time);
            runner.add_output(time, false);
        }
    }

    // reads command line options.  Returns the index of the first non-option argument
    int read_command_line(int argc, char* argv[], bool verify = true) {
        optind = 1; // reset library's state.  Needed for testing and if user wants to read arguments 2+ times

        int option_code;
        const option long_options[] = {
            {"dt", required_argument, nullptr, 'd'},           {"grid", required_argument, nullptr, 'n'},
            {"endtime", required_argument, nullptr, 't'},      {"majoroutputs", required_argument, nullptr, 'm'},
            {"minoroutputs", required_argument, nullptr, 'o'}, {"startoutput", required_argument, nullptr, 's'},
            {"endoutput", required_argument, nullptr, 'e'},    {"log", no_argument, nullptr, 'l'},
            {"outputatzero", no_argument, nullptr, 'z'},       {0, 0, 0, 0}};
        while ((option_code = getopt_long(argc, argv, "d:n:t:m:o:s:e:lz", long_options, nullptr)) != -1)
            read_argument(option_code);
        // endtime and endoutput default to each other if only one is specified:
        if (end_output == 0 && end_time != 0)
            end_output = end_time;
        if (end_time == 0 && end_output != 0)
            end_time = end_output;
        if (verify)
            verify_options();
        return optind;
    }

    // checks the options for logical consistency.  Raises std::invalid_argument
    void verify_options() {
        if (grid_points <= 0)
            throw std::invalid_argument("grid points must be specified and positive");
        if (dt <= 0)
            throw std::invalid_argument("dt must be specified and positive");
        if (end_time <= 0)
            throw std::invalid_argument("end time must be specified and positive");
        // if we're outputting, need a range to output over:
        bool outputting = major_outputs > 0 || minor_outputs > 0;
        if (outputting && start_output >= end_output)
            throw std::invalid_argument("startoutput cannot be >= endoutput when outputting");
        if (outputting && (start_output < 0 || end_output < 0))
            throw std::invalid_argument("output bounds cannot be negative");
        // log(0) = -inf so that can't be start:
        if (outputting && log_scale && start_output == 0)
            throw std::invalid_argument("log-scale outputs cannot start at 0.  Use --outputatzero instead");
    }

  private:
    // helper for read_command_line
    void read_argument(int option_code) {
        switch (option_code) {
        case 'd':
            dt = std::stod(optarg);
            return;
        case 'n':
            grid_points = std::stoi(optarg);
            return;
        case 't':
            end_time = std::stod(optarg);
            return;
        case 'm':
            major_outputs = std::stoi(optarg);
            return;
        case 'o':
            minor_outputs = std::stoi(optarg);
            return;
        case 's':
            start_output = std::stod(optarg);
            return;
        case 'e':
            end_output = std::stod(optarg);
            return;
        case 'l':
            log_scale = true;
            return;
        case 'z':
            output_at_zero = true;
            return;
        case '?':
            throw std::invalid_argument("");
        }
    }
};

} // namespace CabanaPF

#endif
