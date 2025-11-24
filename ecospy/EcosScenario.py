from .lib import dll

from ctypes import c_void_p, c_double, c_int, c_bool, c_char_p


class EcosScenario:

    def __init__(self, scenario_file: str):

        self._handle = None
        if scenario_file is None:
            _scenario_create = dll.ecos_scenario_create
            _scenario_create.restype = c_void_p
            self._handle = _scenario_create()
        else:
            _scenario_load = dll.ecos_scenario_load
            _scenario_load.restype = c_void_p
            _scenario_load.argtypes = [c_char_p]
            self._handle = _scenario_load(scenario_file.encode('utf-8'))


    def add_real_action(self, time_point: float, identifier: str, value: float):
        """
        Adds a real action to the scenario.
        Args:
            time_point (float): The time point at which the action should be executed.
            identifier (str): The identifier of the variable to be set.
            value (float): The value to set the variable to.
        """
        _scenario_add_real_action = dll.ecos_scenario_add_real_action
        _scenario_add_real_action.argtypes = [c_void_p, c_double, c_char_p, c_double]
        _scenario_add_real_action(self._handle, time_point, identifier.encode('utf-8'), value)

    def add_int_action(self, time_point: float, identifier: str, value: int):
        """
        Adds an integer action to the scenario.
        Args:
            time_point (float): The time point at which the action should be executed.
            identifier (str): The identifier of the variable to be set.
            value (int): The value to set the variable to.
        """
        _scenario_add_int_action = dll.ecos_scenario_add_int_action
        _scenario_add_int_action.argtypes = [c_void_p, c_double, c_char_p, c_int]
        _scenario_add_int_action(self._handle, time_point, identifier.encode('utf-8'), value)

    def add_boolean_action(self, time_point: float, identifier: str, value: bool):
        """
        Adds an boolean action to the scenario.
        Args:
            time_point (float): The time point at which the action should be executed.
            identifier (str): The identifier of the variable to be set.
            value (int): The value to set the variable to.
        """
        _scenario_add_int_action = dll.ecos_scenario_add_bool_action
        _scenario_add_int_action.argtypes = [c_void_p, c_double, c_char_p, c_bool]
        _scenario_add_int_action(self._handle, time_point, identifier.encode('utf-8'), value)
