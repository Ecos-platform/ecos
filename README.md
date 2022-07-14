# Ecos

>Ecos is a co-simulation engine.

Ecos (Easy co-simulation) is a fast and easy to use co-simulation
engine written in modern C++.

Ecos supports:
* FMI for Co-simulation version 1.0 & 2.0
* SSP version 1.0
* Optional sandboxed/remote model execution using proxy-fmu
* Post-simulation plotting

### Example

```cpp
simulation_structure ss;

// add models
ss.add_model("chassis", "../data/fmus/2.0/quarter-truck/chassis.fmu");
ss.add_model("ground", "../data/fmus/2.0/quarter-truck/ground.fmu");
ss.add_model("wheel", "../data/fmus/2.0/quarter-truck/wheel.fmu");

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
sim->step_until(10);

sim->terminate();
```

### SSP example

```cpp
auto ss = load_ssp("../data/ssp/quarter_truck/quarter-truck.ssp");

// use a fixed-step algorithm and apply parameterset from SSP file
auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

// setup csv logging
csv_config config;
config.log_variable("chassis.zChassis"); // logs a single variable

auto csvWriter = std::make_unique<csv_writer>("data.csv", config);
csvWriter->enable_plotting("ChartConfig.xml");
sim->add_listener(std::move(csvWriter));

sim->init();
sim->step_until(10);

sim->terminate();
```


### Requirements

* Conan
* C++17 compiler (MSVC 16 || gcc9 >=)
* CMake >= 3.15
