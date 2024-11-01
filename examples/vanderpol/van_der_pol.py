from ecospy import *
from ecospy.plotter import Plotter, XYSeriesConfig

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = f"{__file__}/../../../data/fmus/3.0/ref/VanDerPol.fmu"
    result_file = f"results/python/VanDerPol.csv"

    ss = EcosSimulationStructure()
    if not ss.add_model("model", fmu_path):
        raise Exception(EcosLib.get_last_error())

    with(EcosSimulation(structure=ss, step_size=1/100)) as sim:

        del ss

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
