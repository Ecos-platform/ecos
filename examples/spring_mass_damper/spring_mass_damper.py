
from ecospy import *
from ecospy.plotter import *

def main():

    fmuFolder = f"{__file__}/../../../data/fmus/1.0"
    result_file = f"{__file__}/../results/python/bouncing_ball.csv"

    ss = EcosSimulationStructure()
    ss.add_model("damper", f"proxyfmu://localhost?file={fmuFolder}/Damper.fmu")
    ss.add_model("mass", f"proxyfmu://localhost?file={fmuFolder}/Mass.fmu")
    ss.add_model("spring", f"proxyfmu://localhost?file={fmuFolder}/Spring.fmu")

    ss.make_real_connection("spring::for_xx", "mass::in_l_u")
    ss.make_real_connection("spring::for_yx", "mass::in_l_w")
    ss.make_real_connection("mass::out_l_u", "spring::dis_xx")
    ss.make_real_connection("mass::out_l_w", "spring::dis_yx")
    ss.make_real_connection("damper::df_0", "mass::in_f_u")
    ss.make_real_connection("damper::df_1", "mass::in_f_w")
    ss.make_real_connection("mass::out_f_u", "damper::lv_0")
    ss.make_real_connection("mass::out_f_w", "damper::lv_1")
    
    sim = EcosSimulation(structure=ss, step_size=1.0/100)
    sim.add_csv_writer(result_file)

    sim.init()
    sim.step_until(80)
    sim.terminate()

    config = TimeSeriesConfig(
        title="Mass-spring-damper",
        y_label="Height[m]",
        identifiers=["mass::out_f_w"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()