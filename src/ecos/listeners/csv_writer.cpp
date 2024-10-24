
#include "ecos/listeners/csv_writer.hpp"

#include "ecos/logger/logger.hpp"
#include "ecos/simulation.hpp"

#include <pugixml.hpp>
#include <sstream>
#include <thread>

using namespace ecos;

namespace
{

const char* separator = ", ";

void writeData(std::ofstream& out, const simulation& sim, const csv_config& config)
{
    out << sim.iterations() << separator << sim.time();

    for (auto& instance : sim.get_instances()) {

        if (config.shouldLogInstance(instance->instanceName())) {
            auto& properties = instance->get_properties();

            for (auto& [name, p] : properties.get_reals()) {
                bool logVar = config.shouldLogVariable(name);
                if (logVar) out << separator << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_integers()) {
                bool logVar = config.shouldLogVariable(name);
                if (logVar) out << separator << std::to_string(p->get_value());
            }
            for (auto& [name, p] : properties.get_booleans()) {
                bool logVar = config.shouldLogVariable(name);
                if (logVar) out << separator << std::noboolalpha << p->get_value();
            }
            for (auto& [name, p] : properties.get_strings()) {
                bool logVar = config.shouldLogVariable(name);
                if (logVar) out << separator << p->get_value();
            }
        }
    }
    out << "\n";
}

} // namespace

std::string plotScript();

csv_writer::csv_writer(const std::filesystem::path& path)
    : path_(absolute(path))
    , config_(csv_config{})
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
    outFile_ << "iterations" << separator << "time";

    config_.verify(sim.identifiers());

    for (auto& instance : sim.get_instances()) {

        const auto instanceName = instance->instanceName();

        if (config_.shouldLogInstance(instanceName)) {
            auto& properties = instance->get_properties();
            for (auto& [variableName, p] : properties.get_reals()) {
                bool logVar = config_.shouldLogVariable(variableName);
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[REAL]";
            }
            for (auto& [variableName, p] : properties.get_integers()) {
                bool logVar = config_.shouldLogVariable(variableName);
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[INT]";
            }
            for (auto& [variableName, p] : properties.get_booleans()) {
                bool logVar = config_.shouldLogVariable(variableName);
                if (logVar) outFile_ << separator << instanceName << "::" << variableName << "[BOOL]";
            }
            for (auto& [variableName, p] : properties.get_strings()) {
                bool logVar = config_.shouldLogVariable(variableName);
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
    log::info("Wrote CSV data to file: '{}'", path_.string());

    if (auto plotConfig = config_.plotConfig_) {

        std::filesystem::path plotter("ecos_plotter.py");
        if (!exists(plotter)) {
            std::ofstream out(plotter, std::ios::trunc);
            out << plotScript();
        }

        std::stringstream ss;
        ss << "python ecos_plotter.py \"" << path_.string() << "\" \"" << plotConfig->string() << "\"";
        auto t = std::thread([&ss] {
            if (int status = system(ss.str().c_str())) {
                log::warn("Command {} returned with status: {}", ss.str(), status);
            }
        });
        log::info("Waiting for plotting window(s) to close..");
        t.join();
        log::info("Plotting window(s) closed.");

        std::filesystem::remove(plotter);
    }
}

void csv_config::verify(const std::vector<variable_identifier>& ids)
{
    if (variable_register.empty()) {
        log::debug("Logging all {} variables", ids.size());
    } else {

        int foundCount = 0;
        int missingCount = 0;
        std::stringstream missing;
        std::stringstream found;
        for (const auto& v : variable_register) {
            if (std::find(ids.begin(), ids.end(), v) == std::end(ids)) {
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

bool csv_config::shouldLogVariable(const std::string& variableName) const
{
    if (variable_register.empty()) return true;

    bool log = std::find_if(variable_register.begin(), variable_register.end(), [variableName](const variable_identifier& v) {
        return v.variableName == variableName;
    }) != std::end(variable_register);
    return log;
}

bool csv_config::shouldLogInstance(const std::string& instanceName) const
{
    if (variable_register.empty()) return true;

    bool log = std::find_if(variable_register.begin(), variable_register.end(), [instanceName, this](const variable_identifier& v) {
        return v.instanceName == instanceName;
    }) != std::end(variable_register);
    return log;
}

void csv_config::load(const std::filesystem::path& configPath)
{
    if (!std::filesystem::exists(configPath)) {
        throw std::runtime_error("No such file: '" + std::filesystem::absolute(configPath).string() + "'");
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

    const auto root = doc.child("ecos:LogConfig");
    const auto components = root.child("ecos:components");
    for (const auto& instances : components) {
        const auto instanceName = instances.attribute("name").as_string();
        for (const auto& variable : instances) {
            const auto variableName = variable.attribute("name").as_string();
            register_variable({instanceName, variableName});
        }
    }
}

void csv_config::enable_plotting(const std::filesystem::path& plotConfig)
{
    if (!exists(plotConfig)) {
        log::warn("No such file: '{}'", absolute(plotConfig).string());
        return;
    }
    plotConfig_ = absolute(plotConfig);
}

std::string plotScript()
{
    return R"(

import sys

import pandas as pd
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt

fig_id = 0
namespaces = {"ecos": "http://github.com/Ecos-platform/libecos/resources/schema/ChartConfig"}


def make_tag(tag: str) -> str:
    return "{" + namespaces["ecos"] + "}" + tag


def make_time_series(csv, timeseries):
    global fig_id
    t = csv['time']
    plt.figure("figure_{}".format(fig_id))
    fig_id = fig_id+1
    plt.title(timeseries.attrib["title"])
    plt.xlabel("time[s]")
    plt.ylabel(timeseries.attrib["label"])
    for series in timeseries:
        for comp in series:
            comp_name = comp.attrib["name"]
            for variable in comp:
                var_name = variable.attrib["name"]
                identifier = "{}::{}".format(comp_name, var_name)
                m = csv.columns.str.contains(identifier)
                data = csv.loc[:, m]
                plt.plot(t, data, label=csv.columns[m][0])

    plt.legend(loc='upper right')


def make_xy_series(csv, xyseries):

    global fig_id
    plt.figure("figure_{}".format(fig_id))
    fig_id = fig_id+1
    plt.title(xyseries.attrib["title"])
    plt.xlabel(xyseries.attrib["xLabel"])
    plt.ylabel(xyseries.attrib["yLabel"])
    for series in xyseries:
        x = series[0]
        v1 = "{}::{}".format(x.attrib["component"], x.attrib["variable"])
        y = series[1]
        v2 = "{}::{}".format(y.attrib["component"], y.attrib["variable"])
        m1 = csv.columns.str.contains(v1)
        data1 = csv.loc[:, m1]
        m2 = csv.columns.str.contains(v2)
        data2 = csv.loc[:, m2]
        plt.plot(data1, data2, label=series.attrib["name"])

    plt.legend(loc='upper right')


if __name__ == "__main__":
    csvFile = sys.argv[1]
    csv = pd.read_csv(csvFile, delimiter=r",\s+", engine="python")
    config = sys.argv[2]

    tree = ET.parse(config)
    root = tree.getroot()

    for chart in root:
        for seriesChoice in chart:
            if seriesChoice.tag == make_tag("timeseries"):
                make_time_series(csv, seriesChoice)
            elif seriesChoice.tag == make_tag("xyseries"):
                make_xy_series(csv, seriesChoice)
                pass
            else:
                pass
    plt.show()
)";
}
