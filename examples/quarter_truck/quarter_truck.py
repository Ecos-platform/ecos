
from ecospy import *

if __name__ == "__main__":

    print_version()

    sim = EcosSimulation(f"{__file__}/../../../data/ssp/quarter_truck/quarter-truck.ssp", 1.0/100)
    sim.init("initialValues")
    sim.step(500)
    sim.terminate()
    sim.destroy()
