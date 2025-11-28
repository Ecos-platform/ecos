
#ifndef LIBECOS_SCENARIO_HPP
#define LIBECOS_SCENARIO_HPP

#include "listeners/simulation_listener.hpp"
#include "simulation.hpp"
#include "variable_identifier.hpp"

#include <algorithm>
#include <filesystem>
#include <utility>
#include <vector>

namespace ecos
{

struct action
{
    double timePoint;
    variable_identifier id;
    double eps;

    action(double time_point, variable_identifier id, double eps)
        : timePoint(time_point)
        , id(std::move(id))
        , eps(eps)
    { }

    virtual void apply(simulation& sim) = 0;

    bool operator<(const action& t) const
    {
        return timePoint > t.timePoint;
    }

    virtual ~action() = default;
};

struct real_action : action
{
    double value;

    real_action(double time_point, const variable_identifier& id, double value, double eps)
        : action(time_point, id, eps)
        , value(value)
    { }

    void apply(simulation& sim) override
    {
        auto prop = sim.get_real_property(id);
        if (!prop) {
            throw std::runtime_error(
                "Real property not found: " + id.instance_name() + "::" + id.variable_name());
        }
        prop->set_value(value);
    }
};

struct bool_action : action
{
    bool value;

    bool_action(double time_point, const variable_identifier& id, bool value, double eps)
        : action(time_point, id, eps)
        , value(value)
    { }

    void apply(simulation& sim) override
    {
        auto prop = sim.get_bool_property(id);
        if (!prop) {
            throw std::runtime_error(
                "Boolean property not found: " + id.instance_name() + "::" + id.variable_name());
        }
        prop->set_value(value);
    }
};

struct int_action : action
{
    int value;

    int_action(double time_point, const variable_identifier& id, int value, double eps)
        : action(time_point, id, eps)
        , value(value)
    { }

    void apply(simulation& sim) override
    {
        auto prop = sim.get_int_property(id);
        if (!prop) {
            throw std::runtime_error(
                "Integer property not found: " + id.instance_name() + "::" + id.variable_name());
        }
        prop->set_value(value);
    }
};

struct string_action : action
{
    std::string value;

    string_action(double time_point, const variable_identifier& id, std::string value, double eps)
        : action(time_point, id, eps)
        , value(std::move(value))
    { }

    void apply(simulation& sim) override
    {
        auto prop = sim.get_string_property(id);
        if (!prop) {
            throw std::runtime_error(
                "String property not found: " + id.instance_name() + "::" + id.variable_name());
        }
        prop->set_value(value);
    }
};

class scenario : public simulation_listener
{

public:
    std::string name;

    template<class T>
    void add_action(double timePoint, variable_identifier id, T value, double eps = 1e-9)
    {

        if constexpr (std::is_same_v<T, double>) {
            actions_.emplace_back(std::make_unique<real_action>(timePoint, std::move(id), value, eps));
        } else if constexpr (std::is_same_v<T, int>) {
            actions_.emplace_back(std::make_unique<int_action>(timePoint, std::move(id), value, eps));
        } else if constexpr (std::is_same_v<T, bool>) {
            actions_.emplace_back(std::make_unique<bool_action>(timePoint, std::move(id), value, eps));
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            actions_.emplace_back(std::make_unique<string_action>(timePoint, std::move(id), value, eps));
        } else {
            throw std::runtime_error("Unsupported action type: " + std::string(typeid(T).name()));
        }
        sortActions();
    }

    void pre_step(simulation& sim) override;

    void on_reset() override;

    static std::unique_ptr<scenario> load(const std::filesystem::path& path);

private:
    std::vector<std::unique_ptr<action>> actions_;
    std::vector<std::unique_ptr<action>> used_actions_;

    void sortActions();
};


} // namespace ecos

#endif // LIBECOS_SCENARIO_HPP
