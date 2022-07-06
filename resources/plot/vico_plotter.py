
import sys

import pandas as pd
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt

namespaces = {"vico": "http://github.com/Vico-platform/libvico/resources/schema/ChartConfig"}


def make_vico_tag(tag: str) -> str:
    return "{" + namespaces["vico"] + "}" + tag


def make_time_series(csv, timeseries):
    t = csv['time']
    plt.title(timeseries.attrib["title"])
    plt.xlabel("time[s]")
    plt.ylabel(timeseries.attrib["label"])
    for series in timeseries:
        for comp in series:
            comp_name = comp.attrib["name"]
            for variable in comp:
                var_name = variable.attrib["name"]
                id = "{}.{}[REAL]".format(comp_name, var_name)
                data = csv[id]
                plt.plot(t, data, label=id)

    plt.legend(loc='upper right')
    plt.show()


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
            else:
                pass


