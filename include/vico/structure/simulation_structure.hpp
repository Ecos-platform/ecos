
#ifndef VICO_SIMULATION_STRUCTURE_HPP
#define VICO_SIMULATION_STRUCTURE_HPP

#include "variable_identifier.hpp"

#include "vico/model.hpp"
#include "vico/property.hpp"
#include "vico/simulation.hpp"

#include <fmilibcpp/fmu.hpp>
#include <optional>
#include <unordered_map>
#include <map>
#include <utility>
#include <variant>
#include <vector>


namespace vico
{

template<class T>
struct unbound_connection_t
{
    variable_identifier source;
    variable_identifier sink;
    std::optional<std::function<T(const T&)>> modifier = std::nullopt;

    unbound_connection_t(variable_identifier source, variable_identifier sink, std::optional<std::function<T(const T&)>> modifier = std::nullopt)
        : source(std::move(source))
        , sink(std::move(sink))
        , modifier(std::move(modifier))
    { }

};

using int_connection = unbound_connection_t<int>;
using real_connection = unbound_connection_t<double>;
using string_connection = unbound_connection_t<std::string>;
using bool_connection = unbound_connection_t<bool>;

using unbound_connection = std::variant<int_connection, real_connection, string_connection, bool_connection>;

class simulation_structure
{

public:
    void add_model(const std::string& instanceName, std::shared_ptr<model> model);

    template<class T>
    void make_connection(const std::string& source, const std::string& sink, const std::optional<std::function<T(const T&)>>& modifier = std::nullopt) {
        make_connection(variable_identifier{source}, variable_identifier{sink}, modifier);
    }

    template<class T>
    void make_connection(const variable_identifier& source, const variable_identifier& sink, const std::optional<std::function<T(const T&)>>& modifier = std::nullopt) {
        unbound_connection_t<T> c(source, sink, modifier);
        connections_.emplace_back(c);
    }

    void add_parameter_set(const std::string& name, const std::map<variable_identifier, std::variant<double, int, bool, std::string>>& map) {
        parameterSets[name] = map;
    }

    std::unique_ptr<simulation> load(std::unique_ptr<algorithm> algorithm, std::optional<std::string> parameterSet = std::nullopt);

private:

    std::vector<unbound_connection> connections_;
    std::vector<std::pair<std::string, std::shared_ptr<model>>> models_;
    std::unordered_map<std::string, std::map<variable_identifier, std::variant<double, int, bool, std::string>>> parameterSets;

};

} // namespace vico

#endif // VICO_SIMULATION_STRUCTURE_HPP
