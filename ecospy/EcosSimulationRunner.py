from .lib import dll, EcosLib
from .EcosSimulation import EcosSimulation

from ctypes import c_void_p, c_double


class EcosSimulationRunner:

    def __init__(self, sim: EcosSimulation):

        _simulation_runner_create = dll.ecos_simulation_runner_create
        _simulation_runner_create.restype = c_void_p
        _simulation_runner_create.argtypes = [c_void_p]

        self._handle = _simulation_runner_create(sim.sim)
        if self._handle is None:
            raise Exception(EcosLib.get_last_error())

    def start(self):
        _simulation_runner_start = dll.ecos_simulation_runner_start
        _simulation_runner_start.argtypes = [c_void_p]
        _simulation_runner_start(self._handle)

    def stop(self):
        _simulation_runner_stop = dll.ecos_simulation_runner_stop
        _simulation_runner_stop.argtypes = [c_void_p]
        _simulation_runner_stop(self._handle)

    def set_real_time_factor(self, factor: float):
        _simulation_runner_set_real_time_factor = dll.ecos_simulation_runner_set_real_time_factor
        _simulation_runner_set_real_time_factor.argtypes = [c_void_p, c_double]
        _simulation_runner_set_real_time_factor(self._handle, factor)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def free(self):
        if not self._handle is None:
            destroy_simulation_runner = dll.ecos_simulation_runner_destroy
            destroy_simulation_runner.argtypes = [c_void_p]
            destroy_simulation_runner(self._handle)
            self._handle = None

    def __del__(self):
        self.free()
