# Ecos

[![DOI](https://joss.theoj.org/papers/10.21105/joss.08182/status.svg)](https://doi.org/10.21105/joss.08182)


Ecos (Easy co-simulation) is a fast, efficient and very easy to use co-simulation
engine written in modern C++.

##### Ecos provides the following features:
* FMI for Co-simulation version 1.0 & 2.0.
* Partial support for FMI 3.0.
* SSP version 1.0.
* Optional per process/remote model execution.
* Post-simulation plotting using matplotlib.
* Command-line-interface (CLI).
* Simplified Python and C interface.
* Minimal (and automatically resolved) build dependencies.

#### Who is this for?

`Ecos` is a framework for performing FMI-based co-simulation. Several such libraries/tools exist and cover much of the same ground. 
However, the value-proposition of `Ecos` is the "ease-of-deployment", while still being efficient. The library is written in Object-Oriented C++, 
supports all versions of FMI for Co-simulation and provides a [pypi hosted](https://pypi.org/project/ecospy/) Python interface.
The framework can be used either in C++, C, Python or through a CLI. 
Moreover, it provides seamless integration with plotting (for quick prototyping) and per-process execution of model instances (remoting). 
The project is maintained as a mono-repo with small and few dependencies, making building from a source a breeze. 
This is especially valuable in an educational setting, where emphasis should be on how to use and not how to build.

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

Ecos is compatible with CMake FetchContent:

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
### Features

#### Per process / remote execution

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


#### Scenario configuration
Scenarios in Ecos are actions to be performed during the simulation.
Scenarios are most useful in a CLI context, and can be specified using the `--scenarioConfig` option.
The structure of a scenario file follows the XML schema defined in `resources/scema/ScenarioConfig.xsd`.
Loading a scenario through the API is demonstrated in [here](examples/dp-ship)


#### CSV logging
Ecos supports CSV logging of simulation data. Which variables to log, and how often, is configurable.
In a CLI context, this is done using the `--csvConfig` option with a path to an XML configuration file adhering to 
the `CsvConfig.xsd` schema located in `resources/schema/`. The API also provides the means to configure this programmatically. 
See `/examples` for various demonstrations.


#### Plotting
Ecos supports out-of-the-box plotting of simulation data using matplotlib in both C++ and Python.
Time- and XY series plots can be configured inline or using the `ChartConfig.xsd` XML schema located in `resources/schema/`.
See `/examples` for demonstrations.

### C++ Example

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
    
    double stepSize = 1.0 / 100; // 100 Hz
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(stepSize));
    
    // setup csv logging
    csv_config config;
    config.register_variable("chassis::*"); // logs all chassis variables
    
    auto csvWriter = std::make_unique<csv_writer>("results.csv", config);
    const auto outputPath = csvWriter->output_path();
    sim->add_listener("writer", std::move(csvWriter));
    
    // apply named set of parameters
    sim->init("initialValues");
    sim->step_until(10);
    
    sim->terminate();
    
    plot_csv(outputPath, "ChartConfig.xml"); // plot results
}
```

### C++ SSP example

```cpp
using namespace ecos;

int main() {
    
    auto ss = load_ssp("quarter-truck.ssp"); // accepts .ssp file or extracted folder

    double stepSize = 1.0 / 100; // 100 Hz
    auto sim = ss->load(std::make_unique<fixed_step_algorithm>(stepSize));
    
    // ... see example above for further usage
}
```

### Command line interface (CLI)

```
Options:
  -h,--help                   Print this help message and exit
  -v,--version                Display program version information and exit
  
  subcommands:
    simulate
      -i,--interactive            Make execution interactive.
      --noCsv                     Disable CSV logging.
      --noParallel                Run single-threaded.
      --path REQUIRED             Location of the fmu/ssp to simulate.
      --stopTime [1]              Simulation end.
      --startTime [0]             Simulation start.
      --stepSize REQUIRED         Simulation stepSize.
      --rtf [-1]                  Target real time factor (non-positive number -> inf).
      --csvConfig                 Path to CSV configuration.
      --chartConfig               Path to chart configuration.
      --scenarioConfig            Path to scenario configuration.
      -l,--logLevel ENUM:value in {trace->0,debug->1,info->2,warn->3,err->4,off->5} OR {0,1,2,3,4,5}
                                  Specify log level.
      
    compile
        --fmu REQUIRED         Location of the (FMI3) Source code FMU to compile binaries for.
        --dest                  Output path for generated FMU (defaults to overwrite).
        --force                 Overwrite existing binaries (if any).
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


#### Python Example
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
