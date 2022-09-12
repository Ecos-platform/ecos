
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/lib_info.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger.hpp"
#include "ecos/scenario/scenario_loader.hpp"
#include "ecos/simulation_runner.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <CLI/CLI.hpp>
#include <iostream>
#include <sstream>

using namespace ecos;

namespace
{

std::unordered_map<std::string, ecos_logger::level> map{
    {"trace",  ecos_logger::level::trace},
    {"debug",  ecos_logger::level::debug},
    {"info",  ecos_logger::level::info},
    {"warn",  ecos_logger::level::warn},
    {"err",  ecos_logger::level::err},
    {"off",  ecos_logger::level::off},
};

ecos_logger::level lvl =  ecos_logger::level::info;

int print_help(const CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return 0;
}

std::string version()
{
    const auto v = library_version();
    std::stringstream ss;
    ss << "v" << v.major << "." << v.minor << "." << v.patch;
    return ss.str();
}

void create_options(CLI::App& app)
{
    app.set_version_flag("-v,--version", version());

    app.add_flag("-i,--interactive", "Make execution interactive.")->configurable(false);
    app.add_flag("--noLog", "Disable CSV logging.")->configurable(false);
    app.add_flag("--noParallel", "Run single-threaded.")->configurable(false);

    app.add_option("--path", "Location of the fmu/ssp to simulate.")->required();
    app.add_option("--stopTime", "Simulation end.")->default_val(1.0);
    app.add_option("--startTime", "Simulation start.")->default_val(0.0);
    app.add_option("--stepSize", "Simulation stepSize.")->required();
    app.add_option("--rtf", "Target real time factor (non-positive number -> inf).")->default_val(-1);
    app.add_option("--logConfig", "Path to logging configuration.");
    app.add_option("--chartConfig", "Path to chart configuration.");
    app.add_option("--scenarioConfig", "Path to scenario configuration.");
    app.add_option("-l,--logLevel", lvl, "Specify log level.")->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
}

std::unique_ptr<simulation_structure> create_structure(const std::filesystem::path& path, std::string& csvName)
{
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("No such file: " + std::filesystem::absolute(path).string());
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

void setup_scenario(const CLI::App& vm, simulation& sim)
{
    if (vm.count("--scenarioConfig")) {
        std::filesystem::path config = vm["--scenarioConfig"]->as<std::string>();
        load_scenario(sim, config);
    }
}

void setup_logging(const CLI::App& vm, simulation& sim, const std::string& csvName)
{
    if (!vm.get_option("--noLog")->as<bool>()) {

        std::optional<csv_config> config;
        if (vm.count("--logConfig")) {
            config = csv_config::parse(vm["--logConfig"]->as<std::string>());
        }

        auto logger = std::make_unique<csv_writer>(csvName + ".csv", config);
        if (vm.count("--chartConfig")) {
            logger->enable_plotting(vm["--chartConfig"]->as<std::string>());
        }
        sim.add_listener(std::move(logger));
    }
}

void run_simulation(const CLI::App& vm, simulation& sim)
{

    const auto interactive = vm["--interactive"]->as<bool>();

    const auto startTime = vm["--startTime"]->as<double>();
    const auto stepSize = vm["--stepSize"]->as<double>();
    const auto stopTime = vm["--stopTime"]->as<double>();

    sim.init(startTime);

    const auto rtf = vm["--rtf"]->as<double>();
    simulation_runner runner(sim);
    runner.set_real_time_factor(rtf);

    const double totalSimulationTime = stopTime - startTime;
    const unsigned long numSteps = static_cast<long>(totalSimulationTime / stepSize);
    const unsigned long aTenth = numSteps / 10;

    runner.set_callback([&] {
        const unsigned long i = sim.iterations();
        const double percentComplete = static_cast<double>(i) / static_cast<double>(numSteps) * 100;
        if (i != 0 && i % aTenth == 0 || i == numSteps) {
            info("{}% complete, simulated {:.3f}s in {:.3f}s, target RTF={:.2f}, actual RTF={:.2f}",
                percentComplete,
                sim.time(),
                runner.wall_clock(),
                runner.target_real_time_factor(),
                runner.real_time_factor());
        }
    });

    if (interactive) {
        std::cout << "Command line options:" << std::endl;
        std::cout << "\t'q' -> exit application.." << std::endl;
        std::cout << "\t'p' -> pause simulation.." << std::endl;
    }

    info("Simulation commencing. Start={}s, stop={}s, stepSize={}s, target RTF={}",
        startTime, stopTime, stepSize, runner.target_real_time_factor());
    auto f = runner.run_while([&] {
        return sim.time() <= stopTime;
    });

    bool inputQuit = false;
    std::thread inputThread;
    if (interactive) {
        inputThread = std::thread([&inputQuit, &sim, &runner] {
            std::string s;
            while (!sim.terminated()) {
                std::getline(std::cin, s);
                if (!s.empty()) {
                    switch (s[0]) {
                        case 'q':
                            if (!sim.terminated()) {
                                runner.stop();
                                inputQuit = true;
                                info("Simulation manually aborted at t={:.3f}s", sim.time());
                            }
                            return;
                        case 'p':
                            if (runner.toggle_pause()) {
                                info("Simulation paused at t={:.3f}. Press 'p' to continue..", sim.time());
                            } else {
                                info("Simulation un-paused..");
                            }
                            break;
                    }
                }
            }
        });
    }

    f.get();

    sim.terminate();

    if (inputThread.joinable()) {
        if (!inputQuit) {
            std::cerr << "Press 'q' to exit application.." << std::endl;
        }
        inputThread.join();
    }
}

} // namespace

int main(int argc, char** argv)
{

    CLI::App app("ecos");
    create_options(app);

    if (argc == 1) {
        return print_help(app);
    }

    try {

        CLI11_PARSE(app, argc, argv)

        set_logging_level(lvl);

        std::string csvName;
        const std::filesystem::path path = app["--path"]->as<std::string>();
        std::unique_ptr<simulation_structure> ss = create_structure(path, csvName);

        const auto stepSize = app["--stepSize"]->as<double>();
        const bool parallel = !app["--noParallel"]->as<bool>();
        auto sim = ss->load(std::make_unique<fixed_step_algorithm>(stepSize, parallel));
        setup_logging(app, *sim, csvName);
        setup_scenario(app, *sim);

        run_simulation(app, *sim);

    } catch (std::exception& e) {
        std::cerr << "[ecos] Unhandled Exception reached the top of main: '" << e.what() << "', application will now exit" << std::endl;
        return 1;
    }
}
