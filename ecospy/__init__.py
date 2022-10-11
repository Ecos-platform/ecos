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

        self.__simStep = lib.ecos_simulation_step
        self.__simStep.argtypes = [c_void_p, c_size_t]

        self.__getInteger = lib.ecos_simulation_get_integer
        self.__getInteger.argtypes = [c_void_p, c_char_p, POINTER(c_int)]
        self.__getInteger.restype = c_bool

        self.__getReal = lib.ecos_simulation_get_real
        self.__getReal.argtypes = [c_void_p, c_char_p, POINTER(c_double)]
        self.__getReal.restype = c_bool

        self.__getBool = lib.ecos_simulation_get_bool
        self.__getBool.argtypes = [c_void_p, c_char_p, POINTER(c_bool)]
        self.__getBool.restype = c_bool

        self.__getString = lib.ecos_simulation_get_string
        self.__getString.argtypes = [c_void_p, c_char_p, c_char_p]
        self.__getString.restype = c_bool

        simCreate = lib.ecos_simulation_create
        simCreate.restype = c_void_p
        simCreate.argtypes = [c_char_p, c_double]
        self.sim = simCreate(sspPath.encode(), stepSize)
        if not self.sim:
            raise Exception(getLastError())

    def add_csv_writer(self, resultFile: str, logConfig: str, plotConfig: str) -> bool:
        simAddCsv = lib.ecos_simulation_add_csv_writer
        simAddCsv.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p]
        simAddCsv.restype = c_bool
        return simAddCsv(self.sim, resultFile.encode(),
                  None if logConfig is None else logConfig.encode(),
                  None if plotConfig is None else plotConfig.encode())

    def get_integer(self, identifier: str):
        val = c_int()
        if not self.__getInteger(self.sim, identifier.encode(), byref(val)):
            raise Exception(getLastError())
        return val.value

    def get_real(self, identifier: str):
        val = c_double()
        if not self.__getReal(self.sim, identifier.encode(), byref(val)):
            raise Exception(getLastError())
        return val.value

    def get_bool(self, identifier: str):
        val = c_bool()
        if not self.__getBool(self.sim, identifier.encode(), byref(val)):
            raise Exception(getLastError())
        return val.value

    def get_string(self, identifier: str):
        val = c_char()
        if not self.__getString(self.sim, identifier.encode(), byref(val)):
            raise Exception(getLastError())
        return val.value

    def init(self, parameterSet: str = None):
        simInit = lib.ecos_simulation_init
        simInit.argtypes = [c_void_p, c_double, c_char_p]
        simInit.restype = c_bool
        return simInit(self.sim, 0, None if parameterSet is None else parameterSet.encode())

    def step(self, numSteps: int = 1):
        self.__simStep(self.sim, numSteps)

    def terminate(self):
        simTerminate = lib.ecos_simulation_terminate
        simTerminate.argtypes = [c_void_p]
        simTerminate(self.sim)

    def destroy(self):
        simDestroy = lib.ecos_simulation_destroy
        simDestroy.argtypes = [c_void_p]
        simDestroy(self.sim)
