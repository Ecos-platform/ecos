import math
import os, requests

from ecospy import *
from ecospy.EcosParameterSet import EcosParameterSet
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
    ss = EcosSimulationStructure()

    ss.add_model("vesselModel", f"{model_folder}/VesselFmu2.fmu")
    ss.add_model("azimuth0", f"proxyfmu://localhost?file={model_folder}/PMAzimuth.fmu")
    ss.add_model("azimuth1", f"proxyfmu://localhost?file={model_folder}/PMAzimuth.fmu")
    ss.add_model("azimuth0_rpmActuator", f"proxyfmu://localhost?file={model_folder}/ThrusterDrive2.fmu")
    ss.add_model("azimuth1_rpmActuator", f"proxyfmu://localhost?file={model_folder}/ThrusterDrive2.fmu")
    ss.add_model("powerPlant", f"proxyfmu://localhost?file={model_folder}/PowerPlant.fmu")
    ss.add_model("trackController", f"{model_folder}/TrajectoryController.fmu")
    ss.add_model("wpProvider", f"{model_folder}/WaypointProvider2DOF.fmu")

    ss.make_real_connection("vesselModel::cgShipMotion.nedDisplacement.north", "trackController::northPosition")
    ss.make_real_connection("vesselModel::cgShipMotion.nedDisplacement.east", "trackController::eastPosition")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.surge", "trackController::surgeVelocity")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.sway", "trackController::swayVelocity")
    ss.make_real_connection("vesselModel::cgShipMotion.angularDisplacement.yaw", "trackController::headingAngle")
    ss.make_real_connection("wpProvider::targetWP.north", "trackController::targetWP.north")
    ss.make_real_connection("wpProvider::targetWP.east", "trackController::targetWP.east")
    ss.make_real_connection("wpProvider::targetWP.speed", "trackController::targetWP.speed")
    ss.make_real_connection("wpProvider::prevWP.north", "trackController::prevWP.north")
    ss.make_real_connection("wpProvider::prevWP.east", "trackController::prevWP.east")
    ss.make_real_connection("wpProvider::prevWP.speed", "trackController::prevWP.speed")
    
    ss.make_real_connection("vesselModel::cgShipMotion.nedDisplacement.north", "wpProvider::northPosition")
    ss.make_real_connection("vesselModel::cgShipMotion.nedDisplacement.east", "wpProvider::eastPosition")
    ss.make_real_connection("vesselModel::cgShipMotion.angularDisplacement.yaw", "wpProvider::headingAngle")
    
    ss.make_real_connection("azimuth0_rpmActuator::d_in.f", "powerPlant::p1.f[1]")
    ss.make_real_connection("azimuth0_rpmActuator::q_in.f", "powerPlant::p1.f[2]")
    ss.make_real_connection("azimuth1_rpmActuator::d_in.f", "powerPlant::p2.f[1]")
    ss.make_real_connection("azimuth1_rpmActuator::q_in.f", "powerPlant::p2.f[2]")
    
    ss.make_real_connection("powerPlant::p1.e[1]", "azimuth0_rpmActuator::d_in.e")
    ss.make_real_connection("powerPlant::p1.e[2]", "azimuth0_rpmActuator::q_in.e")
    ss.make_real_connection("trackController::forceCommand", "azimuth0_rpmActuator::ThrustCom")
    ss.make_real_connection("azimuth0::output_torque", "azimuth0_rpmActuator::Shaft.e")
    
    ss.make_real_connection("powerPlant::p2.e[1]", "azimuth1_rpmActuator::d_in.e")
    ss.make_real_connection("powerPlant::p2.e[2]", "azimuth1_rpmActuator::q_in.e")
    ss.make_real_connection("trackController::forceCommand", "azimuth1_rpmActuator::ThrustCom")
    ss.make_real_connection("azimuth1::output_torque", "azimuth1_rpmActuator::Shaft.e")
    
    ss.make_real_connection("azimuth0_rpmActuator::Shaft.f", "azimuth0::input_act_revs", lambda val: val * 60 / (2*math.pi))
    ss.make_real_connection("trackController::rudderCommand", "azimuth0::input_act_angle")
    ss.make_real_connection("vesselModel::cg_x_rel_ap", "azimuth0::input_cg_x_rel_ap")
    ss.make_real_connection("vesselModel::cg_y_rel_cl", "azimuth0::input_cg_y_rel_cl")
    ss.make_real_connection("vesselModel::cg_z_rel_bl", "azimuth0::input_cg_z_rel_bl")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.surge", "azimuth0::input_cg_surge_vel")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.sway", "azimuth0::input_cg_sway_vel")
    ss.make_real_connection("vesselModel::cgShipMotion.angularVelocity.yaw", "azimuth0::input_yaw_vel")
    
    ss.make_real_connection("azimuth1_rpmActuator::Shaft.f", "azimuth1::input_act_revs", lambda val: val * 60 / (2*math.pi))
    ss.make_real_connection("trackController::rudderCommand", "azimuth1::input_act_angle")
    ss.make_real_connection("vesselModel::cg_x_rel_ap", "azimuth1::input_cg_x_rel_ap")
    ss.make_real_connection("vesselModel::cg_y_rel_cl", "azimuth1::input_cg_y_rel_cl")
    ss.make_real_connection("vesselModel::cg_z_rel_bl", "azimuth1::input_cg_z_rel_bl")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.surge", "azimuth1::input_cg_surge_vel")
    ss.make_real_connection("vesselModel::cgShipMotion.linearVelocity.sway", "azimuth1::input_cg_sway_vel")
    ss.make_real_connection("vesselModel::cgShipMotion.angularVelocity.yaw", "azimuth1::input_yaw_vel")
    
    ss.make_real_connection("azimuth0::output_force_heave", "vesselModel::additionalBodyForce[0].force.heave")
    ss.make_real_connection("azimuth0::output_force_surge", "vesselModel::additionalBodyForce[0].force.surge")
    ss.make_real_connection("azimuth0::output_force_sway", "vesselModel::additionalBodyForce[0].force.sway")
    ss.make_real_connection("azimuth0::output_x_rel_ap", "vesselModel::additionalBodyForce[0].pointOfAttackRel2APAndBL.xpos")
    ss.make_real_connection("azimuth0::output_y_rel_cl", "vesselModel::additionalBodyForce[0].pointOfAttackRel2APAndBL.ypos")
    ss.make_real_connection("azimuth0::output_z_rel_bl", "vesselModel::additionalBodyForce[0].pointOfAttackRel2APAndBL.zpos")
    
    ss.make_real_connection("azimuth1::output_force_heave", "vesselModel::additionalBodyForce[1].force.heave")
    ss.make_real_connection("azimuth1::output_force_surge", "vesselModel::additionalBodyForce[1].force.surge")
    ss.make_real_connection("azimuth1::output_force_sway", "vesselModel::additionalBodyForce[1].force.sway")
    ss.make_real_connection("azimuth1::output_x_rel_ap", "vesselModel::additionalBodyForce[1].pointOfAttackRel2APAndBL.xpos")
    ss.make_real_connection("azimuth1::output_y_rel_cl", "vesselModel::additionalBodyForce[1].pointOfAttackRel2APAndBL.ypos")
    ss.make_real_connection("azimuth1::output_z_rel_bl", "vesselModel::additionalBodyForce[1].pointOfAttackRel2APAndBL.zpos")
    

    params = EcosParameterSet()
    params.add_bool("vesselModel::additionalBodyForce[0].enabled", True)
    params.add_bool("vesselModel::additionalBodyForce[1].enabled", True)
    params.add_string("vesselModel::vesselZipFile", "%fmu%/resources/ShipModel-gunnerus-elongated.zip")

    params.add_real("azimuth0::input_x_rel_ap", 1.5)
    params.add_real("azimuth0::input_y_rel_cl", -2.7)
    params.add_real("azimuth0::input_z_rel_bl", 0)
    params.add_real("azimuth0::input_prop_diam", 1.9)
    params.add_real("azimuth0::input_distancetohull", 1.5)
    params.add_real("azimuth0::input_bilgeradius", 3)
    params.add_real("azimuth0::input_rho", 1025)
    params.add_real("azimuth0::input_lpp", 33.9)

    params.add_real("azimuth1::input_x_rel_ap", 1.5)
    params.add_real("azimuth1::input_y_rel_cl", 2.7)
    params.add_real("azimuth1::input_z_rel_bl", 0)
    params.add_real("azimuth1::input_prop_diam", 1.9)
    params.add_real("azimuth1::input_distancetohull", 1.5)
    params.add_real("azimuth1::input_bilgeradius", 3)
    params.add_real("azimuth1::input_rho", 1025)
    params.add_real("azimuth1::input_lpp", 33.9)

    params.add_bool("trackController::enable", False)
    params.add_bool("trackController::shouldLog", False)
    params.add_real("trackController::autopilot.heading.kp", 0.5)
    params.add_real("trackController::autopilot.heading.ki", 0)
    params.add_real("trackController::autopilot.heading.kd", 2)
    params.add_real("trackController::autopilot.speed.kp", 1000)
    params.add_real("trackController::autopilot.speed.ki", 100)
    params.add_real("trackController::autopilot.speed.kd", 500)
    params.add_real("trackController::lookaheadDistance", 50)

    params.add_bool("wpProvider::shouldLog", False)
    params.add_real("wpProvider::wpSwitchDistance", 20)

    ss.add_parameter_set("initialValues", params)

    with (EcosSimulation(structure=ss, step_size=0.05)) as sim:

        sim.add_csv_writer(result_file, "LogConfig.xml")
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
