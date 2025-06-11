
#include "ecos/listeners/csv_writer.hpp"

#include "ecos/logger/logger.hpp"
#include "ecos/simulation.hpp"

#include <pugixml.hpp>
#include <set>
#include <sstream>

using namespace ecos;

namespace
{

const char* separator = ", ";


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
    writeData(outFile_, sim);
}

void csv_writer::post_step(simulation& sim)
{
    if (sim.iterations() % config().decimation_factor() == 0) {
        writeData(outFile_, sim);
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


void csv_config::report(const std::vector<variable_identifier>& ids) const
{
    if (variable_register.empty()) {
        log::debug("Logging all {} variables", ids.size());
    } else {

        int missingCount = 0;
        std::stringstream missing;
        std::set<variable_identifier> found;
        for (const auto& pattern : variable_register) {
            bool matched = false;

            for (const auto& id : ids) {
                if (id.matches(pattern)) {
                    found.emplace(id);
                    matched = true;
                }
            }

            if (!matched) {
                if (missingCount++ > 0) missing << ", ";
                missing << pattern.str();
            }
        }
        if (missingCount > 0) {
            log::warn("Missing {} variables declared for logging: {}", missingCount, missing.str());
        }

        std::stringstream found_str;
        bool first = true;
        for (const auto& elem : found) {
            if (!first) found_str << ", ";
            first = false;
            found_str << elem;
        }

        log::debug("Logging {} variables: {}", found.size(), found_str.str());
    }
}

bool csv_config::should_log(const variable_identifier& identifier) const
{
    if (variable_register.empty()) return true;

    return std::ranges::any_of(variable_register, [&](const variable_identifier& pattern) {
        return identifier.matches(pattern);
    });
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
    std::ostringstream line;
    line << "iterations" << separator << "time";

    config_.report(sim.identifiers());

    for (const auto& instance : sim.get_instances()) {

        const auto instanceName = instance->instanceName();
        const auto& properties = instance->get_properties();

        for (const auto& [variableName, p] : properties.get_reals()) {
            if (config_.should_log({instanceName, variableName})) {
                line << separator << instanceName << "::" << variableName << "[REAL]";
                props_.emplace_back(p.get());
            }
        }
        for (const auto& [variableName, p] : properties.get_integers()) {
            if (config_.should_log({instanceName, variableName})) {
                line << separator << instanceName << "::" << variableName << "[INT]";
                props_.emplace_back(p.get());
            }
        }
        for (const auto& [variableName, p] : properties.get_booleans()) {
            if (config_.should_log({instanceName, variableName})) {
                line << separator << instanceName << "::" << variableName << "[BOOL]";
                props_.emplace_back(p.get());
            }
        }
        for (const auto& [variableName, p] : properties.get_strings()) {
            if (config_.should_log({instanceName, variableName})) {
                line << separator << instanceName << "::" << variableName << "[STR]";
                props_.emplace_back(p.get());
            }
        }
    }


    outFile_ << line.str() << "\n";
    outFile_.flush();
}

void csv_writer::writeData(std::ofstream& out, const simulation& sim)
{
    std::ostringstream line;
    line << sim.iterations() << separator << sim.time();

    for (const auto& p : props_) {

        line << separator << *p;
    }

    out << line.str() << "\n";
}
