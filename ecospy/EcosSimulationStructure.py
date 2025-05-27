import os
from .EcosParameterSet import EcosParameterSet
from .lib import dll, EcosLib
from ctypes import c_void_p, c_bool, c_char_p, CFUNCTYPE, c_double


class EcosSimulationStructure:
    """
    Represents the structure of a simulation.

    Can be passed to EcosSimulation to create a simulation instance.

    Supports context management for automatic resource management.
    """

    def __init__(self):

        self.modifiers = []  # to keep modifier function alive

        create_simulation_structure = dll.ecos_simulation_structure_create
        create_simulation_structure.restype = c_void_p

        self._add_model = dll.ecos_simulation_structure_add_model
        self._add_model.argtypes = [c_void_p, c_char_p, c_char_p, c_double]
        self._add_model.restype = c_bool

        self._add_parameter_set = dll.ecos_simulation_structure_add_parameter_set
        self._add_parameter_set.argtypes = [c_void_p, c_char_p, c_void_p]
        self._add_parameter_set.restype = c_bool

        self._make_int_connection = dll.ecos_simulation_structure_make_int_connection
        self._make_int_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_real_connection_mod = dll.ecos_simulation_structure_make_real_connection_mod
        self._make_real_connection_mod.argtypes = [c_void_p, c_char_p, c_char_p, CFUNCTYPE(c_double, c_double)]

        self._make_real_connection = dll.ecos_simulation_structure_make_real_connection
        self._make_real_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_string_connection = dll.ecos_simulation_structure_make_string_connection
        self._make_string_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_bool_connection = dll.ecos_simulation_structure_make_bool_connection
        self._make_bool_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._handle = create_simulation_structure()
        if self._handle is None:
            raise Exception(EcosLib.get_last_error())

    def add_model(self, instance_name: str, uri: str | os.PathLike, step_size_hint: float = None):
        """
        Adds a model to the simulation structure.
        Args:
            instance_name (str): The name of the model instance.
            uri (str | os.PathLike): The URI or path to the model file.
            step_size_hint (float, optional): A hint for the step size. Defaults to None.
        Raises:
            Exception: If adding the model fails.
        """
        if isinstance(uri, os.PathLike):
            uri = os.fspath(uri)

        if not self._add_model(self.handle, instance_name.encode(), uri.encode(),
                               -1 if step_size_hint is None else step_size_hint):
            raise Exception(EcosLib.get_last_error())

    def add_parameter_set(self, name: str, parameters: EcosParameterSet | dict[str, any]):
        """
        Adds a parameter set to the simulation structure.
        Args:
            name (str): The name of the parameter set.
            parameters (EcosParameterSet | dict[str, any]): The parameters to add, either as an EcosParameterSet or a dictionary.
        Raises:
            Exception: If the parameters are of an illegal type or if adding the parameter set fails.
        """
        if isinstance(parameters, EcosParameterSet):
            return self._add_parameter_set(self.handle, name.encode(), parameters.handle)
        elif isinstance(parameters, dict):
            with (EcosParameterSet()) as params:
                for key, value in parameters.items():
                    if isinstance(value, bool):
                        params.add_bool(key, value)
                    elif isinstance(value, float):
                        params.add_real(key, value)
                    elif isinstance(value, int):
                        params.add_int(key, value)
                    elif isinstance(value, str):
                        params.add_string(key, value)
                    else:
                        raise Exception("Illegal value type. Must be int, float, bool or str")
                return self.add_parameter_set(name, params)

        else:
            raise Exception("Illegal parameter type. Must be EcosParameterSet or dict")

    def make_real_connection(self, source: str, sink: str, modifier=None):
        """
        Creates a connection between two real variables in the simulation structure.
        Args:
            source (str): The source variable identifier.
            sink (str): The sink variable identifier.
            modifier (callable, optional): A function to modify the value before passing it to the sink. Defaults to None.
        """
        if modifier is not None:
            @CFUNCTYPE(c_double, c_double)
            def _modifier(value: float) -> float:
                return modifier(value)

            self.modifiers.append(_modifier)
            self._make_real_connection_mod(self.handle, source.encode(), sink.encode(), _modifier)

        else:
            self._make_real_connection(self.handle, source.encode(), sink.encode())

    def make_int_connection(self, source: str, sink: str):
        """
        Creates a connection between two integer variables in the simulation structure.
        Args:
            source (str): The source variable identifier.
            sink (str): The sink variable identifier.
        """
        self._make_int_connection(self.handle, source.encode(), sink.encode())

    def make_string_connection(self, source: str, sink: str):
        """
       Creates a connection between two string variables in the simulation structure.
       Args:
           source (str): The source variable identifier.
           sink (str): The sink variable identifier.
       """
        self._make_string_connection(self.handle, source.encode(), sink.encode())

    def make_bool_connection(self, source: str, sink: str):
        """
       Creates a connection between two boolean variables in the simulation structure.
       Args:
           source (str): The source variable identifier.
           sink (str): The sink variable identifier.
       """
        self._make_bool_connection(self.handle, source.encode(), sink.encode())

    @property
    def handle(self):
        return self._handle

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def free(self):
        """
        Frees the resources associated with the simulation structure.
        """
        if not self._handle is None:
            simulation_structure_destroy = dll.ecos_simulation_structure_destroy
            simulation_structure_destroy.argtypes = [c_void_p]

            simulation_structure_destroy(self._handle)
            self._handle = None
            self.modifiers.clear()

    def __del__(self):
        self.free()
