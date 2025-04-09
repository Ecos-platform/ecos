
#include "ecos/util/plotter.hpp"

#include "ecos/logger/logger.hpp"

#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

using namespace ecos;

std::string plotScript();

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
