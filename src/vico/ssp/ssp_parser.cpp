
#include "vico/ssp/ssp_parser.hpp"

#include <vico/util/temp_dir.hpp>
#include <vico/util/unzipper.hpp>

#include <pugixml.hpp>

using namespace vico;

simulation_structure parse(const fs::path& path)
{
    temp_dir dir("ssp");
    unzip(path, dir.path());

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(fs::path(dir.path() / "SystemStructureDescription.ssd").c_str());
    if (!result) {
        throw std::runtime_error("Unable to parse SystemStructureDescription.ssd");
    }
    auto root = doc.child("ssd:SystemStructureDescription");

    return {};
}