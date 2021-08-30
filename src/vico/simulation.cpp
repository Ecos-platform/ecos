
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(double baseStepSize)
    : baseStepSize(baseStepSize)
{ }

void simulation::add_system(std::unique_ptr<vico::system> system)
{
    systems_.emplace_back(std::move(system));
}

//void simulation::add_connection(std::unique_ptr<connection> c)
//{
//    connections_.emplace_back(std::move(c));
//}

void simulation::init(double startTime)
{

    if (!initialized) {
        initialized = true;
        for (auto& listener : listeners_) {
            listener->pre_init();
        }

        for (auto& system : systems_) {
            system->init(startTime);
        }

        for (auto& listener : listeners_) {
            listener->post_init();
        }
    }
}

void simulation::step(unsigned int numStep)
{

    if (!initialized) init();

    for (unsigned i = 0; i < numStep; i++) {

        for (auto& listener : listeners_) {
            listener->pre_step();
        }

        for (auto& system : systems_) {
            system->step(currentTime, baseStepSize);
        }

        currentTime += baseStepSize;

        for (auto& listener : listeners_) {
            listener->post_step();
        }
    }
}

void simulation::terminate()
{
    for (auto& system : systems_) {
        system->terminate();
    }

    for (auto& listener : listeners_) {
        listener->post_terminate();
    }
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}

//void simulation::add_connection(const std::string& source, const std::string& sink)
//{
//    auto p1 = get_property(source);
//    if (!p1) throw std::runtime_error("No such property: " + source);
//    auto p2 = get_property(sink);
//    if (!p2) throw std::runtime_error("No such property: " + sink);
//
//    if (p1->index() != p2->index() ) throw std::runtime_error("Type mismatch!");
//
//    if (std::holds_alternative<int_property>(*p1)) {
//        auto ip1 = std::get<int_property>(*p1);
//        auto ip2 =  std::get<int_property>(*p2);
//        ip1->addSink(ip2);
//    } else if (std::holds_alternative<real_property>(*p1)) {
//        auto ip1 = std::get<real_property>(*p1);
//        auto ip2 =  std::get<real_property>(*p2);
//        ip1->addSink(ip2);
//    } else if (std::holds_alternative<string_property>(*p1)) {
//        auto ip1 = std::get<string_property>(*p1);
//        auto ip2 =  std::get<string_property>(*p2);
//        ip1->addSink(ip2);
//    } else if (std::holds_alternative<bool_property>(*p1)) {
//        auto ip1 = std::get<bool_property>(*p1);
//        auto ip2 =  std::get<bool_property>(*p2);
//        ip1->addSink(ip2);
//    } else {
//        throw std::runtime_error("Assertion error!");
//    }
//
//}
