from .lib import dll, EcosLib
from .EcosSimulation import EcosSimulation

from ctypes import c_void_p, c_double


class EcosSimulationRunner:
    """
    This class is used to run a simulation.
    """

    def __init__(self, sim: EcosSimulation):

        _simulation_runner_create = dll.ecos_simulation_runner_create
        _simulation_runner_create.restype = c_void_p
        _simulation_runner_create.argtypes = [c_void_p]

        self._handle = _simulation_runner_create(sim.sim)
        if self._handle is None:
            raise Exception(EcosLib.get_last_error())

    def start(self):
        """
        Starts simulation, which runs in a separate native thread.
        """
        _simulation_runner_start = dll.ecos_simulation_runner_start
        _simulation_runner_start.argtypes = [c_void_p]
        _simulation_runner_start(self._handle)

    def stop(self):
        """
        Stops the simulation. This will stop the simulation thread.
        """
        _simulation_runner_stop = dll.ecos_simulation_runner_stop
        _simulation_runner_stop.argtypes = [c_void_p]
        _simulation_runner_stop(self._handle)

    def set_real_time_factor(self, factor: float):
        """
        Sets the real-time factor for the simulation. This factor determines how fast the simulation should run compared to real time.
        Args:
            factor (float): The real-time factor. A value of 1.0 means real-time, 2.0 means twice as fast, etc.
        """
        _simulation_runner_set_real_time_factor = dll.ecos_simulation_runner_set_real_time_factor
        _simulation_runner_set_real_time_factor.argtypes = [c_void_p, c_double]
        _simulation_runner_set_real_time_factor(self._handle, factor)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def free(self):
        """
        Frees the resources associated with the simulation structure.
        """
        if not self._handle is None:
            destroy_simulation_runner = dll.ecos_simulation_runner_destroy
            destroy_simulation_runner.argtypes = [c_void_p]
            destroy_simulation_runner(self._handle)
            self._handle = None

    def __del__(self):
        self.free()
