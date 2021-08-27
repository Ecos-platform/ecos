
#ifndef VICO_SYSTEM_HPP
#define VICO_SYSTEM_HPP

#include "property.hpp"

#include <string>
#include <unordered_map>

namespace vico
{

class system
{

public:
    virtual void init() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    template<class T>
    std::optional<property_t<T>> get(const std::string& identifier)
    {

        if (properties_.count(identifier)) {
            return std::get<T>(properties_[identifier]);
        } else {
            return std::nullopt;
        }
    }

    virtual ~system() = default;

protected:
    std::unordered_map<std::string, property> properties_;
};


} // namespace vico

#endif // VICO_SYSTEM_HPP
