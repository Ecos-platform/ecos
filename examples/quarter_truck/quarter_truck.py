
from ecospy import *
from ecospy import plotter

if __name__ == "__main__":

    print_version()

    set_log_level("debug")

    sspDir = f"{__file__}/../../../data/ssp/quarter_truck/"
    resultFile = f"{__file__}/../results/python/quarter_truck.csv"

    sim = EcosSimulation(f"{sspDir}/quarter-truck.ssp", 1.0/100)
    sim.add_csv_writer(resultFile, f"{sspDir}/LogConfig.xml")

    def pre(t: float):
        print(f"pre step at t={t}")

    sim.add_listener("custom_listener", pre)

    sim.init("initialValues")

    sim.step_until(10)

    sim.terminate()
    sim.destroy()

    config = plotter.TimeSeriesConfig("Quarter-truck", "Height[m]", ["chassis::zChassis", "wheel::zWheel", "ground::zGround"])
    plotter = plotter.Plotter(resultFile, config)
    plotter.show()
