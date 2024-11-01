
from .lib import dll, EcosLib
from ctypes import c_void_p, c_bool, c_char_p


class EcosSimulationStructure:

    def __init__(self):
        create_simulation_structure = dll.ecos_simulation_structure_create
        create_simulation_structure.restype = c_void_p

        self._add_model = dll.ecos_simulation_structure_add_model
        self._add_model.argtypes = [c_void_p, c_char_p, c_char_p]
        self._add_model.restype = c_bool

        self._make_int_connection = dll.ecos_simulation_structure_make_int_connection
        self._make_int_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_real_connection = dll.ecos_simulation_structure_make_real_connection
        self._make_real_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_string_connection = dll.ecos_simulation_structure_make_string_connection
        self._make_string_connection.argtypes = [c_void_p, c_char_p, c_char_p]

        self._make_bool_connection = dll.ecos_simulation_structure_make_bool_connection
        self._make_bool_connection.argtypes = [c_void_p, c_char_p, c_char_p]


        self.handle = create_simulation_structure()
        if self.handle is None:
            raise Exception(EcosLib.get_last_error())

    def add_model(self, instance_name: str, uri: str) -> bool:
        return self._add_model(self.handle, instance_name.encode(), uri.encode())

    def make_real_connection(self, source: str, sink: str):
        self._make_real_connection(self.handle, source.encode(), sink.encode())

    def make_int_connection(self, source: str, sink: str):
        self._make_int_connection(self.handle, source.encode(), sink.encode())

    def make_string_connection(self, source: str, sink: str):
        self._make_string_connection(self.handle, source.encode(), sink.encode())

    def make_bool_connection(self, source: str, sink: str):
        self._make_bool_connection(self.handle, source.encode(), sink.encode())

    def __del__(self):
        simulation_structure_destroy = dll.ecos_simulation_structure_destroy
        simulation_structure_destroy.argtypes = [c_void_p]

        simulation_structure_destroy(self.handle)
