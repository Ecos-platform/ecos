

#ifndef VICO_CSV_WRITER_HPP
#define VICO_CSV_WRITER_HPP

#include "simulation_listener.hpp"

namespace vico
{

class csv_writer : public simulation_listener
{

public:
    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;
};

} // namespace vico

#endif // VICO_CSV_WRITER_HPP
