from .lib import *
from ctypes import byref, c_char


class EcosSimulation:

    def __init__(self, ssp_path: str, step_size: float):

        self.listener_configs = {}

        self.sim = EcosLib().create_simulation(ssp_path.encode(), step_size)
        if self.sim is None:
            raise Exception(EcosLib().get_last_error())

    def add_listener(self, name: str, listener: SimulationListener):

        if not isinstance(listener, SimulationListener):
            raise Exception("listener must be of type SimulationListener")

        listener.name = name

        config = ListenerConfig()
        self.listener_configs[name] = config

        @CFUNCTYPE(None, SimulationInfo)
        def pre(info: SimulationInfo):
            listener.pre(info)
        config.preStepCallback = pre

        @CFUNCTYPE(None, SimulationInfo)
        def post(info: SimulationInfo):
            listener.post(info)
        config.postStepCallback = post

        cpp_listener = EcosLib().create_listener(config)
        EcosLib().add_listener(self.sim, name.encode(), cpp_listener)

    def remove_listener(self, name: str):
        EcosLib().remove_listener(self.sim, name.encode())

    def add_csv_writer(self, result_file: str, log_config: str = None):

        listener = EcosLib().create_csv_writer(result_file.encode(),
                                               None if log_config is None else log_config.encode(), None)
        if listener is None:
            raise Exception(EcosLib().get_last_error())

        EcosLib().add_listener(self.sim, b'csv_writer', listener)

    def get_integer(self, identifier: str):
        val = c_int()
        if not EcosLib().get_integer(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib().get_last_error())
        return val.value

    def get_real(self, identifier: str):
        val = c_double()
        if not EcosLib().get_real(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib().get_last_error())
        return val.value

    def get_bool(self, identifier: str):
        val = c_bool()
        if not EcosLib().get_bool(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib().get_last_error())
        return val.value

    def get_string(self, identifier: str):
        val = c_char()
        if not EcosLib().get_string(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib().get_last_error())
        return val.value

    def set_integer(self, identifier: str, value: int):
        if not EcosLib().set_integer(self.sim, identifier.encode(), value):
            raise Exception(EcosLib().get_last_error())

    def set_real(self, identifier: str, value: float):
        if not EcosLib().set_real(self.sim, identifier.encode(), value):
            raise Exception(EcosLib().get_last_error())

    def set_bool(self, identifier: str, value: bool):
        if not EcosLib().set_bool(self.sim, identifier.encode(), value):
            raise Exception(EcosLib().get_last_error())

    def set_string(self, identifier: str, value: str):
        if not EcosLib().set_string(self.sim, identifier.encode(), value.encode()):
            raise Exception(EcosLib().get_last_error())

    def init(self, start_time: int = 0, parameter_set: str = None):
        return EcosLib().init_simulation(self.sim, start_time, None if parameter_set is None else parameter_set.encode())

    def step(self, num_steps: int = 1):
        EcosLib().step_simulation(self.sim, num_steps)

    def step_until(self, time_point: float):
        EcosLib().step_simulation_until(self.sim, time_point)

    def terminate(self):
        EcosLib().terminate_simulation(self.sim)

    def destroy(self):
        EcosLib().destroy_simulation(self.sim)
