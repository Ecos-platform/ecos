from ecospy import *
from ecospy.plotter import *


def main():
    print(f"Ecoslib version: {EcosLib().version()}")

    EcosLib().set_log_level("debug")

    ssp_dir = f"{__file__}/../../../data/ssp/quarter_truck/"
    ssp_file = f"{ssp_dir}/quarter-truck.ssp"
    result_file = "results/python/quarter_truck_ssp.csv"

    with(EcosSimulation(ssp_path=ssp_file, step_size=1.0 / 100)) as sim:

        sim.add_csv_writer(result_file, f"{ssp_dir}/LogConfig.xml")

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
