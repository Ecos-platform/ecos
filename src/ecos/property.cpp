
#include "ecos/property.hpp"

using namespace ecos;

std::ostream& ecos::operator<<(std::ostream& os, const property& p)
{

    if (const auto pd = dynamic_cast<const property_t<double>*>(&p)) {
        os << std::to_string(pd->get_value());
    } else if (const auto pi = dynamic_cast<const property_t<int>*>(&p)) {
        os << std::to_string(pi->get_value());
    } else if (const auto pb = dynamic_cast<const property_t<bool>*>(&p)) {
        os << std::noboolalpha << pb->get_value();
    } else if (const auto ps = dynamic_cast<const property_t<std::string>*>(&p)) {
        os << ps->get_value();
    } else {
        os << "<unknown property type>";
    }
    return os;
}
