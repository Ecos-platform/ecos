import re

import pandas as pd
import matplotlib.pyplot as plt
from typing import Callable


class TimeSeriesConfig:

    def __init__(self, title: str, y_label: str, identifiers: list[str], modifiers: dict[str, Callable[[any], any]] = None):
        self.title = title
        self.y_label = y_label
        self.identifiers = identifiers
        self.modifiers = modifiers


class XYSeriesConfig:

    def __init__(self, title: str, x_label: str, y_label: str, series: dict[str, tuple[str, str]]):
        self.title = title
        self.x_label = x_label
        self.y_label = y_label
        self.series = series


class Plotter:

    fig_id = 0

    def __init__(self, csv_file, configs):
        self.csv = pd.read_csv(csv_file, delimiter=r",\s+", engine="python")

        if not isinstance(configs, (list, tuple)):
            configs = [configs]

        for config in configs:
            if isinstance(config, TimeSeriesConfig):
                self.make_time_series(self.csv, config)
            elif isinstance(config, XYSeriesConfig):
                self.make_xy_series(self.csv, config)
            else:
                raise Exception("Missing or unsupported configuration")

    @staticmethod
    def show():
        plt.show()

    @staticmethod
    def make_time_series(csv, timeseries: TimeSeriesConfig):

        t = csv['time']
        plt.figure("figure_{}".format(Plotter.fig_id))
        Plotter.fig_id += 1
        plt.title(timeseries.title)
        plt.xlabel("time[s]")
        plt.ylabel(timeseries.y_label)
        for identifier in timeseries.identifiers:
            m = csv.columns.str.contains(re.escape(identifier))
            data = csv.loc[:, m]
            if timeseries.modifiers is not None and identifier in timeseries.modifiers:
                data = data.map(timeseries.modifiers[identifier])

            plt.plot(t, data, label=csv.columns[m][0])

        plt.legend(loc='upper right')

    @staticmethod
    def make_xy_series(csv, xyseries: XYSeriesConfig):

        plt.figure("figure_{}".format(Plotter.fig_id))
        Plotter.fig_id += 1
        plt.title(xyseries.title)
        plt.xlabel(xyseries.x_label)
        plt.ylabel(xyseries.y_label)
        for name in xyseries.series:
            xy = xyseries.series[name]
            x = xy[0]
            y = xy[1]
            marker = None if len(xy) < 3 else xy[2]

            m1 = csv.columns.str.contains(re.escape(x))
            data1 = csv.loc[:, m1]
            m2 = csv.columns.str.contains(re.escape(y))
            data2 = csv.loc[:, m2]
            if marker is None:
                plt.plot(data1, data2, label=name)
            else:
                plt.plot(data1, data2, marker, label=name)

        plt.legend(loc='upper right')

