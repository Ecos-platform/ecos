from ecospy import *
from ecospy.plotter import *

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = f"{__file__}/../../../data/fmus/2.0/20sim/ControlledTemperature.fmu"
    result_file = "results/python/temperature.csv"

    ss = EcosSimulationStructure()
    if not ss.add_model("model", fmu_path):
        raise Exception(EcosLib.get_last_error())

    with (EcosSimulation(structure=ss, step_size=1/100)) as sim:

        del ss

        sim.add_csv_writer(result_file)
        sim.init()
        sim.step_until(40)
        sim.terminate()

    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Temperature[kelvin]",
        identifiers=["model::Temperature_Reference", "model::Temperature_Room"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()