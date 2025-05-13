# Ecos

>Ecos is a co-simulation engine.

Ecos (Easy co-simulation) is a fast, efficient and very easy to use co-simulation
engine written in modern C++.

##### Ecos provides the following features:
* FMI for Co-simulation version 1.0, 2.0 & 3.0.
* SSP version 1.0.
* Optional per process/remote model execution.
* Post-simulation plotting using matplotlib.
* Command-line-interface (CLI).
* Simplified Python and C interface.
* Minimal (and automatically resolved) build dependencies.

### Building

Ecos bundles all required dependencies. Just build.

```
//windows
cmake . -A x64 -B build -DECOS_BUILD_EXAMPLES=ON -DECOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

//linux
cmake . -B build -DECOS_BUILD_EXAMPLES=ON -DECOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Consuming C/C++ library

Ecos is compatible with CMake FetchContent

```cmake
include(FetchContent)
set(ECOS_BUILD_CLI OFF)     # Set to ON for building ecos command-line-interface
set(ECOS_BUILD_CLIB OFF)    # Set to ON for building C API
set(ECOS_WITH_PROXYFMU OFF) # Set to ON for remoting
FetchContent_Declare(
        ecos
        GIT_REPOSITORY https://github.com/Ecos-platform/ecos.git
        GIT_TAG git_tag_or_commit_id
)
FetchContent_MakeAvailable(ecos)

add_executable(ecos_standalone main.cpp)
target_link_libraries(ecos_standalone PRIVATE libecos) # or libecosc for C API
```

### Per process / remote execution

Ecos enables models to run on separate processes, possibly on another PC.  <br>
Simply prepend `proxyfmu://localhost?file=` to the path of the `fmu(s)` you load. <br>
When targeting an external `proxyfmu` instance, replace `localhost` with `host:port`. 
For each FMU instance created when using `proxyfmu` a new process is created. 
When targeting localhost, Unix Domain Sockets are used, while TCP/IP is used when targeting 
a remote process started with `proxufmu boot --port <portNumber>`.
To successfully make use of `proxyfmu`, make sure the executable built by the project 
is located in the working directory of your application or added to `PATH`
when targeting localhost. Using the Python API, however, this should work out-of-the-box.
`proxyfmu` is implemented using [simplesocket](https://github.com/markaren/SimpleSocket) in conjunction with [flexbuffers](https://flatbuffers.dev/flexbuffers.html).

>Ecos may be built without this feature (less dependencies, faster build) by passing `-DECOS_WITH_PROXYFMU=OFF` to CMake.


### Example

```cpp
using namespace ecos;

int main() {
    
    simulation_structure ss;

    // add models
    ss.add_model("chassis", "chassis.fmu");
    ss.add_model("ground", "ground.fmu");
    ss.add_model("wheel", "wheel.fmu");
    
    //make connections
    ss.make_connection<double>("chassis::p.e", "wheel::p1.e");
    ss.make_connection<double>("wheel::p1.f", "chassis::p.f");
    ss.make_connection<double>("wheel::p.e", "ground::p.e");
    ss.make_connection<double>("ground::p.f", "wheel::p.f");
    
    // setup initialValues
    std::map<variable_identifier, scalar_value> map;
    map["chassis::C.mChassis"] = 4000.0;
    ss.add_parameter_set("initialValues", map);
    
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    
    sim->init("initialValues");
    sim->step_until(10);
    
    sim->terminate();
}
```

### SSP example

```cpp
using namespace ecos;

int main() {
    
    auto ss = load_ssp("quarter-truck.ssp");

    // use a fixed-step algorithm and apply parameterset from SSP file
    auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    
    // setup csv logging
    csv_config config;
    config.register_variable("chassis::zChassis"); // logs a single variable
    
    auto csvWriter = std::make_unique<csv_writer>("data.csv", config);
    const auto outputPath = csvWriter->output_path();
    sim->add_listener(std::move(csvWriter));
    
    sim->init("initialValues");
    sim->step_until(10);
    
    sim->terminate();
    
    plot_csv(outputPath, "ChartConfig.xml");
}
```

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
  --noCsv                       Disable CSV logging.
  --noParallel                  Run single-threaded.
  --csvConfig arg               Path to CSV configuration.
  --chartConfig arg             Path to chart configuration.
  --scenarioConfig arg          Path to scenario configuration.

```

### Python interface

`pip install ecospy`


#### Local pip installation
To install the python package locally:

1. Clone the project
2. Run CMake installation (see Building)
3. Run `pip install .`

> Note: You will need `wheel` in addition to the compile-time requirements listed further below. <br> <br>
> If using an old pip version, append `--use-feature=in-tree-build` if you get an error about `../version.txt`


#### Example
```python
print(f"Ecoslib version: {EcosLib.version()}")

EcosLib.set_log_level("debug")

fmu_path = "BouncingBall.fmu"
result_file = f"results/bouncing_ball.csv"

with EcosSimulationStructure() as ss:
  ss.add_model("ball", fmu_path)
  
  with(EcosSimulation(structure=ss, step_size=1/100)) as sim:
  
      sim.add_csv_writer(result_file)
      sim.init()
      sim.step_until(10)
      sim.terminate()

config = TimeSeriesConfig(
    title="BouncingBall",
    y_label="Height[m]",
    identifiers=["ball::h"])
plotter = Plotter(result_file, config)
plotter.show()
```

#### Colab Notebook Example (pythonfmu + ecospy)

[Mass-spring-damper system](https://colab.research.google.com/drive/1-0jKFViLyo2talqF2W25omwoFRZdOMZx?usp=sharing) 

---

### Compile-time requirements

* Windows (10 >=) or Ubuntu (20.04 >=) 
* C++20 compiler (MSVC >= 17 || gcc11 >=)
* CMake >= 3.19

#### Additional Linux requirements

`sudo apt install libtbb-dev unzip`

### Run-time requirements
* Python3 (optional, required for plotting)
  * matplotlib
  * pandas


### Acknowledgments

This software is made possible thanks to the following third-party projects:
* [CLI11](https://github.com/CLIUtils/CLI11)
* [flatbuffers](https://github.com/google/flatbuffers)
* [pugixml](https://github.com/zeux/pugixml)
* [spdlog](https://github.com/gabime/spdlog)
* [fmi4c](https://github.com/robbr48/fmi4c)
* [subprocess.h](https://github.com/sheredom/subprocess.h)
* [Catch2](https://github.com/catchorg/Catch2)

---
> Want to build FMUs in C++? Check out [FMU4cpp](https://github.com/Ecos-platform/fmu4cpp) </br>
> Want to build FMUs in Python? Check out [PythonFMU](https://github.com/NTNU-IHB/PythonFMU) </br>
