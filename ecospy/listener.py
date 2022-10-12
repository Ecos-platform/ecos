from abc import ABC
from ctypes import Structure, c_double, c_size_t


class SimulationInfo(Structure):
    _fields_ = [("time", c_double), ("iterations", c_size_t)]


class SimulationListener(ABC):

    def pre(self, info: SimulationInfo):
        pass

    def post(self, info: SimulationInfo):
        pass
