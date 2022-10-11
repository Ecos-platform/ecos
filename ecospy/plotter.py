
import pandas as pd
import matplotlib.pyplot as plt


class TimeSeriesConfig:

    def __init__(self, title, ylabel, identifiers):
        self.title = title
        self.ylabel = ylabel
        self.identifiers = identifiers


# class XYSeriesConfig:
#
#     def __init__(self, title, ylabel, identifiers):
#         self.title = title
#         self.ylabel = ylabel
#         self.identifiers = identifiers


class Plotter:

    fig_id = 0

    def __init__(self, csvFile, config):
        self.csv = pd.read_csv(csvFile, delimiter=r",\s+", engine="python")

        if isinstance(config, TimeSeriesConfig):
            self.make_time_series(self.csv, config)
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

    # @staticmethod
    # def make_xy_series(csv, xyseries):
    #
    #     plt.figure("figure_{}".format(Plotter.fig_id))
    #     Plotter.fig_id += 1
    #     plt.title(xyseries.attrib["title"])
    #     plt.xlabel(xyseries.attrib["xLabel"])
    #     plt.ylabel(xyseries.attrib["yLabel"])
    #     for series in xyseries:
    #         x = series[0]
    #         v1 = "{}::{}".format(x.attrib["component"], x.attrib["variable"])
    #         y = series[1]
    #         v2 = "{}::{}".format(y.attrib["component"], y.attrib["variable"])
    #         m1 = csv.columns.str.contains(v1)
    #         data1 = csv.loc[:, m1]
    #         m2 = csv.columns.str.contains(v2)
    #         data2 = csv.loc[:, m2]
    #         plt.plot(data1, data2, label=series.attrib["name"])
    #
    #     plt.legend(loc='upper right')

