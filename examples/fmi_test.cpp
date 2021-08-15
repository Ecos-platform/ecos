
#include <vico/fmi/fmu.hpp>

#include <exception>
#include <iostream>

using namespace vico;

void run()
{

    std::string fmuPath("../fmus/2.0/20sim/ControlledTemperature.fmu");
    auto fmu = loadFmu(fmuPath);

    auto md = fmu->get_model_description();
    std::cout << "GUID=" << md.guid << std::endl;
    std::cout << "description=" << md.description << std::endl;
    std::cout << "generationTool=" << md.generationTool << std::endl;

    auto mv = md.modelVariables;
    std::cout << mv.size() << std::endl;
    std::cout << mv[0].name << std::endl;

    for (auto& v : mv) {
        auto attr = v.typeAttribute;
        if (v.is_real()) {
            auto start = std::get<real>(attr).start;
            if (start) {
                std::cout << "start=" << std::get<real>(attr).start.value() << std::endl;
            }
        }
    }

    auto v = std::vector<std::unique_ptr<slave>>();
    for (auto i = 0; i < 10; i++) {
        auto slave = fmu->new_instance("instance_" + std::to_string(i));
        slave->setup_experiment();
        slave->enter_initialization_mode();
        slave->exit_initialization_mode();
        slave->step(0.0, 0.1);

        v.push_back(std::move(slave));
    }

    for (auto& slave : v) {
        slave->terminate();
        slave->freeInstance();
    }
}

int main()
{
    try {

        run();

    } catch (std::exception& ex) {
        std::cerr << "error: " << ex.what() << std::endl;
    }
    return 0;
}