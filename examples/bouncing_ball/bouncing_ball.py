from ecospy import *
from ecospy.plotter import Plotter, TimeSeriesConfig

from pathlib import Path

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path =  str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'BouncingBall.fmu').resolve())
    result_file = f"results/python/bouncing_ball.csv"

    with EcosSimulationStructure() as ss:
        ss.add_model("ball", fmu_path)

        with(EcosSimulation(structure=ss, step_size=1/100)) as sim:

            sim.add_csv_writer(result_file)
            sim.init()
            sim.step_until(10)
            sim.terminate()

    config = TimeSeriesConfig(
        title="BouncingBall",
        y_label="Height[m]",
        identifiers=["ball::h"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
