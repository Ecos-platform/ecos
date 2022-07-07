
import sys

import pandas as pd
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt

fig_id = 0
namespaces = {"vico": "http://github.com/Vico-platform/libvico/resources/schema/ChartConfig"}


def make_vico_tag(tag: str) -> str:
    return "{" + namespaces["vico"] + "}" + tag


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
            if seriesChoice.tag == make_vico_tag("timeseries"):
                make_time_series(csv, seriesChoice)
            elif seriesChoice.tag == make_vico_tag("xyseries"):
                make_xy_series(csv, seriesChoice)
                pass
            else:
                pass
    plt.show()

