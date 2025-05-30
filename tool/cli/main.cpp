
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/lib_info.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/simulation_runner.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/util/plotter.hpp"

#include <cli11/CLI11.h>
#include <iostream>

using namespace ecos;

namespace
{

std::unordered_map<std::string, log::level> map{
    {"trace", log::level::trace},
    {"debug", log::level::debug},
    {"info", log::level::info},
    {"warn", log::level::warn},
    {"err", log::level::err},
    {"off", log::level::off},
};

log::level lvl = log::level::info;

int print_help(const CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return 0;
}

std::string versionString()
{
    const auto v = library_version();
    std::stringstream ss;
    ss << "v" << v.major << "." << v.minor << "." << v.patch;
    return ss.str();
}

void create_options(CLI::App& app)
{
    app.set_version_flag("-v,--version", versionString());

    app.add_flag("-i,--interactive", "Make execution interactive.")->configurable(false);
    app.add_flag("--noCsv", "Disable CSV logging.")->configurable(false);
    app.add_flag("--noParallel", "Run single-threaded.")->configurable(false);

    app.add_option("--path", "Location of the fmu/ssp to simulate.")->required();
    app.add_option("--stopTime", "Simulation end.")->default_val(1.0);
    app.add_option("--startTime", "Simulation start.")->default_val(0.0);
    app.add_option("--stepSize", "Simulation stepSize.")->required();
    app.add_option("--rtf", "Target real time factor (non-positive number -> inf).")->default_val(-1);
    app.add_option("--parameterSet", "Name of SSP parameterSet to apply.");
    app.add_option("--csvConfig", "Path to CSV configuration.");
    app.add_option("--chartConfig", "Path to chart configuration.");
    app.add_option("--scenarioConfig", "Path to scenario configuration.");
    app.add_option("-l,--logLevel", lvl, "Specify log level.")->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
}

std::unique_ptr<simulation_structure> create_structure(const std::filesystem::path& path, std::string& csvName)
{
    if (!exists(path)) {
        throw std::runtime_error("No such file: " + absolute(path).string());
    }

    std::unique_ptr<simulation_structure> ss;
    if (path.extension() == ".ssp" || is_directory(path)) {
        if (is_directory(path)) {
            bool ssdFound{false};
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
        const auto resolver = default_model_resolver();
        ss->add_model("instance", resolver->resolve(path.string()));
        csvName = path.stem().string();
    }
    return ss;
}

void setup_scenario(const CLI::App& vm, simulation& sim)
{
    if (vm.count("--scenarioConfig")) {
        const std::filesystem::path config = vm["--scenarioConfig"]->as<std::string>();
        sim.load_scenario(config);
    }
}

void setup_logging(const CLI::App& vm, simulation& sim, const std::string& csvName)
{
    if (!vm.get_option("--noCsv")->as<bool>()) {

        auto writer = std::make_unique<csv_writer>(csvName + ".csv");
        csv_config& config = writer->config();

        if (vm.count("--csvConfig")) {
            config.load(vm["--csvConfig"]->as<std::string>());
        }
        if (vm.count("--chartConfig")) {
            const auto chartConfig = vm["--chartConfig"]->as<std::string>();
            const auto outputPath = writer->output_path();
            sim.add_listener("plotter", std::make_shared<post_terminate_hook>([outputPath, chartConfig] {
                plot_csv(outputPath, chartConfig);
            }));
        }

        sim.add_listener("csv_writer", std::move(writer));
    }
}

void run_simulation(const CLI::App& vm, simulation& sim)
{

    const auto interactive = vm["--interactive"]->as<bool>();

    const auto startTime = vm["--startTime"]->as<double>();
    const auto stepSize = vm["--stepSize"]->as<double>();
    const auto stopTime = vm["--stopTime"]->as<double>();

    std::optional<std::string> parameterSetName;
    if (vm.count("--parameterSet")) {
        parameterSetName = vm["--parameterSet"]->as<std::string>();
    }

    sim.init(startTime, parameterSetName);

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
            log::info("{}% complete, simulated {:.3f}s in {:.3f}s, target RTF={:.2f}, actual RTF={:.2f}",
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

    log::info("Simulation commencing. Start={}s, stop={}s, stepSize={}s, target RTF={}",
        startTime, stopTime, stepSize, runner.target_real_time_factor());
    auto f = runner.run_while([&] {
        return sim.time() + stepSize <= stopTime;
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
                                log::info("Simulation manually aborted at t={:.3f}s", sim.time());
                            }
                            return;
                        case 'p':
                            if (runner.toggle_pause()) {
                                log::info("Simulation paused at t={:.3f}. Press 'p' to continue..", sim.time());
                            } else {
                                log::info("Simulation un-paused..");
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
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(stepSize, parallel));
        setup_logging(app, *sim, csvName);
        setup_scenario(app, *sim);

        run_simulation(app, *sim);

    } catch (std::exception& e) {
        log::err("Unhandled Exception reached the top of main: '{}', application will now exit", e.what());
        return 1;
    }
}
