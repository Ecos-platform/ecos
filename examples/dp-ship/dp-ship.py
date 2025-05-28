from ecospy import *
from ecospy.plotter import *

import sys, signal
from pathlib import Path

def signal_handler(sig, frame):
    print("\nSimulation interrupted by user (CTRL+C).")
    sys.exit(0)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    EcosLib.set_log_level("debug")

    ssp_dir = str((Path(__file__).parent.parent.parent / 'data' / 'ssp' / '1.0' / 'dp_ship').resolve())
    log_config = f"{ssp_dir}/CsvConfig.xml"
    scenario = f"{ssp_dir}/waypoints_scenario.xml"

    result_file = "results/dp_ship.csv"

    with(EcosSimulation(ssp_path=ssp_dir, step_size=0.1)) as sim:

        sim.add_csv_writer(result_file, log_config)
        sim.load_scenario(scenario)

        sim.init()
        try:
            print("Press CTRL+C to terminate the simulation.")
            t = 0
            while t < 3000:
                t = sim.step(1)
        except SystemExit:
            print(f"Simulation requested to stop at t={t}")

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
