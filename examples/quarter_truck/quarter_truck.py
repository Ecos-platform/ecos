from ecospy import *
from ecospy.plotter import *

from pathlib import Path

class MyListener(SimulationListener):

    def __init__(self, sim: EcosSimulation):
        self.sim = sim

    def pre(self, info: SimulationInfo):
        print(f"pre invoked @ t={info.time}")
        if info.time > 5:
            self.sim.remove_listener(self.name)
            print(f"Removed listener at t={info.time}")


def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    ssp_dir = (Path(__file__).parent.parent.parent / 'data' / 'ssp' / 'quarter_truck').resolve()
    result_file = f"results/python/quarter_truck.csv"

    with (EcosSimulation(ssp_path=f"{ssp_dir}", step_size=1.0 / 100)) as sim:

        sim.add_csv_writer(result_file, f"{ssp_dir}/LogConfig.xml")
        sim.add_listener("custom_listener", MyListener(sim))

        sim.init(parameter_set="initialValues")
        sim.step_until(time_point=10)
        sim.terminate()

    config = TimeSeriesConfig(
        title="Quarter-truck",
        y_label="Height[m]",
        identifiers=["chassis::zChassis", "wheel::zWheel", "ground::zGround"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
