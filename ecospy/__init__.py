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


class ListenerConfig(Structure):
    _fields_ = [("preStepCallback", CFUNCTYPE(None, c_double)), ("postStepCallback", CFUNCTYPE(None, c_double))]


def version():
    version_fun = lib.ecos_library_version
    version_fun.restype = c_void_p
    return Version.from_address(lib.ecos_library_version())


def print_version():
    v = version()
    print(f"v{v.major}.{v.minor}.{v.patch}")


def get_last_error():
    last_error_fun = lib.ecos_last_error_msg
    last_error_fun.restype = c_char_p
    err = last_error_fun()
    return err.decode()


def set_log_level(lvl: str):
    setLogLevel = lib.set_log_level
    setLogLevel.argtypes = [c_char_p]
    setLogLevel(lvl.encode())


class EcosSimulation:

    def __init__(self, ssp_path: str, step_size: float):

        self.__simStep = lib.ecos_simulation_step
        self.__simStep.argtypes = [c_void_p, c_size_t]

        self.__simStepUntil = lib.ecos_simulation_step_until
        self.__simStepUntil.argtypes = [c_void_p, c_double]

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

        self.__setInteger = lib.ecos_simulation_set_integer
        self.__setInteger.argtypes = [c_void_p, c_char_p, c_int]
        self.__setInteger.restype = c_bool

        self.__setReal = lib.ecos_simulation_set_real
        self.__setReal.argtypes = [c_void_p, c_char_p, c_double]
        self.__setReal.restype = c_bool

        self.__setBool = lib.ecos_simulation_set_bool
        self.__setBool.argtypes = [c_void_p, c_char_p, c_bool]
        self.__setBool.restype = c_bool

        self.__setString = lib.ecos_simulation_set_string
        self.__setString.argtypes = [c_void_p, c_char_p, c_char_p]
        self.__setString.restype = c_bool

        self.listener_configs = []

        simCreate = lib.ecos_simulation_create
        simCreate.restype = c_void_p
        simCreate.argtypes = [c_char_p, c_double]
        self.sim = simCreate(ssp_path.encode(), step_size)
        if self.sim is None:
            raise Exception(get_last_error())

    def add_listener(self, name: str, preStep = None, postStep = None):

        if preStep is None and postStep is None:
            return

        config = ListenerConfig()
        self.listener_configs.append(config)

        if preStep is not None:
            @CFUNCTYPE(None, c_double)
            def pre(t: float):
                preStep(t)
            config.preStepCallback = pre

        if postStep is not None:
            @CFUNCTYPE(None, c_double)
            def post(t: float):
                postStep(t)
            config.postStepCallback = post

        createListener = lib.ecos_simulation_listener_create
        createListener.argtypes = [ListenerConfig]
        createListener.restype = c_void_p
        listener = createListener(config)

        simAddListener = lib.ecos_simulation_add_listener
        simAddListener.argtypes = [c_void_p, c_char_p, c_void_p]
        simAddListener(self.sim, name.encode(), listener)

    def add_csv_writer(self, result_file: str, log_config: str = None):
        createCsv = lib.ecos_csv_writer_create
        createCsv.argtypes = [c_char_p, c_char_p, c_char_p]
        createCsv.restype = c_void_p

        listener = createCsv(result_file.encode(), None if log_config is None else log_config.encode(), None)

        if listener is None:
            raise Exception(get_last_error())

        simAddListener = lib.ecos_simulation_add_listener
        simAddListener.argtypes = [c_void_p, c_char_p, c_void_p]
        simAddListener(self.sim, b'csv_writer', listener)

    def get_integer(self, identifier: str):
        val = c_int()
        if not self.__getInteger(self.sim, identifier.encode(), byref(val)):
            raise Exception(get_last_error())
        return val.value

    def get_real(self, identifier: str):
        val = c_double()
        if not self.__getReal(self.sim, identifier.encode(), byref(val)):
            raise Exception(get_last_error())
        return val.value

    def get_bool(self, identifier: str):
        val = c_bool()
        if not self.__getBool(self.sim, identifier.encode(), byref(val)):
            raise Exception(get_last_error())
        return val.value

    def get_string(self, identifier: str):
        val = c_char()
        if not self.__getString(self.sim, identifier.encode(), byref(val)):
            raise Exception(get_last_error())
        return val.value

    def set_integer(self, identifier: str, value: int):
        if not self.__getInteger(self.sim, identifier.encode(), value):
            raise Exception(get_last_error())

    def set_real(self, identifier: str, value: float):
        if not self.__getReal(self.sim, identifier.encode(), value):
            raise Exception(get_last_error())

    def set_bool(self, identifier: str, value: bool):
        if not self.__setBool(self.sim, identifier.encode(), value):
            raise Exception(get_last_error())

    def set_string(self, identifier: str, value: str):
        if not self.__setString(self.sim, identifier.encode(), value.encode()):
            raise Exception(get_last_error())

    def init(self, parameter_set: str = None):
        simInit = lib.ecos_simulation_init
        simInit.argtypes = [c_void_p, c_double, c_char_p]
        simInit.restype = c_bool
        return simInit(self.sim, 0, None if parameter_set is None else parameter_set.encode())

    def step(self, num_steps: int = 1):
        self.__simStep(self.sim, num_steps)

    def step_until(self, time_point: float):
        self.__simStepUntil(self.sim, time_point)

    def terminate(self):
        simTerminate = lib.ecos_simulation_terminate
        simTerminate.argtypes = [c_void_p]
        simTerminate(self.sim)

    def destroy(self):
        simDestroy = lib.ecos_simulation_destroy
        simDestroy.argtypes = [c_void_p]
        simDestroy(self.sim)
