from ecospy import *
from ecospy.plotter import *

def main():
    EcosLib.set_log_level("debug")

    ssp_dir = f"{__file__}/../../../data/ssp/dp_ship"
    log_config = f"{ssp_dir}/LogConfig.xml"
    scenario = f"{ssp_dir}/waypoints_scenario.xml"

    result_file = "results/dp_ship.csv"

    with(EcosSimulation(ssp_path=ssp_dir, step_size=0.1)) as sim:

        sim.add_csv_writer(result_file, log_config)
        if not sim.load_scenario(scenario):
            print(EcosLib.get_last_error())

        sim.init()
        sim.step_for(1000)

        sim.terminate()

    config = XYSeriesConfig(
        title="Position and reference",
        y_label="Position (north)",
        x_label="Position (east)",
        series={
            "actual": ("Ship::q[1]", "Ship::q[2]"),
            "ref": ("Reference Generator::x_wp", "Reference Generator::y_wp", 'o')
        })
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
