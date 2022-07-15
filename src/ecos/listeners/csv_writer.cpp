
#include "ecos/listeners/csv_writer.hpp"

#include "pugixml.hpp"
#include "spdlog/spdlog.h"

#include "ecos/simulation.hpp"

#include <sstream>
#include <thread>

using namespace ecos;

namespace
{

const char* separator = ", ";

void writeData(std::ofstream& out, const simulation& sim, const std::optional<csv_config>& config)
{
    out << sim.iterations() << separator << sim.time();

    for (auto& instance : sim.get_instances()) {

        bool logInstance = true;
        if (config) {
            logInstance = config->shouldLogInstance(instance->instanceName);
        }

        if (logInstance) {
            auto& properties = instance->get_properties();

            for (auto& [name, p] : properties.get_reals()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << separator << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_integers()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << separator << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_booleans()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << separator << std::noboolalpha << p->get_value();
            }
            for (auto& [name, p] : properties.get_strings()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << separator << p->get_value();
            }
        }
    }
    out << "\n";
}

} // namespace

csv_writer::csv_writer(const std::filesystem::path& path, std::optional<csv_config> config)
    : path_(std::filesystem::absolute(path))
    , config_(std::move(config))
{
    if (path.extension().string() != ".csv") {
        throw std::runtime_error("File extension must be .csv, was: " + path.extension().string());
    }

    const auto parentPath = path_.parent_path();
    if (!std::filesystem::exists(parentPath)) {
        if (!std::filesystem::create_directories(parentPath)) {
            throw std::runtime_error("Unable to create missing directories for path: " + path_.string());
        }
    }

    outFile_.open(path.string());
}

csv_writer::csv_writer(const std::filesystem::path& path, const std::filesystem::path& configPath)
    : csv_writer(path, csv_config::parse(configPath))
{ }

void csv_writer::pre_init(simulation& sim)
{
    outFile_ << "iterations" << separator << "time";

    if (config_) {
        config_->verify(sim.identifiers());
    }

    for (auto& instance : sim.get_instances()) {

        bool logInstance = true;
        const auto instanceName = instance->instanceName;
        if (config_) {
            logInstance = config_->shouldLogInstance(instanceName);
        }

        if (logInstance) {
            auto& properties = instance->get_properties();
            for (auto& [variableName, p] : properties.get_reals()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(variableName);
                }
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[REAL]";
            }
            for (auto& [variableName, p] : properties.get_integers()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(variableName);
                }
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[INT]";
            }
            for (auto& [variableName, p] : properties.get_booleans()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(variableName);
                }
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[BOOL]";
            }
            for (auto& [variableName, p] : properties.get_strings()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(variableName);
                }
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[STR]";
            }
        }
    }

    outFile_ << "\n";
    outFile_.flush();
}

void csv_writer::post_init(simulation& sim)
{
    writeData(outFile_, sim, config_);
}

void csv_writer::post_step(simulation& sim)
{
    writeData(outFile_, sim, config_);
}

void csv_writer::post_terminate(simulation& sim)
{
    outFile_.flush();
    outFile_.close();
    spdlog::info("Wrote CSV data to file: {}", path_.string());

    if (plotConfig_) {
        if (!std::filesystem::exists(*plotConfig_)) {
            throw std::runtime_error("No such file: " + plotConfig_->string());
        }
        std::stringstream ss;
        ss << "python ecos_plotter.py \"" << path_.string() << "\" \"" << plotConfig_->string() << "\"";
        auto t = std::thread([&ss] {
            system(ss.str().c_str());
        });
        spdlog::info("Waiting for plotting window(s) to close..");
        t.join();
        spdlog::info("Plotting window(s) closed.");
    }
}

void csv_writer::enable_plotting(const std::filesystem::path& plotConfig)
{
    std::filesystem::path plotter("ecos_plotter.py");
    if (std::filesystem::exists(plotter)) {
        plotConfig_ = std::filesystem::absolute(plotConfig);
    } else {
        spdlog::warn("Plotting will be disabled as {} is not present.", std::filesystem::absolute(plotter).string());
    }
}


void csv_config::verify(const std::vector<variable_identifier>& ids)
{
    int foundCount = 0;
    int missingCount = 0;
    std::stringstream missing;
    std::stringstream found;
    for (auto& v : variablesToLog_) {
        if (std::find(ids.begin(), ids.end(), v) == std::end(ids)) {
            if (missingCount > 0) {
                missing << ", ";
            }
            missing << v.str();
            ++missingCount;
        } else {
            if (foundCount > 0) {
                found << ", ";
            }
            found << v.str();
            ++foundCount;
        }
    }
    if (missingCount > 0) {
        spdlog::warn("Missing {} variables declared for logging: {}", missingCount, missing.str());
    }
    spdlog::debug("Logging {} variables: {}", foundCount, found.str());
}

bool csv_config::shouldLogVariable(const std::string& variableName) const
{
    bool log = std::find_if(variablesToLog_.begin(), variablesToLog_.end(), [variableName](const variable_identifier& v) {
        return v.variableName == variableName;
    }) != std::end(variablesToLog_);
    return log;
}

bool csv_config::shouldLogInstance(const std::string& instanceName) const
{
    bool log = std::find_if(variablesToLog_.begin(), variablesToLog_.end(), [instanceName](const variable_identifier& v) {
        return v.instanceName == instanceName;
    }) != std::end(variablesToLog_);
    return log;
}

csv_config csv_config::parse(const std::filesystem::path& configPath)
{
    if (!std::filesystem::exists(configPath)) {
        throw std::runtime_error("No such file: " + std::filesystem::absolute(configPath).string());
    }
    const auto ext = configPath.extension().string();
    if (ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(configPath.c_str());
    if (!result) {
        throw std::runtime_error(
            "Unable to parse '" + std::filesystem::absolute(configPath).string() + "': " +
            result.description());
    }

    csv_config config;
    const auto root = doc.child("ecos:LogConfig");
    const auto components = root.child("ecos:components");
    for (const auto& instances : components) {
        const auto instanceName = instances.attribute("name").as_string();
        for (const auto& variable : instances) {
            const auto variableName = variable.attribute("name").as_string();
            config.log_variable(variable_identifier{instanceName, variableName});
        }
    }
    return config;
}
