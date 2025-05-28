import pytest

from pathlib import Path

from ecospy import EcosLib, EcosSimulationStructure, EcosSimulation

fmi1_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / '1.0').resolve()
fmu_files = list(fmi1_folder.rglob("*.fmu"))


@pytest.mark.parametrize("fmu_path", fmu_files, ids=lambda p: p.relative_to(fmi1_folder))
@pytest.mark.parametrize("use_proxy", [False, True], ids=["direct", "proxy"])
def test_fmi1(fmu_path: Path, use_proxy: bool):
    EcosLib.set_log_level("debug")

    try:
        with EcosSimulationStructure() as ss:
            model_path = (
                f"proxyfmu://localhost?file={fmu_path}" if use_proxy else fmu_path
            )
            ss.add_model(fmu_path.stem, model_path)

            with EcosSimulation(structure=ss, step_size=1 / 100) as sim:
                sim.init()
                sim.step_until(10)
                sim.terminate()
    except Exception as e:
        pytest.fail(f"Unexpected exception raised: {e}")
