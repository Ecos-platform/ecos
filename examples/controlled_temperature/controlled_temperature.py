from ecospy import *


def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path = f"{__file__}/../../../data/fmus/2.0/20sim/ControlledTemperature.fmu"

    ss = EcosSimulationStructure()
    if not ss.add_model("instance", fmu_path):
        raise Exception(EcosLib.get_last_error())

    sim = EcosSimulation(structure=ss, step_size=1/100)
    sim.add_csv_writer("results/python/temperature.csv")
    sim.init()
    sim.step_until(10)
    sim.terminate()


if __name__ == "__main__":
    main()