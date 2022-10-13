
import pandas as pd
import matplotlib.pyplot as plt


class TimeSeriesConfig:

    def __init__(self, title, ylabel, identifiers):
        self.title = title
        self.ylabel = ylabel
        self.identifiers = identifiers


class XYSeriesConfig:

    def __init__(self, title, xlabel, ylabel, series):
        self.title = title
        self.xlabel = xlabel
        self.ylabel = ylabel
        self.series = series


class Plotter:

    fig_id = 0

    def __init__(self, csvFile, config):
        self.csv = pd.read_csv(csvFile, delimiter=r",\s+", engine="python")

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
        plt.ylabel(timeseries.ylabel)
        for identifier in timeseries.identifiers:
            m = csv.columns.str.contains(identifier)
            data = csv.loc[:, m]
            plt.plot(t, data, label=csv.columns[m][0])

        plt.legend(loc='upper right')

    @staticmethod
    def make_xy_series(csv, xyseries: XYSeriesConfig):

        plt.figure("figure_{}".format(Plotter.fig_id))
        Plotter.fig_id += 1
        plt.title(xyseries.title)
        plt.xlabel(xyseries.xlabel)
        plt.ylabel(xyseries.ylabel)
        for series in xyseries.series:
            name = series[0]
            x = series[1][0]
            y = series[1][1]

            m1 = csv.columns.str.contains(x)
            data1 = csv.loc[:, m1]
            m2 = csv.columns.str.contains(y)
            data2 = csv.loc[:, m2]
            plt.plot(data1, data2, label=name)

        plt.legend(loc='upper right')

