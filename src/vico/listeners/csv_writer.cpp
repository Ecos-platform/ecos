
#include "vico/listeners/csv_writer.hpp"

#include "vico/simulation.hpp"

#include <iostream>
#include <sstream>

using namespace vico;

namespace
{

void writeData(std::ofstream& out, const simulation& sim, const std::optional<csv_config>& config)
{
    out << sim.iterations() << ", " << sim.time();

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
                if (logVar) out << ", " << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_integers()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << ", " << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_booleans()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << ", " << std::noboolalpha << p->get_value();
            }
            for (auto& [name, p] : properties.get_strings()) {
                bool logVar = true;
                if (config) {
                    logVar = config->shouldLogVariable(name);
                }
                if (logVar) out << ", " << p->get_value();
            }
        }
    }
    out << "\n";
    out.flush();
}

} // namespace

csv_writer::csv_writer(const fs::path& path, std::optional<csv_config> config)
    : config_(std::move(config))
{
    if (path.extension().string() != ".csv") {
        throw std::runtime_error("File extension must be .csv, was: " + path.extension().string());
    }

    const auto parentPath = fs::absolute(path).parent_path();
    if (!fs::exists(parentPath)) {
        if (!fs::create_directories(parentPath)) {
            throw std::runtime_error("Unable to create missing directories for path: " + fs::absolute(path).string());
        }
    }

    outFile_.open(path.string());
}

void csv_writer::pre_init(simulation& sim)
{
    outFile_ << "iterations, time";

    if (config_) {
        config_->verify(sim.identifiers());
    }

    for (auto& instance : sim.get_instances()) {

        bool logInstance = true;
        if (config_) {
            logInstance = config_->shouldLogInstance(instance->instanceName);
        }

        if (logInstance) {
            auto& properties = instance->get_properties();
            for (auto& [name, p] : properties.get_reals()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(name);
                }
                if (logVar) outFile_ << ", " << name << "[REAL]";
            }
            for (auto& [name, p] : properties.get_integers()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(name);
                }
                if (logVar) outFile_ << ", " << name << "[INT]";
            }
            for (auto& [name, p] : properties.get_booleans()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(name);
                }
                if (logVar) outFile_ << ", " << name << "[BOOL]";
            }
            for (auto& [name, p] : properties.get_strings()) {
                bool logVar = true;
                if (config_) {
                    logVar = config_->shouldLogVariable(name);
                }
                if (logVar) outFile_ << ", " << name << "[STR]";
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
    outFile_.close();
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
        std::cerr << "[warning] Missing " << missingCount << " variables declared for logging: "
                  << missing.str() << std::endl;
    }
    std::cout << "[info] Logging " << foundCount << " variables: "
              << found.str() << std::endl;
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
