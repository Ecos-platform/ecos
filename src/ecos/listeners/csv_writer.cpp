
#include "ecos/listeners/csv_writer.hpp"

#include "ecos/logger/logger.hpp"
#include "ecos/simulation.hpp"

#include <pugixml.hpp>
#include <sstream>

using namespace ecos;

namespace
{

const char* separator = ", ";

void writeData(std::ofstream& out, const simulation& sim, const csv_config& config)
{
    out << sim.iterations() << separator << sim.time();

    for (const auto& instance : sim.get_instances()) {

        const auto instanceName = instance->instanceName();
        const auto& properties = instance->get_properties();

        for (auto& [variableName, p] : properties.get_reals()) {
            if (config.should_log({instanceName, variableName})) {
                out << separator << std::to_string(p->get_value());
            }
        }
        for (auto& [variableName, p] : properties.get_integers()) {
            if (config.should_log({instanceName, variableName})) {
                out << separator << std::to_string(p->get_value());
            }
        }
        for (auto& [variableName, p] : properties.get_booleans()) {
            if (config.should_log({instanceName, variableName})) {
                out << separator << std::noboolalpha << p->get_value();
            }
        }
        for (auto& [variableName, p] : properties.get_strings()) {
            if (config.should_log({instanceName, variableName})) {
                out << separator << p->get_value();
            }
        }
    }

    out << "\n";
}

} // namespace

std::string plotScript();

csv_writer::csv_writer(const std::filesystem::path& path)
    : config_(csv_config{})
    , path_(absolute(path))
{
    if (path.extension().string() != ".csv") {
        throw std::runtime_error("File extension must be .csv, was: " + path.extension().string());
    }

    const auto parentPath = path_.parent_path();
    if (!exists(parentPath)) {
        if (!create_directories(parentPath)) {
            throw std::runtime_error("Unable to create missing directories for path: " + path_.string());
        }
    }

    outFile_.open(path.string());
}

void csv_writer::pre_init(simulation& sim)
{
    if (!headerWritten) {
        write_header(sim);
        headerWritten = true;
    }
}

void csv_writer::post_init(simulation& sim)
{
    writeData(outFile_, sim, config_);
}

void csv_writer::post_step(simulation& sim)
{
    if (sim.iterations() % config().decimation_factor() == 0) {
        writeData(outFile_, sim, config_);
    }
}

void csv_writer::post_terminate(simulation& sim)
{
    outFile_.flush();
    outFile_.close();
    log::info("Wrote CSV data to file: '{}'", path_.string());
}

void csv_writer::on_reset()
{
    if (config().clear_on_reset_) {
        headerWritten = false;
        outFile_ = std::ofstream(path_.string(), std::ios::out | std::ios::trunc);
    }
}


void csv_config::verify(const std::vector<variable_identifier>& ids) const
{
    if (variable_register.empty()) {
        log::debug("Logging all {} variables", ids.size());
    } else {

        int foundCount = 0;
        int missingCount = 0;
        std::stringstream missing;
        std::stringstream found;
        for (const auto& v : variable_register) {
            if (std::ranges::find(ids, v) == std::end(ids)) {
                if (missingCount++ > 0) {
                    missing << ", ";
                }
                missing << v.str();
            } else {
                if (foundCount++ > 0) {
                    found << ", ";
                }
                found << v.str();
            }
        }
        if (missingCount > 0) {
            log::warn("Missing {} variables declared for logging: {}", missingCount, missing.str());
        }
        log::debug("Logging {} variables: {}", foundCount, found.str());
    }
}

bool csv_config::should_log(const variable_identifier& identifier) const
{
    if (variable_register.empty()) return true;

    const bool log = std::ranges::find(variable_register, identifier) != std::end(variable_register);
    return log;
}

void csv_config::load(const std::filesystem::path& configPath)
{
    if (!exists(configPath)) {
        throw std::runtime_error("No such file: '" + absolute(configPath).string() + "'");
    }
    if (const auto ext = configPath.extension().string(); ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(configPath.c_str());
    if (!result) {
        throw std::runtime_error(
            "Unable to parse '" + absolute(configPath).string() + "': " +
            result.description());
    }

    const auto root = doc.child("ecos:CsvConfig");
    if (const auto decimationFactor = root.attribute("decimationFactor")) {
        decimationFactor_ = decimationFactor.as_int();
    }

    const auto components = root.child("ecos:components");
    for (const auto& instances : components) {
        const auto instanceName = instances.attribute("name").as_string();
        for (const auto& variable : instances) {
            const auto variableName = variable.attribute("name").as_string();
            register_variable({instanceName, variableName});
        }
    }
}

void csv_config::register_variable(variable_identifier v)
{
    variable_register.emplace_back(std::move(v));
}

void csv_config::clear_on_reset(bool flag)
{
    clear_on_reset_ = flag;
}

size_t& csv_config::decimation_factor()
{
    return decimationFactor_;
}

void csv_writer::write_header(const simulation& sim)
{
    outFile_ << "iterations" << separator << "time";

    config_.verify(sim.identifiers());

    for (const auto& instance : sim.get_instances()) {

        const auto instanceName = instance->instanceName();
        const auto& properties = instance->get_properties();

        for (const auto& variableName : properties.get_reals() | std::views::keys) {
            if (config_.should_log({instanceName, variableName})) {
                outFile_ << separator << instanceName << "::" << variableName << "[REAL]";
            }
        }
        for (const auto& variableName : properties.get_integers() | std::views::keys) {
            if (config_.should_log({instanceName, variableName})) {
                outFile_ << separator << instanceName << "::" << variableName << "[INT]";
            }
        }
        for (const auto& variableName : properties.get_booleans() | std::views::keys) {
            if (config_.should_log({instanceName, variableName})) {
                outFile_ << separator << instanceName << "::" << variableName << "[BOOL]";
            }
        }
        for (const auto& variableName : properties.get_strings() | std::views::keys) {
            if (config_.should_log({instanceName, variableName})) {
                outFile_ << separator << instanceName << "::" << variableName << "[STR]";
            }
        }
    }


    outFile_ << "\n";
    outFile_.flush();
}
