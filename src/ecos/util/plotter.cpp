
#include "ecos/util/plotter.hpp"

#include "util/temp_dir.hpp"

#include "ecos/logger/logger.hpp"

#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

using namespace ecos;

namespace
{

// --- XML Serialization Helpers ---

std::string escape(const std::string& str)
{
    std::string out;
    for (char c : str) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '\"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += c;
        }
    }
    return out;
}

void indent(std::ostream& os, int level)
{
    os << std::string(level * 2, ' ');
}

void writeVariableIdentifier(std::ostream& os, const std::string& tag, const variable_identifier& vi, int indentLevel)
{
    indent(os, indentLevel);
    os << "<" << tag
       << " component=\"" << escape(vi.instanceName) << "\""
       << " variable=\"" << escape(vi.variableName) << "\"/>\n";
}

void writeXYSeries(std::ostream& os, const TXYSeries& series, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:series name=\"" << escape(series.name) << "\"";
    if (series.marker)
        os << " marker=\"" << escape(*series.marker) << "\"";
    os << ">\n";

    writeVariableIdentifier(os, "x", series.x, indentLevel + 1);
    writeVariableIdentifier(os, "y", series.y, indentLevel + 1);

    indent(os, indentLevel);
    os << "</ecos:series>\n";
}

void writeXySeriesChart(std::ostream& os, const TXYSeriesChart& chart, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:xyseries title=\"" << escape(chart.title)
       << "\" xLabel=\"" << escape(chart.xLabel)
       << "\" yLabel=\"" << escape(chart.yLabel) << "\">\n";

    for (const auto& series : chart.series) {
        writeXYSeries(os, series, indentLevel + 1);
    }

    indent(os, indentLevel);
    os << "</ecos:xyseries>\n";
}

void writeLinearTransformation(std::ostream& os, const TLinearTransformation& lt, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:linearTransformation offset=\"" << lt.offset << "\" factor=\"" << lt.factor << "\"/>\n";
}

void writeVariable(std::ostream& os, const TVariable& var, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:variable name=\"" << escape(var.name) << "\">\n";
    if (var.linearTransformation) {
        writeLinearTransformation(os, *var.linearTransformation, indentLevel + 1);
    }
    indent(os, indentLevel);
    os << "</ecos:variable>\n";
}

void writeComponent(std::ostream& os, const TComponent& comp, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:component name=\"" << escape(comp.name) << "\">\n";
    for (const auto& var : comp.variables) {
        writeVariable(os, var, indentLevel + 1);
    }
    indent(os, indentLevel);
    os << "</ecos:component>\n";
}

void writeTimeSeriesChart(std::ostream& os, const TTimeSeriesChart& chart, int indentLevel)
{
    indent(os, indentLevel);
    os << "<ecos:timeseries title=\"" << escape(chart.title)
       << "\" label=\"" << escape(chart.label) << "\">\n";

    indent(os, indentLevel + 1);
    os << "<ecos:series>\n";
    for (const auto& comp : chart.series.components) {
        writeComponent(os, comp, indentLevel + 2);
    }
    indent(os, indentLevel + 1);
    os << "</ecos:series>\n";

    indent(os, indentLevel);
    os << "</ecos:timeseries>\n";
}

void writeChart(std::ostream& os, const TChart& chart, int indentLevel)
{
    switch (chart.type) {
        case TChart::Type::XYSeries:
            writeXySeriesChart(os, std::get<TXYSeriesChart>(chart.data), indentLevel);
            break;
        case TChart::Type::TimeSeries:
            writeTimeSeriesChart(os, std::get<TTimeSeriesChart>(chart.data), indentLevel);
            break;
    }
}

} // namespace

std::string plotScript();

void TChartConfig::addChart(TChart chart)
{
    charts.emplace_back(std::move(chart));
}

std::string TChartConfig::toXML() const
{
    std::ostringstream os;
    os << R"(<?xml version="1.0" encoding="utf-8"?>)"
       << "\n";
    os << R"(<ecos:ChartConfig xmlns:ecos="http://github.com/Ecos-platform/libecos/resources/schema/ChartConfig">)"
       << "\n\n";

    indent(os, 1);
    os << R"(<ecos:chart>)"
       << "\n";
    for (const auto& chart : charts) {
        writeChart(os, chart, 2);
    }

    indent(os, 1);
    os << R"(</ecos:chart>)"
       << "\n\n";

    os << "</ecos:ChartConfig>\n";
    return os.str();
}

void ecos::plot_csv(const std::filesystem::path& csvFile, const std::filesystem::path& plotConfig)
{

    if (!exists(csvFile)) {
        log::warn("No such file: '{}'", absolute(csvFile).string());
        return;
    }

    if (!exists(plotConfig)) {
        log::warn("No such file: '{}'", absolute(plotConfig).string());
        return;
    }

    const std::filesystem::path plotter("ecos_plotter.py");
    if (!exists(plotter)) {
        std::ofstream out(plotter, std::ios::trunc);
        if (!out) {
            log::warn("Failed to write plotter script to '{}'", plotter.string());
            return;
        }
        out << plotScript();
    }

    std::ostringstream ss;
    ss << "python ecos_plotter.py "
       << std::quoted(csvFile.string())
       << " "
       << std::quoted(plotConfig.string());
    auto t = std::thread([&ss] {
        if (int status = std::system(ss.str().c_str())) {
            log::warn("Command {} returned with status: {}", ss.str(), status);
        }
    });
    log::info("Waiting for plotting window(s) to close..");
    t.join();
    log::info("Plotting window(s) closed.");

    std::filesystem::remove(plotter);
}

void ecos::plot_csv(const std::filesystem::path& csvFile, const TChartConfig& config)
{
    const auto xml = config.toXML();
    const temp_dir tmp("chart_config");

    const std::filesystem::path tmpFile = tmp.path() / "ChartConfig.xml";
    {
        std::ofstream out(tmpFile, std::ios::trunc);
        out << xml;
    }
    plot_csv(csvFile, tmpFile);
}

std::string plotScript()
{
    return R"(

import re
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
                m = csv.columns.str.contains(re.escape(identifier))
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
        name = series.attrib["name"]
        marker = series.attrib["marker"] if 'marker' in series.attrib else None
        x = series[0]
        v1 = "{}::{}".format(x.attrib["component"], x.attrib["variable"])
        y = series[1]
        v2 = "{}::{}".format(y.attrib["component"], y.attrib["variable"])
        m1 = csv.columns.str.contains(re.escape(v1))
        data1 = csv.loc[:, m1]
        m2 = csv.columns.str.contains(re.escape(v2))
        data2 = csv.loc[:, m2]

        if marker is None:
            plt.plot(data1, data2, label=name)
        else:
            plt.plot(data1, data2, marker, label=name)

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
