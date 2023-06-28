
import pandas as pd
import matplotlib.pyplot as plt


class TimeSeriesConfig:

    def __init__(self, title: str, y_label: str, identifiers: list[str]):
        self.title = title
        self.y_label = y_label
        self.identifiers = identifiers


class XYSeriesConfig:

    def __init__(self, title: str, x_label: str, y_label: str, series):
        self.title = title
        self.x_label = x_label
        self.y_label = y_label
        self.series = series


class Plotter:

    fig_id = 0

    def __init__(self, csv_file, config):
        self.csv = pd.read_csv(csv_file, delimiter=r",\s+", engine="python")

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
            m = csv.columns.str.contains(identifier)
            data = csv.loc[:, m]
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

            m1 = csv.columns.str.contains(x)
            data1 = csv.loc[:, m1]
            m2 = csv.columns.str.contains(y)
            data2 = csv.loc[:, m2]
            plt.plot(data1, data2, label=name)

        plt.legend(loc='upper right')

