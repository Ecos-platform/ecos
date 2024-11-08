#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/logger/logger.hpp"

using namespace ecos;

int main()
{
    set_logging_level(log::level::debug);

    try {
        const auto ss = load_ssp(std::string(DATA_FOLDER));
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(0.04));

        sim->init();
        sim->step_until(10);

        sim->terminate();
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
