# Vico

>Vico is a co-simulation engine.

Vico supports:
* FMI for Co-simulation version 1.0 & 2.0
* SSP version 1.0
* Optional sandboxed/remote model execution using proxy-fmu


### Example

```cpp
auto ss = load_ssp("../data/ssp/quarter_truck/quarter-truck.ssp");

// use a fixed-step algorithm and apply parameterset from SSP file
auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

auto p = sim->get_real_property("chassis.zChassis");

// setup csv logging
csv_config config;
config.log_variable("chassis.zChassis"); // logs a single variable
sim->add_listener(std::make_unique<csv_writer>("sub/sub/data.csv", config));

sim->init();
while (sim->time() < 1) {
    sim->step();
}

sim->terminate();
```


### Requirements

* Conan
* C++17 compiler (MSVC 16 || gcc9 >=)
* CMake >= 3.15
