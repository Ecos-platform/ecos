from .lib import dll, EcosLib
from .EcosSimulationStructure import EcosSimulationStructure
from ctypes import c_bool, c_int, c_void_p, c_double, c_char_p, c_size_t, POINTER, Structure, CFUNCTYPE, byref, \
    create_string_buffer


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


class EcosSimulation:
    """
    This class represents a co-simulation.
    Supports context management for automatic resource management.
    """

    def __init__(self, step_size: float, ssp_path: str = None, structure: EcosSimulationStructure = None):
        """
        Initialize a new EcosSimulation instance.

        Args:
           step_size (float): The fixed simulation step size.
           ssp_path (str, optional): Path to an SSP file to create the simulation from. Mutually exclusive with `structure`.
           structure (EcosSimulationStructure, optional): Simulation structure to create the simulation from. Mutually exclusive with `ssp_path`.

        Raises:
           ValueError: If both or neither of `ssp_path` and `structure` are provided.
           Exception: If simulation creation fails in the underlying library.
        """
        if ssp_path is not None and structure is not None:
            raise ValueError("Only one of 'ssp_path' or 'structure' should be provided.")

        self.listener_configs = {}

        self._init_simulation = dll.ecos_simulation_init
        self._init_simulation.argtypes = [c_void_p, c_double, c_char_p]
        self._init_simulation.restype = c_bool

        self._step_simulation = dll.ecos_simulation_step
        self._step_simulation.argtypes = [c_void_p, c_size_t]
        self._step_simulation.restype = c_double

        self._step_simulation_for = dll.ecos_simulation_step_for
        self._step_simulation_for.argtypes = [c_void_p, c_double]

        self._step_simulation_until = dll.ecos_simulation_step_until
        self._step_simulation_until.argtypes = [c_void_p, c_double]

        self._get_time = dll.ecos_simulation_get_time
        self._get_time.argtypes = [c_void_p]
        self._get_time.restype = c_double

        self._get_integer = dll.ecos_simulation_get_integer
        self._get_integer.argtypes = [c_void_p, c_char_p, POINTER(c_int)]
        self._get_integer.restype = c_bool

        self._get_real = dll.ecos_simulation_get_real
        self._get_real.argtypes = [c_void_p, c_char_p, POINTER(c_double)]
        self._get_real.restype = c_bool

        self._get_bool = dll.ecos_simulation_get_bool
        self._get_bool.argtypes = [c_void_p, c_char_p, POINTER(c_bool)]
        self._get_bool.restype = c_bool

        self._get_string = dll.ecos_simulation_get_string
        self._get_string.argtypes = [c_void_p, c_char_p, c_char_p]
        self._get_string.restype = c_bool

        self._set_integer = dll.ecos_simulation_set_integer
        self._set_integer.argtypes = [c_void_p, c_char_p, c_int]
        self._set_integer.restype = c_bool

        self._set_real = dll.ecos_simulation_set_real
        self._set_real.argtypes = [c_void_p, c_char_p, c_double]
        self._set_real.restype = c_bool

        self._set_bool = dll.ecos_simulation_set_bool
        self._set_bool.argtypes = [c_void_p, c_char_p, c_bool]
        self._set_bool.restype = c_bool

        self._set_string = dll.ecos_simulation_set_string
        self._set_string.argtypes = [c_void_p, c_char_p, c_char_p]
        self._set_string.restype = c_bool

        self._create_listener = dll.ecos_simulation_listener_create
        self._create_listener.argtypes = [ListenerConfig]
        self._create_listener.restype = c_void_p

        self._create_csv_writer = dll.ecos_csv_writer_create
        self._create_csv_writer.argtypes = [c_char_p, c_char_p]
        self._create_csv_writer.restype = c_void_p

        self._add_listener = dll.ecos_simulation_add_listener
        self._add_listener.argtypes = [c_void_p, c_char_p, c_void_p]

        self._remove_listener = dll.ecos_simulation_remove_listener
        self._remove_listener.argtypes = [c_void_p, c_char_p]

        if ssp_path is not None:

            _create_simulation_from_ssp = dll.ecos_simulation_create_from_ssp
            _create_simulation_from_ssp.argtypes = [c_char_p, c_double]
            _create_simulation_from_ssp.restype = c_void_p

            self.sim = _create_simulation_from_ssp(ssp_path.encode(), step_size)
            if self.sim is None:
                raise Exception(EcosLib.get_last_error())

        elif structure is not None:
            _create_simulation_from_structure = dll.ecos_simulation_create_from_structure
            _create_simulation_from_structure.argtypes = [c_void_p, c_double]
            _create_simulation_from_structure.restype = c_void_p

            self.sim = _create_simulation_from_structure(structure.handle, step_size)
            if self.sim is None:
                raise Exception(EcosLib.get_last_error())

        else:
            raise ValueError("Either 'ssp_path' or 'structure' must be provided.")

    def add_listener(self, name: str, listener: SimulationListener):
        """
        Add a listener to the simulation.

        Args:
            name (str): name of the listener (used to identify the listener)
            listener (SimulationListener): a listener object that implements the SimulationListener interface
        """
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

        cpp_listener = self._create_listener(config)
        self._add_listener(self.sim, name.encode(), cpp_listener)

    def remove_listener(self, name: str):
        """
        Remove a listener from the simulation.
        Args:
            name (str): Name of the listener to remove.
        """
        self._remove_listener(self.sim, name.encode())

    def add_csv_writer(self, result_file: str, csv_config: str = None, identifiers: list[str] = None,
                       decimation_factor: int = None):
        """
        Add a CSV writer to the simulation.
        Args:
            result_file (str): Path to the CSV result file.
            csv_config (str, optional): Optional CSV configuration string.
            identifiers (list[str], optional): List of variable identifiers to record.
            decimation_factor (int, optional): Factor to reduce the number of recorded samples.
        """

        listener = self._create_csv_writer(result_file.encode(),
                                           None if csv_config is None else csv_config.encode())
        if listener is None:
            raise Exception(EcosLib.get_last_error())

        if identifiers is not None:
            register_variable = dll.ecos_csv_writer_register_variable
            register_variable.argtypes = [c_void_p, c_char_p]
            register_variable.restype = c_bool

            for identifier in identifiers:
                if not register_variable(listener, identifier.encode()):
                    raise Exception(EcosLib.get_last_error())

        if decimation_factor is not None:
            set_decimation = dll.ecos_csv_writer_set_decimation_factor
            set_decimation.argtypes = [c_void_p, c_int]
            set_decimation.restype = c_bool

            if not set_decimation(listener, decimation_factor):
                raise Exception(EcosLib.get_last_error())

        self._add_listener(self.sim, b'csv_writer', listener)

    def load_scenario(self, scenario_file: str):
        """
        Load a scenario file into the simulation.
        Args:
            scenario_file (str): Path to the scenario file to load.
        Raises:
            Exception: If loading the scenario fails in the underlying library.
        """
        _load_scenario = dll.ecos_simulation_load_scenario
        _load_scenario.argtypes = [c_void_p, c_char_p]
        _load_scenario.restype = c_bool

        if not _load_scenario(self.sim, scenario_file.encode()):
            raise Exception(EcosLib.get_last_error())

    def get_integer(self, identifier: str):
        val = c_int()
        if not self._get_integer(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib.get_last_error())
        return val.value

    def get_real(self, identifier: str):
        val = c_double()
        if not self._get_real(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib.get_last_error())
        return val.value

    def get_bool(self, identifier: str):
        val = c_bool()
        if not self._get_bool(self.sim, identifier.encode(), byref(val)):
            raise Exception(EcosLib.get_last_error())
        return val.value

    def get_string(self, identifier: str, buffer_size: int = 1024):
        buffer = create_string_buffer(buffer_size)
        if not self._get_string(self.sim, identifier.encode(), buffer):
            raise Exception(EcosLib.get_last_error())
        return buffer.value.decode()

    def set_integer(self, identifier: str, value: int):
        if not self._set_integer(self.sim, identifier.encode(), value):
            raise Exception(EcosLib.get_last_error())

    def set_real(self, identifier: str, value: float):
        if not self._set_real(self.sim, identifier.encode(), value):
            raise Exception(EcosLib.get_last_error())

    def set_bool(self, identifier: str, value: bool):
        if not self._set_bool(self.sim, identifier.encode(), value):
            raise Exception(EcosLib.get_last_error())

    def set_string(self, identifier: str, value: str):
        if not self._set_string(self.sim, identifier.encode(), value.encode()):
            raise Exception(EcosLib.get_last_error())

    def init(self, start_time: int = 0, parameter_set: str = None):
        """
        Initialize the simulation with a start time and an optional parameter set.
        Args:
            start_time (int): The time to start the simulation from in seconds. Defaults to 0.
            parameter_set (str, optional): Optional parameter set to initialize the simulation with.
        """
        return self._init_simulation(self.sim, start_time, None if parameter_set is None else parameter_set.encode())

    def step(self, num_steps: int = 1) -> float:
        """
        Step the simulation for a given number of steps.
        Args:
            num_steps (int): Number of steps to take in the simulation. Defaults to 1.
        Returns:
            float: The simulation time after stepping.
        """
        return self._step_simulation(self.sim, num_steps)

    def step_for(self, duration: float):
        """
        Step the simulation for a specified duration.
        Args:
            duration (float): Duration to step the simulation for in seconds.
        """
        self._step_simulation_for(self.sim, duration)

    def step_until(self, time_point: float):
        """
        Step the simulation until a specified time point.
        Args:
            time_point (float): The time point to step the simulation until in seconds.
        """
        self._step_simulation_until(self.sim, time_point)

    def get_time(self) -> float:
        """
        Get the current simulation time.
        Returns:
            float: The current simulation time in seconds.
        """
        return self._get_time(self.sim)

    def reset(self):
        """
        Reset the simulation to its initial state.
        Raises:
            Exception: If the reset operation fails in the underlying library.
        """
        reset_simulation = dll.ecos_simulation_reset
        reset_simulation.argtypes = [c_void_p]
        reset_simulation.restype = c_bool

        if not reset_simulation(self.sim):
            raise Exception(EcosLib.get_last_error())

    def terminate(self):
        """
        Terminate the simulation gracefully.
        Raises:
            Exception: If the termination operation fails in the underlying library.
        """
        terminate_simulation = dll.ecos_simulation_terminate
        terminate_simulation.argtypes = [c_void_p]
        terminate_simulation.restype = c_bool

        if not terminate_simulation(self.sim):
            raise Exception(EcosLib.get_last_error())

    def free(self):
        """
        Frees the resources associated with the simulation structure.
        """
        if not self.sim is None:
            destroy_simulation = dll.ecos_simulation_destroy
            destroy_simulation.argtypes = [c_void_p]

            destroy_simulation(self.sim)
            self.sim = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def __del__(self):
        self.free()
