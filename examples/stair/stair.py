from ecospy import *
from ecospy.plotter import *


def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = f"{__file__}/../../../data/fmus/3.0/ref/Stair.fmu"
    result_file = "results/python/stair.csv"

    ss = EcosSimulationStructure()
    if not ss.add_model("model", fmu_path):
        raise Exception(EcosLib.get_last_error())

    step = 0.2
    with (EcosSimulation(structure=ss, step_size=step)) as sim:

        del ss

        sim.add_csv_writer(result_file)
        sim.init()
        sim.step_until(10-step)
        sim.terminate()

    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Steps",
        identifiers=["model::counter"]
    )
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()