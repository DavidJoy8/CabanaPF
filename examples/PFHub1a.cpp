#include <CabanaPF_CommandLine.hpp>
#include <CabanaPF_PFHub.hpp>

using namespace CabanaPF;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    {
        Kokkos::ScopeGuard scope_guard(argc, argv);
        try {
            CommandLineInput parser;
            const int arg_start = parser.read_command_line(argc, argv);
            if (argc == arg_start) // no problem name specified
                throw std::invalid_argument("");

            // read in the type of simulation to run:
            std::unique_ptr<PFHub1aBase> simulation;
            std::string problem_name(argv[arg_start]);
            if (problem_name == "2017")
                simulation = std::make_unique<PFHub1aBenchmark2017>(parser.grid_points, parser.dt);
            else if (problem_name == "2023")
                simulation = std::make_unique<PFHub1aCHiMaD2023>(parser.grid_points, parser.dt);
            else if (problem_name == "custom") {
                const int N_COEFFICIENTS = 12;
                if (argc < arg_start + N_COEFFICIENTS + 1)
                    throw std::invalid_argument("too few coefficients (need 12) for custom");

                std::array<int, N_COEFFICIENTS - 2> Ns;
                // read in cosine terms:
                for (int i = 0; i < 8; i++)
                    Ns[i] = std::stoi(argv[arg_start + i + 1]);
                // read in sine terms:
                const double A_X = std::stod(argv[arg_start + 9]);
                Ns[8] = std::stoi(argv[arg_start + 10]);
                const double A_Y = std::stod(argv[arg_start + 11]);
                Ns[9] = std::stoi(argv[arg_start + 12]);
                // make problem:
                simulation = std::make_unique<PFHub1aCustom>(parser.grid_points, parser.dt, Ns[0], Ns[1], Ns[2], Ns[3],
                                                             Ns[4], Ns[5], Ns[6], Ns[7], A_X, Ns[8], A_Y, Ns[9]);
            } else
                throw std::invalid_argument("unrecognized problem name");
            // print header (must be before adding outputs):
            std::cout << simulation->subproblem_name() << "\ntime,free_energy" << std::endl;
            // register when to output:
            parser.add_outputs_to_runner(*simulation);
            // do the run:
            simulation->run_until_time(parser.end_time);
        } catch (const std::invalid_argument& e) {
            std::cerr << e.what() << std::endl;
            std::cerr << "Run a version of the PFHub1a benchmark\nUsage: ./PFHub1a\n"
                      << CommandLineInput::HELP
                      << "<2017|2023|custom> [custom coefficients]\n"
                         "\t2017: The established benchmark\n"
                         "\t2023: Our modification proposed at the August 2023 meeting\n"
                         "\tcustom: Infinitely differentiable version with custom coefficients\n"
                      << std::endl;
        }
    }
    MPI_Finalize();
    return 0;
}
