
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
    virtual void init(double startTime) = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    property *get_property(const std::string& identifier)
    {

        if (properties_.count(identifier)) {
            return properties_.at(identifier).get();
        } else {
            return nullptr;
        }
    }

    template<class T>
    property_t<T> *get_property(const std::string& identifier)
    {

        if (properties_.count(identifier)) {
            return static_cast<property_t<T>*>(dynamic_cast<property*>(properties_.at(identifier).get()));
        } else {
            return nullptr;
        }
    }


    virtual ~system() = default;

protected:
    std::unordered_map<std::string, std::shared_ptr<property>> properties_;
};


} // namespace vico

#endif // VICO_SYSTEM_HPP
