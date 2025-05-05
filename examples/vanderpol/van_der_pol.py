from ecospy import *
from ecospy.plotter import Plotter, XYSeriesConfig

from pathlib import Path

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'VanDerPol.fmu').resolve())
    result_file = f"results/python/VanDerPol.csv"

    with EcosSimulationStructure() as ss:
        ss.add_model("model", fmu_path)

        with(EcosSimulation(structure=ss, step_size=1/100)) as sim:

            sim.add_csv_writer(result_file)
            sim.init()
            mu = 0.01
            while mu < 5:
                sim.set_real("model::mu", mu)
                sim.step(1000)
                mu += mu / 5
            sim.terminate()

    config = XYSeriesConfig(
        title="Van der Pol Oscillator",
        y_label="Position (x)",
        x_label="Velocity (y)",
        series={"test": ("model::x0", "model::der(x0)")})
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
