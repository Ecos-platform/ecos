# Ecos

>Ecos is a co-simulation engine.

Ecos (Easy co-simulation) is a fast, efficient and very easy to use co-simulation
engine written in modern C++.

Ecos provides the following features:
* FMI for Co-simulation version 1.0 & 2.0
* SSP version 1.0
* Optional sandboxed/remote model execution using [proxy-fmu](https://github.com/Ecos-platform/proxy-fmu)
* Post-simulation plotting using matplotlib
* Command-line-interface (CLI)
* Simplified Python and C interface

### Example

```cpp
simulation_structure ss;

// add models
ss.add_model("chassis", "chassis.fmu");
ss.add_model("ground", "ground.fmu");
ss.add_model("wheel", "wheel.fmu");

//make connections
ss.make_connection<double>("chassis::p.e", "wheel.p1::e");
ss.make_connection<double>("wheel::p1.f", "chassis.p::f");
ss.make_connection<double>("wheel::p.e", "ground.p::e");
ss.make_connection<double>("ground::p.f", "wheel.p::f");

// setup initialValues
std::map<variable_identifier, scalar_value> map;
map["chassis::C.mChassis"] = 4000.0;
ss.add_parameter_set("initialValues", map);

auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

sim->init();
sim->step_until(10);

sim->terminate();
```

### SSP example

```cpp
auto ss = load_ssp("quarter-truck.ssp");

// use a fixed-step algorithm and apply parameterset from SSP file
auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

// setup csv logging
csv_config config;
config.register_variable("chassis::zChassis"); // logs a single variable

auto csvWriter = std::make_unique<csv_writer>("data.csv", config);
csvWriter->enable_plotting("ChartConfig.xml"); // enable post-simulation plotting
sim->add_listener(std::move(csvWriter));

sim->init();
sim->step_until(10);

sim->terminate();
```

### Installing

The ecos C++ library, `libecos`, is available through the conan remote ais:

`conan remote add ais https://ais.jfrog.io/artifactory/api/conan/ais-conan-local`

Then add a dependency to: </br>
`libecos/<version>@ais/stable` (stable channel -> releases) </br>
`libecos/<version>@ais/testing` (development builds -> master) </br>
`libecos/<version>@ais/testing-<branch>` (development builds -> branches)


### Command line interface

```
Options:
  -h [ --help ]                 Print this help message and quits.
  -v [ --version ]              Print program version.
  -i [ --interactive ]          Make execution interactive.
  -l [ --logLevel ] arg (=info) Specify log level [trace,debug,info,warn,err,of
                                f].
  --path arg                    Location of the fmu/ssp to simulate.
  --stopTime arg (=1)           Simulation end.
  --startTime arg (=0)          Simulation start.
  --stepSize arg                Simulation stepSize.
  --rtf arg (=-1)               Target real time factor (non-positive number ->
                                inf).
  --noLog                       Disable CSV logging.
  --noParallel                  Run single-threaded.
  --logConfig arg               Path to logging configuration.
  --chartConfig arg             Path to chart configuration.
  --scenarioConfig arg          Path to scenario configuration.

```

### Python interface

To use the python interface, simply clone the project and run:

`pip install ecospy`

---

### Compile-time requirements

* Windows (10 >=) or Ubuntu (20.04 >=) 
* C++17 compiler (MSVC >= 16 || gcc9 >=)
* CMake >= 3.15
* Conan

#### Addditional Linux requirements

`sudo apt install libtbb-dev`

### Run-time requirements
* Python3 (required for plotting)
  * matplotlib
  * pandas


---
> Want to build FMUs in C++? Check out [FMU4cpp](https://github.com/Vico-platform/fmu4cpp) </br>
> Want to build FMUs in Kotlin/Java? Check out [FMU4j](https://github.com/Vico-platform/FMU4j) </br>
> Want to build FMUs in Python? Check out [PythonFMU](https://github.com/NTNU-IHB/PythonFMU) </br>
> SSP generation made easy? Check out [SSPgen](https://github.com/Vico-platform/sspgen) </br>
