
#ifndef VICO_COMPONENT_HPP
#define VICO_COMPONENT_HPP

#include "property.hpp"

#include <unordered_map>

namespace vico
{

class model_instance
{

public:
    const std::string instanceName;

    explicit model_instance(std::string instanceName)
        : instanceName(std::move(instanceName))
    { }

    virtual void setup_experiment(double start = 0) = 0;

    virtual void enter_initialization_mode() = 0;

    virtual void exit_initialization_mode() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    void applySets() {
        for(auto& [name, p] : properties_) {
            p->applySet();
        }
    }

    void applyGets() {
        for(auto& [name, p] : properties_) {
            p->applyGet();
        }
    }

    template<class T>
    property_t<T>* getProperty(const std::string& name)
    {
        if (properties_.count(name))
        {
            auto& property = properties_[name];
            return static_cast<property_t<T>*>(property.get());
        }
        return nullptr;
    }

    virtual ~model_instance() = default;

protected:
    std::unordered_map<std::string, std::unique_ptr<property>> properties_;
};

} // namespace vico

#endif // VICO_COMPONENT_HPP
