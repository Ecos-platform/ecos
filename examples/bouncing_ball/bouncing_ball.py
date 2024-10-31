from ecospy import *
from ecospy.plotter import Plotter, TimeSeriesConfig

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = f"{__file__}/../../../data/fmus/3.0/ref/BouncingBall.fmu"
    result_file = f"{__file__}/../results/python/quarter_truck.csv"

    ss = EcosSimulationStructure()
    if not ss.add_model("ball", fmu_path):
        raise Exception(EcosLib.get_last_error())

    sim = EcosSimulation(structure=ss, step_size=1/100)
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
