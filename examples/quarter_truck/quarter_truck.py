from ecospy import *
from ecospy.plotter import *


class MyListener(SimulationListener):

    def __init__(self, sim: EcosSimulation):
        self.sim = sim

    def pre(self, info: SimulationInfo):
        print(f"pre invoked @ t={info.time}")
        if info.time > 5:
            self.sim.remove_listener(self.name)


def main():
    print(EcosLib().version())

    EcosLib().set_log_level("debug")

    sspDir = f"{__file__}/../../../data/ssp/quarter_truck/"
    resultFile = f"{__file__}/../results/python/quarter_truck.csv"

    sim = EcosSimulation(f"{sspDir}/quarter-truck.ssp", 1.0 / 100)

    sim.add_csv_writer(resultFile, f"{sspDir}/LogConfig.xml")
    sim.add_listener("custom_listener", MyListener(sim))

    sim.init("initialValues")

    sim.step_until(10)

    sim.terminate()
    sim.destroy()

    config = TimeSeriesConfig("Quarter-truck", "Height[m]",
                                      ["chassis::zChassis", "wheel::zWheel", "ground::zGround"])
    plotter = Plotter(resultFile, config)
    plotter.show()


if __name__ == "__main__":
    main()
