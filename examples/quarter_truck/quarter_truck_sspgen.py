from ecospy import *
from ecospy.plotter import *
from os import path


def main():
    print(f"Ecoslib version: {EcosLib().version()}")

    EcosLib().set_log_level("debug")

    sspDir = f"{__file__}/../../../data/ssp/quarter_truck/"
    sspFile = f"{sspDir}/quarter_truck_sspgen.ssp"
    resultFile = f"{__file__}/../results/python/quarter_truck_sspgen.csv"

    if not path.isfile(sspFile):
        print(f"Error: No such file '{sspFile}'. Aborting. Have you invoked sspgen?")
        return

    sim = EcosSimulation(ssp_path=f"{sspDir}/quarter_truck_sspgen.ssp", step_size=1.0 / 100)

    sim.add_csv_writer(resultFile, f"{sspDir}/LogConfig.xml")

    sim.init(parameter_set="initialValues")

    sim.step_until(time_point=10)

    sim.terminate()
    sim.destroy()

    config = TimeSeriesConfig(
        title="Quarter-truck",
        y_label="Height[m]",
        identifiers=["chassis::zChassis", "wheel::zWheel", "ground::zGround"])
    plotter = Plotter(resultFile, config)
    plotter.show()


if __name__ == "__main__":
    main()
