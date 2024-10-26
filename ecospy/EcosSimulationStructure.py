
from .lib import dll, EcosLib
from ctypes import c_void_p, c_bool, c_char_p


class EcosSimulationStructure:

    def __init__(self):
        create_simulation_structure = dll.ecos_simulation_structure_create
        create_simulation_structure.restype = c_void_p

        self._add_model = dll.ecos_simulation_structure_add_model
        self._add_model.argtypes = [c_void_p, c_char_p, c_char_p]
        self._add_model.restype = c_bool

        self.handle = create_simulation_structure()
        if self.handle is None:
            raise Exception(EcosLib.get_last_error())

    def add_model(self, instance_name: str, uri: str) -> bool:
        return self._add_model(self.handle, instance_name.encode(), uri.encode())

    def __del__(self):
        simulation_structure_destroy = dll.ecos_simulation_structure_destroy
        simulation_structure_destroy.argtypes = [c_void_p]

        simulation_structure_destroy(self.handle)
