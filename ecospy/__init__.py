import os
import os.path
from ctypes import *


def __loadlib():

    def suffix() -> str:
        if os.name == "nt":
            return ".dll"
        else:
            return ".so"

    return CDLL(f"{__file__}/../build/bin/libecosc{suffix()}")


lib = __loadlib()


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]


def version():
    version_fun = lib.ecos_library_version
    version_fun.restype = c_void_p
    return Version.from_address(lib.ecos_library_version())


def print_version():
    v = version()
    print(f"v{v.major}.{v.minor}.{v.patch}")


def getLastError():
    last_error_fun = lib.ecos_last_error_msg
    last_error_fun.restype = c_char_p
    err = last_error_fun()
    return err.decode()


class EcosSimulation:

    def __init__(self, sspPath: str, stepSize: float):
        simCreate = lib.ecos_simulation_create
        simCreate.restype = c_void_p
        simCreate.argtypes = [c_char_p, c_double]
        self.sim = simCreate(sspPath.encode(), stepSize)
        if not self.sim:
            raise Exception(getLastError())

    def init(self, parameterSet: str = None):
        simInit = lib.ecos_simulation_init
        simInit.argtypes = [c_void_p, c_double, c_char_p]
        if parameterSet is None:
            simInit(self.sim, 0, None)
        else:
            simInit(self.sim, 0, parameterSet.encode())

    def step(self, numSteps: int = 1):
        simStep = lib.ecos_simulation_step
        simStep.argtypes = [c_void_p, c_size_t]
        simStep(self.sim, numSteps)

    def terminate(self):
        simTerminate = lib.ecos_simulation_terminate
        simTerminate.argtypes = [c_void_p]
        simTerminate(self.sim)

    def destroy(self):
        simDestroy = lib.ecos_simulation_destroy
        simDestroy.argtypes = [c_void_p]
        simDestroy(self.sim)
