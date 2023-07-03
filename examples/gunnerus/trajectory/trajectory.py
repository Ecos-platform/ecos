from os import path
from ecospy import *
from ecospy.plotter import *


class MyListener(SimulationListener):

    def __init__(self, sim: EcosSimulation):
        self.sim = sim
        self.reset = False

    def pre(self, info: SimulationInfo):
        if info.time > 50:
            if not self.reset:
                self.sim.set_bool("vesselModel::reset_position", True)
                self.reset = True
            else:
                self.sim.set_bool("vesselModel::reset_position", False)
                self.sim.set_bool("trackController::enable", True)
                self.sim.remove_listener(self.name)


def main():

    sspFile = "gunnerus-trajectory.ssp"
    resultFile = "results/python/gunnerus_trajectory.csv"

    if not path.exists(sspFile):
        raise Exception(f"Missing file: {path.abspath(sspFile)}")

    print(EcosLib().version())
    EcosLib().set_log_level("debug")

    sim = EcosSimulation(sspFile, 1.0 / 50)
    sim.add_csv_writer(resultFile, "LogConfig.xml")
    sim.add_listener("custom_listener", MyListener(sim))

    sim.init(parameter_set="initialValues")

    sim.step_until(250)

    sim.terminate()
    sim.destroy()

    config = XYSeriesConfig("Quarter-truck", "North[m]", "East[m]",
                            {"trajectory": ("vesselModel::cgShipMotion.nedDisplacement.east", "vesselModel::cgShipMotion.nedDisplacement.north")})
    plotter = Plotter(resultFile, config)
    plotter.show()


if __name__ == "__main__":
    main()
