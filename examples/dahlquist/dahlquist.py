from ecospy import *
from ecospy.plotter import *

from pathlib import Path

def kelvin_to_deg(value: float) -> float:
    return value - 273.15

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'Dahlquist.fmu').resolve())
    result_file = "results/python/results.csv"

    ss = EcosSimulationStructure()
    ss.add_model("model", fmu_path)

    ss.add_parameter_set("initials", {
        "model::k": 2.0
    })

    with (EcosSimulation(structure=ss, step_size=1/10)) as sim:

        del ss

        sim.add_csv_writer(result_file)

        sim.init(parameter_set="initials")

        sim.step_until(10)
        sim.terminate()

    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Temperature[deg]",
        identifiers=["model::x", "model::der(x)"],

    )
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()