
#include "vico/ssp/ssp_loader.hpp"

#include <ssp/ssp_parser.hpp>

using namespace vico;


simulation_structure vico::load_ssp(const fs::path& path)
{
    ssp::SystemStructureDescription desc(path.string());

    return {};
}