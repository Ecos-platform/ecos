# Vico

>Vico is a co-simulation engine.

Vico supports:
* FMI for Co-simulation version 1.0 & 2.0
* SSP version 1.0
* Optional sandboxed/remote model execution using proxy-fmu

### Example

```cpp
simulation_structure ss;

// add models
ss.add_model("chassis", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/chassis.fmu"));
ss.add_model("ground", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/ground.fmu"));
ss.add_model("wheel", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/wheel.fmu"));

//make connections
ss.make_connection<double>("chassis.p.e", "wheel.p1.e");
ss.make_connection<double>("wheel.p1.f", "chassis.p.f");
ss.make_connection<double>("wheel.p.e", "ground.p.e");
ss.make_connection<double>("ground.p.f", "wheel.p.f");

// setup initialValues
std::map<variable_identifier, std::variant<double, int, bool, std::string>> map;
map[variable_identifier{"chassis.C.mChassis"}] = 4000.0;
ss.add_parameter_set("initialValues", map);

auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

sim->init();
while (sim->time() < 1) {
    sim->step();
}

sim->terminate();
```

### SSP example

```cpp
auto ss = load_ssp("../data/ssp/quarter_truck/quarter-truck.ssp");

// use a fixed-step algorithm and apply parameterset from SSP file
auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

auto p = sim->get_real_property("chassis.zChassis");

// setup csv logging
csv_config config;
config.log_variable("chassis.zChassis"); // logs a single variable
sim->add_listener(std::make_unique<csv_writer>("data.csv", config));

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
