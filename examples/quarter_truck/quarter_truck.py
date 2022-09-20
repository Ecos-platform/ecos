
import os
from ctypes import *


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]


def suffix() -> str:
    if os.name == "nt":
        return ".dll"
    else:
        return ".so"


if __name__ == "__main__":

    path = os.path.abspath(f"libecosc{suffix()}")
    lib = CDLL(path)

    version_fun = lib.ecos_library_version
    version_fun.restype = c_void_p
    version = Version.from_address(lib.ecos_library_version())
    print(f"v{version.major}.{version.minor}.{version.patch}")

    simCreate = lib.ecos_simulation_create
    simCreate.restype = c_void_p
    simCreate.argtypes = [c_char_p, c_double]
    sim = simCreate(b"../../data/ssp/quarter_truck", 1.0/100)

    simAddCsv = lib.ecos_simulation_add_csv_writer
    simAddCsv.argtypes = [c_void_p, c_char_p]
    simAddCsv(sim, b"results/python/quarter_truck.csv")

    simInit = lib.ecos_simulation_init
    simInit.argtypes = [c_void_p, c_double, c_char_p]
    simInit(sim, 0, b"initialValues")

    simStep = lib.ecos_simulation_step
    simStep.argtypes = [c_void_p, c_size_t]
    simStep(sim, 10)

    simTerminate = lib.ecos_simulation_terminate
    simTerminate.argtypes = [c_void_p]
    simTerminate(sim)

    simDestroy = lib.ecos_simulation_destroy
    simDestroy.argtypes = [c_void_p]
    simDestroy(sim)
