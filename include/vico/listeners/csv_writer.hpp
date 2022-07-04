

#ifndef VICO_CSV_WRITER_HPP
#define VICO_CSV_WRITER_HPP

#include "simulation_listener.hpp"

namespace vico
{

class csv_writer : public simulation_listener
{

public:
    void pre_init(simulation& sim) override
    {
        for (auto& instance : sim.get_instances()) {
            for (auto& [name, property] : instance->get_properties()) {
                property->get_value();
            }
        }
    }

    void post_init(simulation& sim) override
    {
    }

    void pre_step(simulation& sim) override
    {
    }

    void post_step(simulation& sim) override
    {
    }

    void post_terminate(simulation& sim) override
    {
    }
};

} // namespace vico

#endif // VICO_CSV_WRITER_HPP
