
#ifndef ECOS_SIMULATION_STRUCTURE_HPP
#define ECOS_SIMULATION_STRUCTURE_HPP

#include "ecos/model.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/scalar.hpp"
#include "ecos/simulation.hpp"
#include "ecos/variable_identifier.hpp"

#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>


namespace ecos
{

using parameter_set = std::map<variable_identifier, scalar_value>;

class simulation_structure
{

public:
    simulation_structure();

    void add_model(const std::string& instanceName, const std::string& uri, std::optional<double> stepSizeHint = std::nullopt);

    void add_model(const std::string& instanceName, const std::filesystem::path& path, std::optional<double> stepSizeHint = std::nullopt);

    void add_model(const std::string& instanceName, std::shared_ptr<model> model, std::optional<double> stepSizeHint = std::nullopt);

    template<class T>
    void make_connection(variable_identifier source, variable_identifier sink, const std::optional<std::function<T(const T&)>>& modifier = std::nullopt)
    {
        unbound_connection_t<T> c(source, sink, modifier);
        connections_.emplace_back(c);
    }

    void add_parameter_set(const std::string& name, const parameter_set& map);

    std::unique_ptr<simulation> load(std::unique_ptr<algorithm> algorithm);

private:
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

    using unbound_int_connection = unbound_connection_t<int>;
    using unbound_real_connection = unbound_connection_t<double>;
    using unbound_string_connection = unbound_connection_t<std::string>;
    using unbound_bool_connection = unbound_connection_t<bool>;

    using unbound_connection = std::variant<unbound_int_connection, unbound_real_connection, unbound_string_connection, unbound_bool_connection>;

    std::unique_ptr<model_resolver> resolver_;
    std::vector<unbound_connection> connections_;
    std::unordered_map<std::string, parameter_set> parameterSets;
    std::unordered_map<std::string, std::pair<std::shared_ptr<model>, std::optional<double>>> models_;
};

} // namespace ecos

#endif // ECOS_SIMULATION_STRUCTURE_HPP
