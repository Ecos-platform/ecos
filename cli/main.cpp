
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/simulation_runner.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <boost/program_options.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace ecos;
namespace po = boost::program_options;

namespace
{

int printHelp(boost::program_options::options_description& desc)
{
    std::cout << "ecos" << '\n'
              << desc << std::endl;
    return 0;
}

} // namespace

int main(int argc, char** argv)
{

    po::options_description desc("Options");
    desc.add_options()("help,h", "Print this help message and quits.");
    //    desc.add_options()("version,v", "Print program version.");
    desc.add_options()("path", po::value<std::string>()->required(), "Location of the fmu/ssp to simulate.");
    desc.add_options()("stopTime", po::value<double>()->default_value(1), "Simulation end.");
    desc.add_options()("startTime", po::value<double>()->default_value(0), "Simulation start.");
    desc.add_options()("stepSize", po::value<double>()->required(), "Simulation stepSize.");
    desc.add_options()("rtf", po::value<double>()->default_value(-1), "Target real time factor (non-positive number -> inf).");
    desc.add_options()("noLog", po::bool_switch()->default_value(false), "Disable CSV logging.");
    desc.add_options()("noParallel", po::bool_switch()->default_value(false), "Run single-threaded.");
    desc.add_options()("logConfig", po::value<std::string>(), "Path to logging configuration.");
    desc.add_options()("chartConfig", po::value<std::string>(), "Path to chart configuration.");

    if (argc == 1) {
        return printHelp(desc);
    }

    try {
        po::variables_map vm;

        try {

            po::store(po::parse_command_line(argc, argv, desc), vm);

            if (vm.count("help")) {
                return printHelp(desc);
            }

            po::notify(vm);

        } catch (po::error& e) {
            std::cerr << "ERROR: " << e.what() << std::endl
                      << std::endl;
            std::cerr << desc << std::endl;
            return -1;
        }

        const std::filesystem::path path = vm["path"].as<std::string>();

        if (!std::filesystem::exists(path)) {
            std::cerr << "No such file: " << path.string() << std::endl;
            return -1;
        }

        std::unique_ptr<simulation_structure> ss;

        std::string csvName;
        if (path.extension() == ".ssp" || std::filesystem::is_directory(path)) {
            if (std::filesystem::is_directory(path)) {
                bool ssdFound;
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.path().extension() == ".ssd") {
                        ssdFound = true;
                        break;
                    }
                }
                if (!ssdFound) {
                    std::cerr << "No such ssd found in directory: " << path.string() << std::endl;
                    return -1;
                }
            }

            ss = load_ssp(path);
            csvName = path.stem().string();
        } else if (path.extension() == ".fmu") {
            ss = std::make_unique<simulation_structure>();
            auto resolver = default_model_resolver();
            ss->add_model("instance", resolver->resolve(path.string()));
            csvName = path.stem().string();
        }

        double startTime = vm["startTime"].as<double>();
        double stepSize = vm["stepSize"].as<double>();
        double stopTime = vm["stopTime"].as<double>();

        auto sim = ss->load(std::make_unique<fixed_step_algorithm>(stepSize));

        if (!vm["noLog"].as<bool>()) {

            std::optional<csv_config> config;
            if (vm.count("logConfig")) {
                config = csv_config::parse(vm["logConfig"].as<std::string>());
            }

            auto logger = std::make_unique<csv_writer>(csvName + ".csv", config);
            if (vm.count("chartConfig")) {
                logger->enable_plotting(vm["chartConfig"].as<std::string>());
            }
            sim->add_listener(std::move(logger));
        }

        sim->init(startTime);

        simulation_runner runner(*sim);
        runner.set_real_time_factor(vm["rtf"].as<double>());

        double totalSimulationTime = stopTime - startTime;
        unsigned long numSteps = static_cast<long>(totalSimulationTime / stepSize);
        unsigned long aTenth = numSteps / 10;

        runner.set_callback([&] {
            unsigned long i = sim->iterations();
            double percentComplete = static_cast<double>(i) / numSteps * 100;
            if (i != 0L && i % aTenth == 0 || i == numSteps) {
                spdlog::info("{}% complete, simulated {:.3f}s in {:.3f}s, target RTF={:.2f}, actual RTF={:.2f}",
                    percentComplete,
                    sim->time(),
                    runner.wall_clock(),
                    runner.target_real_time_factor(),
                    runner.real_time_factor());
            }
        });

        auto f = runner.run_while([&] {
            return sim->time() <= stopTime;
        });

        f.get();

        sim->terminate();

    } catch (std::exception& e) {
        std::cerr << "[ecos] Unhandled Exception reached the top of main: " << e.what() << ", application will now exit" << std::endl;
        return -1;
    }
}
