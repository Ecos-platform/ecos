

#ifndef VICO_CSV_WRITER_HPP
#define VICO_CSV_WRITER_HPP

#include "simulation_listener.hpp"

#include "vico/util/fs_portability.hpp"
#include <fstream>

namespace vico
{

class csv_writer : public simulation_listener
{

public:

    explicit csv_writer(const fs::path& path);

    void pre_init(simulation& sim) override;

    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;

private:
    std::ofstream outFile_;
};

} // namespace vico

#endif // VICO_CSV_WRITER_HPP
