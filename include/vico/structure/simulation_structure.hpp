
#ifndef VICO_SIMULATION_STRUCTURE_HPP
#define VICO_SIMULATION_STRUCTURE_HPP

#include "vico/property.hpp"
#include "variable_identifier.hpp"

#include <fmilibcpp/fmu.hpp>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>


namespace vico
{

template<class T>
struct unbound_connector
{
    std::string instanceName;
    std::string propertyName;
    std::optional<std::function<T(const T&)>> modifier = std::nullopt;

    unbound_connector(std::string instanceName, std::string propertyName, std::optional<std::function<T(const T&)>> modifier = std::nullopt)
        : instanceName(std::move(instanceName))
        , propertyName(std::move(propertyName))
        , modifier(std::move(modifier))
    { }
};

template<class T>
struct unbound_connection_t
{
    const unbound_connector<T> source;
    const std::vector<unbound_connector<T>> sinks;

    unbound_connection_t(const unbound_connector<T>& source, const unbound_connector<T>& sink)
        : source(source)
        , sinks({sink})
    { }

    unbound_connection_t(const unbound_connector<T>& source, const std::vector<unbound_connector<T>>& sinks)
        : source(source)
        , sinks(sinks)
    { }
};

using int_connection = unbound_connection_t<int>;
using real_connection = unbound_connection_t<double>;
using string_connection = unbound_connection_t<std::string>;
using bool_connection = unbound_connection_t<bool>;

using unbound_connection = std::variant<int_connection, real_connection, string_connection, bool_connection>;

struct model_instance_template
{
    const std::string instanceName;

    model_instance_template(std::string instanceName, std::shared_ptr<fmilibcpp::fmu> model)
        : instanceName(std::move(instanceName))
        , model_(std::move(model))
    { }

    [[nodiscard]] fmilibcpp::model_description get_model_description() const
    {
        return model_->get_model_description();
    }

    [[nodiscard]] std::unique_ptr<fmilibcpp::slave> instantiate() const
    {
        return model_->new_instance(instanceName);
    }

private:
    const std::shared_ptr<fmilibcpp::fmu> model_;
};

class simulation_structure
{

public:
    void add_model(const std::string& instanceName, std::shared_ptr<fmilibcpp::fmu> model);

    void make_connection(const variable_identifier& source, const variable_identifier& target);

private:
    std::vector<unbound_connection> connections_;
    std::vector<model_instance_template> models_;

    friend class simulation;
};

} // namespace vico

#endif // VICO_SIMULATION_STRUCTURE_HPP
