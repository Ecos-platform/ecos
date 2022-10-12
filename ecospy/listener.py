from abc import ABC


class SimulationListener(ABC):

    def pre(self, t: float):
        pass

    def post(self, t: float):
        pass
