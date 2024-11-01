
from ecospy import *

def main():

    fmuFolder = f"{__file__}/../../../data/fmus/1.0"

    ss = EcosSimulationStructure()
    ss.add_model("damper", f"proxyfmu://localhost?file={fmuFolder}/Damper.fmu")
    ss.add_model("mass", f"proxyfmu://localhost?file={fmuFolder}/Mass.fmu")
    ss.add_model("spring", f"proxyfmu://localhost?file={fmuFolder}/Spring.fmu")



    sim = EcosSimulation(structure=ss, step_size=1.0/100)
    sim.init()
    sim.step_until(5)
    sim.terminate()


if __name__ == "__main__":
    main()