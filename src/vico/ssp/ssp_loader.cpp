
#include "vico/ssp/ssp_loader.hpp"

#include "vico/ssp/ssp_parser.hpp"

using namespace vico;


simulation_structure vico::load_ssp(const fs::path& path)
{
    SystemStructureDescription desc = parse_ssp(path);

    return {};
}