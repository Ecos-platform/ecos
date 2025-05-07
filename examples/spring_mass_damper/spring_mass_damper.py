
from ecospy import *
from ecospy.plotter import *

from pathlib import Path

def main():

    EcosLib.set_log_level("debug")

    fmu_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / '1.0' / 'mass_spring_damper').resolve()
    result_file = f"results/spring_mass_damper.csv"

    with EcosSimulationStructure() as ss:
        if True:
            ss.add_model("damper", f"{fmu_folder}/Damper.fmu")
            ss.add_model("mass", f"{fmu_folder}/Mass.fmu")
            ss.add_model("spring", f"{fmu_folder}/Spring.fmu")
        else:
            ss.add_model("damper", f"proxyfmu://localhost?file={fmu_folder}/Damper.fmu")
            ss.add_model("mass", f"proxyfmu://localhost?file={fmu_folder}/Mass.fmu")
            ss.add_model("spring", f"proxyfmu://localhost?file={fmu_folder}/Spring.fmu")

        ss.make_real_connection("spring::for_xx", "mass::in_l_u", lambda val: val) # dummy modifier for test
        ss.make_real_connection("spring::for_yx", "mass::in_l_w")
        ss.make_real_connection("mass::out_l_u", "spring::dis_xx")
        ss.make_real_connection("mass::out_l_w", "spring::dis_yx")
        ss.make_real_connection("damper::df_0", "mass::in_f_u")
        ss.make_real_connection("damper::df_1", "mass::in_f_w")
        ss.make_real_connection("mass::out_f_u", "damper::lv_0")
        ss.make_real_connection("mass::out_f_w", "damper::lv_1")

        params = {
            "spring::springStiffness" : 5.,
            "spring::zeroForceLength": 5.,
            "damper::dampingCoefficient": 6.,
            "mass::initialPositionX": 6.,
            "mass::mediumDensity": 1.
        }

        if not ss.add_parameter_set("initialValues", params):
            print(EcosLib.get_last_error())

        with(EcosSimulation(structure=ss, step_size=1.0/100) as sim):

            sim.add_csv_writer(result_file)

            if not sim.init(parameter_set="initialValues"):
                print(EcosLib.get_last_error())

            sim.step_until(80)
            sim.terminate()

    config = TimeSeriesConfig(
        title="Mass-spring-damper",
        y_label="Height[m]",
        identifiers=["mass::out_l_u"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
