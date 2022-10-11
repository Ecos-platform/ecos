
from ecospy import *

if __name__ == "__main__":

    print_version()

    sspDir = f"{__file__}/../../../data/ssp/quarter_truck/"

    sim = EcosSimulation(f"{sspDir}/quarter-truck.ssp", 1.0/100)
    sim.add_csv_writer(f"{__file__}/../results/python/quarter_truck.csv", f"{sspDir}/LogConfig.xml", f"{sspDir}/ChartConfig.xml")
    sim.init("initialValues")
    sim.step(500)
    sim.terminate()
    sim.destroy()
