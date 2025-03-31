
import os, requests

from ecospy import *
from ecospy.plotter import *

def download_file(url, filename):

    # Check if the file exists and is not empty
    if os.path.exists(filename) and os.path.getsize(filename) > 0:
        print(f"File '{filename}' already exists and is not empty. Doing nothing.")
        return  # Do nothing if the file exists and is not empty

    # Create the folder if it doesn't exist
    if not os.path.exists(os.path.dirname(filename)):
        os.makedirs(os.path.dirname(filename))
        print(f"Created folder: {filename}")

    try:
        response = requests.get(url)
        response.raise_for_status()  # Check for HTTP errors
        with open(filename, 'wb') as file:
            file.write(response.content)
        print(f"File downloaded successfully: {filename}")
    except requests.exceptions.RequestException as e:
        print(f"Error downloading file: {e}")

class MyListener(SimulationListener):

    def __init__(self, sim: EcosSimulation):
        self.sim = sim
        self.reset = False

    def pre(self, info: SimulationInfo):
        if info.time > 60:
            if not self.reset:
                self.reset = True
                self.sim.set_bool("vesselModel::reset_position", True)
            else:
                self.sim.set_bool("vesselModel::reset_position", False)
                self.sim.set_bool("trackController::enable", True)
                self.sim.remove_listener(self.name)


def download_models(model_folder: str):
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/VesselFmu2.fmu", f"{model_folder}/VesselFmu2.fmu")
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/PMAzimuth.fmu", f"{model_folder}/PMAzimuth.fmu")
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/ThrusterDrive2.fmu", f"{model_folder}/ThrusterDrive2.fmu")
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/PowerPlant.fmu", f"{model_folder}/PowerPlant.fmu")
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/TrajectoryController.fmu", f"{model_folder}/TrajectoryController.fmu")
    download_file("https://raw.githubusercontent.com/gunnerus-case/gunnerus-fmus-bin/master/WaypointProvider2DOF.fmu", f"{model_folder}/WaypointProvider2DOF.fmu")


def main():

    model_folder = "models"
    download_models(model_folder)

    print(f"Ecoslib version: {EcosLib.version()}")
    EcosLib.set_log_level("debug")

    result_file = f"results/gunnerus_trajectory.csv"


    with (EcosSimulation(ssp_path=".", step_size=0.05)) as sim:

        sim.add_csv_writer(result_file, "CsvConfig.xml")
        sim.add_listener("custom_listener", MyListener(sim))

        sim.init(parameter_set="initialValues")
        sim.step_until(time_point=1000)
        sim.terminate()

    config = XYSeriesConfig(
        title="Gunnerus trajectory demo",
        x_label="Position [east]",
        y_label="Position [north]",
        series={
            "path": ("vesselModel::cgShipMotion.nedDisplacement.east", "vesselModel::cgShipMotion.nedDisplacement.north"),
            "target": ("wpProvider::targetWP.east", "wpProvider::targetWP.north", "x")
        })
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
