
import os
from ctypes import *


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]


def suffix() -> str:
    if os.name == "nt":
        return ".dll"
    else:
        return ".so"


def print_version(lib):
    version_fun = lib.ecos_library_version
    version_fun.restype = c_void_p
    version = Version.from_address(lib.ecos_library_version())
    print(f"v{version.major}.{version.minor}.{version.patch}")


def create_sim(lib):
    simCreate = lib.ecos_simulation_create
    simCreate.restype = c_void_p
    simCreate.argtypes = [c_char_p, c_double]
    sim = simCreate(b"../../data/ssp/quarter_truck", 1.0/100)

    simAddCsv = lib.ecos_simulation_add_csv_writer
    simAddCsv.argtypes = [c_void_p, c_char_p]
    simAddCsv(sim, b"results/python/quarter_truck.csv")

    return sim


def init_sim(lib, sim):
    simInit = lib.ecos_simulation_init
    simInit.argtypes = [c_void_p, c_double, c_char_p]
    simInit(sim, 0, b"initialValues")


def step_sim(lib, sim, numSteps: int):
    simStep = lib.ecos_simulation_step
    simStep.argtypes = [c_void_p, c_size_t]
    simStep(sim, numSteps)


def terminate_sim(lib, sim):
    simTerminate = lib.ecos_simulation_terminate
    simTerminate.argtypes = [c_void_p]
    simTerminate(sim)


def destroy_sim(lib, sim):
    simDestroy = lib.ecos_simulation_destroy
    simDestroy.argtypes = [c_void_p]
    simDestroy(sim)


if __name__ == "__main__":

    path = os.path.abspath(f"libecosc{suffix()}")
    lib = CDLL(path)

    print_version(lib)

    sim = create_sim(lib)
    init_sim(lib, sim)
    step_sim(lib, sim, 10)
    terminate_sim(lib, sim)
    destroy_sim(lib, sim)
