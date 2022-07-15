
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/scenario/scenario_loader.hpp"
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

po::options_description create_description()
{
    po::options_description desc("Options");
    desc.add_options()("help,h", "Print this help message and quits.");
    //    desc.add_options()("version,v", "Print program version.");
    desc.add_options()("path", po::value<std::string>()->required(), "Location of the fmu/ssp to run_simulation.");
    desc.add_options()("stopTime", po::value<double>()->default_value(1), "Simulation end.");
    desc.add_options()("startTime", po::value<double>()->default_value(0), "Simulation start.");
    desc.add_options()("stepSize", po::value<double>()->required(), "Simulation stepSize.");
    desc.add_options()("rtf", po::value<double>()->default_value(-1), "Target real time factor (non-positive number -> inf).");
    desc.add_options()("noLog", po::bool_switch()->default_value(false), "Disable CSV logging.");
    desc.add_options()("noParallel", po::bool_switch()->default_value(false), "Run single-threaded.");
    desc.add_options()("logConfig", po::value<std::string>(), "Path to logging configuration.");
    desc.add_options()("chartConfig", po::value<std::string>(), "Path to chart configuration.");
    desc.add_options()("scenarioConfig", po::value<std::string>(), "Path to scenario configuration.");

    return desc;
}

std::unique_ptr<simulation_structure> create_structure(const std::filesystem::path& path, std::string& csvName)
{
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("No such file: " + path.string());
    }

    std::unique_ptr<simulation_structure> ss;
    if (path.extension() == ".ssp" || std::filesystem::is_directory(path)) {
        if (std::filesystem::is_directory(path)) {
            bool ssdFound;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.path().filename() == "SystemStructure.ssd") {
                    ssdFound = true;
                    break;
                }
            }
            if (!ssdFound) {
                throw std::runtime_error("No SystemStructure.ssd found in directory: " + path.string());
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
    return ss;
}

void setup_scenario(const po::variables_map& vm, simulation& sim)
{
    if (vm.count("scenarioConfig")) {
        std::filesystem::path config = vm["scenarioConfig"].as<std::string>();
        load_scenario(sim, config);
    }
}

void setup_logging(const po::variables_map& vm, simulation& sim, const std::string& csvName)
{
    if (!vm["noLog"].as<bool>()) {

        std::optional<csv_config> config;
        if (vm.count("logConfig")) {
            config = csv_config::parse(vm["logConfig"].as<std::string>());
        }

        auto logger = std::make_unique<csv_writer>(csvName + ".csv", config);
        if (vm.count("chartConfig")) {
            logger->enable_plotting(vm["chartConfig"].as<std::string>());
        }
        sim.add_listener(std::move(logger));
    }
}

void run_simulation(const po::variables_map& vm, simulation& sim)
{

    double startTime = vm["startTime"].as<double>();
    double stepSize = vm["stepSize"].as<double>();
    double stopTime = vm["stopTime"].as<double>();

    sim.init(startTime);

    double rtf = vm["rtf"].as<double>();
    simulation_runner runner(sim);
    runner.set_real_time_factor(rtf);

    double totalSimulationTime = stopTime - startTime;
    unsigned long numSteps = static_cast<long>(totalSimulationTime / stepSize);
    unsigned long aTenth = numSteps / 10;

    runner.set_callback([&] {
        unsigned long i = sim.iterations();
        double percentComplete = static_cast<double>(i) / numSteps * 100;
        if (i != 0 && i % aTenth == 0 || i == numSteps) {
            spdlog::info("{}% complete, simulated {:.3f}s in {:.3f}s, target RTF={:.2f}, actual RTF={:.2f}",
                percentComplete,
                sim.time(),
                runner.wall_clock(),
                runner.target_real_time_factor(),
                runner.real_time_factor());
        }
    });

    auto f = runner.run_while([&] {
        return sim.time() <= stopTime;
    });

    bool inputQuit = false;
    std::thread inputThread = std::thread([&inputQuit, &sim, &runner] {

        std::cout << "Command line options:" << std::endl;
        std::cout << "\t'q' -> exit application.." << std::endl;
        std::cout << "\t'p' -> pause simulation.." << std::endl;

        std::string s;
        while (true) {
            std::getline(std::cin, s);
            if (!s.empty()) {
                switch (s[0]) {
                    case 'q':
                        runner.stop();
                        inputQuit = true;
                        spdlog::info("Simulation manually aborted at t={:.3f}", sim.time());
                        return;
                    case 'p':
                        if (runner.toggle_pause()) {
                            spdlog::info("Simulation paused at t={:.3f}. Press 'p' to continue..", sim.time());
                        } else {
                            spdlog::info("Simulation un-paused..");
                        }
                        break;
                }
            }
        }
    });

    f.get();

    sim.terminate();

    if (inputThread.joinable()) {
        if (!inputQuit) {
            spdlog::info("Press 'q' to exit application..");
        }
        inputThread.join();
    }

}

} // namespace

int main(int argc, char** argv)
{

    po::options_description desc = create_description();

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

        std::string csvName;
        const std::filesystem::path path = vm["path"].as<std::string>();
        std::unique_ptr<simulation_structure> ss = create_structure(path, csvName);

        double stepSize = vm["stepSize"].as<double>();
        auto sim = ss->load(std::make_unique<fixed_step_algorithm>(stepSize));
        setup_logging(vm, *sim, csvName);
        setup_scenario(vm, *sim);

        run_simulation(vm, *sim);

    } catch (std::exception& e) {
        std::cerr << "[ecos] Unhandled Exception reached the top of main: " << e.what() << ", application will now exit" << std::endl;
        return -1;
    }
}
