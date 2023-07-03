import os
from ctypes import CDLL, CFUNCTYPE, POINTER, Structure, c_void_p, c_char_p, c_int, c_bool, c_double, c_size_t


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]

    def __repr__(self):
        return f"v{self.major}.{self.minor}.{self.patch}"


class SimulationInfo(Structure):
    _fields_ = [("time", c_double), ("iterations", c_size_t)]


class SimulationListener:

    def pre(self, info: SimulationInfo):
        pass

    def post(self, info: SimulationInfo):
        pass


class ListenerConfig(Structure):
    _fields_ = [
        ("preStepCallback", CFUNCTYPE(None, SimulationInfo)),
        ("postStepCallback", CFUNCTYPE(None, SimulationInfo))
    ]


class _EcosLib:
    _instance = None

    def __init__(self):

        def suffix() -> str:
            return ".dll" if os.name == "nt" else ".so"

        binFolder = f"{__file__}\\..\\build\\bin"
        os.add_dll_directory(binFolder)

        self._handle = CDLL(f"libecosc{suffix()}")

        self._set_log_level = self._handle.set_log_level
        self._set_log_level.argtypes = [c_char_p]

        self._get_last_error_msg = self._handle.ecos_last_error_msg
        self._get_last_error_msg.restype = c_char_p

        self.create_simulation = self._handle.ecos_simulation_create
        self.create_simulation.argtypes = [c_char_p, c_double]
        self.create_simulation.restype = c_void_p

        self.init_simulation = self._handle.ecos_simulation_init
        self.init_simulation.argtypes = [c_void_p, c_double, c_char_p]
        self.init_simulation.restype = c_bool

        self.step_simulation = self._handle.ecos_simulation_step
        self.step_simulation.argtypes = [c_void_p, c_size_t]

        self.step_simulation_until = self._handle.ecos_simulation_step_until
        self.step_simulation_until.argtypes = [c_void_p, c_double]

        self.get_integer = self._handle.ecos_simulation_get_integer
        self.get_integer.argtypes = [c_void_p, c_char_p, POINTER(c_int)]
        self.get_integer.restype = c_bool

        self.get_real = self._handle.ecos_simulation_get_real
        self.get_real.argtypes = [c_void_p, c_char_p, POINTER(c_double)]
        self.get_real.restype = c_bool

        self.get_bool = self._handle.ecos_simulation_get_bool
        self.get_bool.argtypes = [c_void_p, c_char_p, POINTER(c_bool)]
        self.get_bool.restype = c_bool

        self.get_string = self._handle.ecos_simulation_get_string
        self.get_string.argtypes = [c_void_p, c_char_p, c_char_p]
        self.get_string.restype = c_bool

        self.set_integer = self._handle.ecos_simulation_set_integer
        self.set_integer.argtypes = [c_void_p, c_char_p, c_int]
        self.set_integer.restype = c_bool

        self.set_real = self._handle.ecos_simulation_set_real
        self.set_real.argtypes = [c_void_p, c_char_p, c_double]
        self.set_real.restype = c_bool

        self.set_bool = self._handle.ecos_simulation_set_bool
        self.set_bool.argtypes = [c_void_p, c_char_p, c_bool]
        self.set_bool.restype = c_bool

        self.set_string = self._handle.ecos_simulation_set_string
        self.set_string.argtypes = [c_void_p, c_char_p, c_char_p]
        self.set_string.restype = c_bool

        self.create_listener = self._handle.ecos_simulation_listener_create
        self.create_listener.argtypes = [ListenerConfig]
        self.create_listener.restype = c_void_p

        self.create_csv_writer = self._handle.ecos_csv_writer_create
        self.create_csv_writer.argtypes = [c_char_p, c_char_p, c_char_p]
        self.create_csv_writer.restype = c_void_p

        self.add_listener = self._handle.ecos_simulation_add_listener
        self.add_listener.argtypes = [c_void_p, c_char_p, c_void_p]

        self.remove_listener = self._handle.ecos_simulation_remove_listener
        self.remove_listener.argtypes = [c_void_p, c_char_p]

        self.terminate_simulation = self._handle.ecos_simulation_terminate
        self.terminate_simulation.argtypes = [c_void_p]

        self.destroy_simulation = self._handle.ecos_simulation_destroy
        self.destroy_simulation.argtypes = [c_void_p]

    def version(self):
        version_fun = self._handle.ecos_library_version
        version_fun.restype = Version
        return self._handle.ecos_library_version()

    def get_last_error(self):
        err = self._get_last_error_msg()
        return err.decode()

    def set_log_level(self, lvl: str):
        self._set_log_level(lvl.encode())


def EcosLib():
    if _EcosLib._instance is None:
        _EcosLib._instance = _EcosLib()
    return _EcosLib._instance
