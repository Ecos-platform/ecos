from pathlib import Path

from ecospy import EcosSimulationStructure, EcosSimulation

fmi1_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / '1.0').resolve()
fmu = fmi1_folder / "identity.fmu"


def test_identity():
    int_value = 42
    real_value = 3.14
    bool_value = True
    string_value = "Hello World!"

    with EcosSimulationStructure() as ss:
        ss.add_model("identity", fmu)

        ss.add_parameter_set("initials", {
            "identity::integerIn": int_value,
            "identity::realIn": real_value,
            "identity::booleanIn": bool_value,
            "identity::stringIn": string_value
        })

        with EcosSimulation(structure=ss, step_size=1 / 100) as sim:
            sim.init(parameter_set="initials")
            assert sim.get_integer("identity::integerIn") == int_value
            assert sim.get_real("identity::realIn") == real_value
            assert sim.get_bool("identity::booleanIn") == bool_value
            assert sim.get_string("identity::stringOut") == string_value

            sim.reset()
            sim.init()
            assert sim.get_integer("identity::integerIn") != int_value
            assert sim.get_real("identity::realIn") != real_value
            assert sim.get_bool("identity::booleanIn") != bool_value
            assert sim.get_string("identity::stringOut") != string_value

            sim.terminate()
