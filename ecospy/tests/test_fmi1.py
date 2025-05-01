import pytest

from pathlib import Path
from ecospy import EcosSimulationStructure, EcosSimulation

fmi1_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / '1.0').resolve()
fmu_files = list(fmi1_folder.rglob("*.fmu"))

@pytest.mark.parametrize("fmu_path", fmu_files, ids=lambda p: str(p.relative_to(fmi1_folder)))
def test_fmi1(fmu_path: Path):

    with EcosSimulationStructure() as ss:

        ss.add_model("instance", fmu_path)

        with EcosSimulation(structure=ss, step_size=1/100) as sim:

            sim.init()
            sim.step_until(10)
            sim.terminate()
