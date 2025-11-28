
#ifndef ECOS_PROPERTY_HPP
#define ECOS_PROPERTY_HPP

#include "ecos/variable_identifier.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecos
{

/* *
 * \brief Base class for properties in the simulation.
 *
 * Properties are used to manage variables in a simulation, allowing for getting and setting values.
 * They can be of different types (e.g., real, integer, string, boolean) and can have input/output modifiers.
 */
struct property
{

    explicit property(variable_identifier id)
        : id_(std::move(id))
    { }

    [[nodiscard]] variable_identifier id() const
    {
        return id_;
    }

    virtual void applySet() = 0;

    friend std::ostream& operator<<(std::ostream& os, const property& p);

    virtual ~property() = default;

protected:
    variable_identifier id_;
};


template<class T>
struct property_t : property
{

    explicit property_t(
        const variable_identifier& id,
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
        : property(id)
        , getter(getter)
        , setter(setter)
    { }

    [[nodiscard]] T get_value() const
    {
        auto value = getter();
        if (outputModifier_) {
            value = outputModifier_->operator()(value);
        }
        return value;
    }

    // Sets the value to be applied later.
    void set_value(const T& value)
    {
        cachedSet = value;
    }

    // Apply the cached set value to the property.
    void applySet() override
    {
        if (setter && cachedSet) {
            T value = cachedSet.value();
            if (inputModifier_) {
                value = inputModifier_->operator()(value);
            }
            setter->operator()(value);
            cachedSet = std::nullopt;
        }
    }

    void set_input_modifier(std::function<T(const T&)> modifier)
    {
        inputModifier_ = std::move(modifier);
    }

    void clear_input_modifier()
    {
        inputModifier_ = std::nullopt;
    }

    // Assign an output modifier to transform any value being set
    void set_output_modifier(std::function<T(const T&)> modifier)
    {
        outputModifier_ = std::move(modifier);
    }

    // Removes the output modifier, if any.
    void clear_output_modifier()
    {
        outputModifier_ = std::nullopt;
    }

private:
    std::optional<T> cachedSet;

    std::function<T()> getter;
    std::optional<std::function<void(const T&)>> setter = std::nullopt;

    std::optional<std::function<T(const T&)>> inputModifier_;
    std::optional<std::function<T(const T&)>> outputModifier_;
};


struct property_listener
{
    virtual void post_sets() = 0;
    virtual void pre_gets() = 0;

    virtual ~property_listener() = default;
};

class properties
{

public:
    properties() = default;
    properties(const properties&) = delete;
    properties(const properties&&) = delete;

    void apply_sets()
    {
        for (auto& p : realProperties_ | std::views::values) {
            p.applySet();
        }
        for (auto& p : intProperties_ | std::views::values) {
            p.applySet();
        }
        for (auto& p : stringProperties_ | std::views::values) {
            p.applySet();
        }
        for (auto& p : boolProperties_ | std::views::values) {
            p.applySet();
        }

        for (const auto& l : listeners_) {
            l->post_sets();
        }
    }

    void apply_gets()
    {
        for (const auto& l : listeners_) {
            l->pre_gets();
        }
    }

    property_t<double>* get_real_property(const std::string& name)
    {
        if (realProperties_.contains(name)) {
            auto& property = realProperties_.at(name);
            return &property;
        }
        return nullptr;
    }

    property_t<int>* get_int_property(const std::string& name)
    {
        if (intProperties_.contains(name)) {
            auto& property = intProperties_.at(name);
            return &property;
        }
        return nullptr;
    }

    property_t<std::string>* get_string_property(const std::string& name)
    {
        if (stringProperties_.contains(name)) {
            auto& property = stringProperties_.at(name);
            return &property;
        }
        return nullptr;
    }

    property_t<bool>* get_bool_property(const std::string& name)
    {
        if (boolProperties_.contains(name)) {
            auto& property = boolProperties_.at(name);
            return &property;
        }
        return nullptr;
    }

    [[nodiscard]] const std::unordered_map<std::string, property_t<double>>& get_reals() const
    {
        return realProperties_;
    }

    [[nodiscard]] const std::unordered_map<std::string, property_t<int>>& get_integers() const
    {
        return intProperties_;
    }

    [[nodiscard]] const std::unordered_map<std::string, property_t<bool>>& get_booleans() const
    {
        return boolProperties_;
    }

    [[nodiscard]] const std::unordered_map<std::string, property_t<std::string>>& get_strings()
    {
        return stringProperties_;
    }

    void add_real_property(property_t<double> p)
    {
        realProperties_.emplace(p.id().variable_name(), std::move(p));
    }

    void add_int_property(property_t<int> p)
    {
        intProperties_.emplace(p.id().variable_name(), std::move(p));
    }

    void add_string_property(property_t<std::string> p)
    {
        stringProperties_.emplace(p.id().variable_name(), std::move(p));
    }

    void add_bool_property(property_t<bool> p)
    {
        boolProperties_.emplace(p.id().variable_name(), std::move(p));
    }

    [[nodiscard]] bool has_property(const std::string& name) const
    {
        const auto& names = get_property_names();
        return std::ranges::find_if(names, [name](const auto& n) {
            return n == name;
        }) != std::end(names);
    }

    [[nodiscard]] std::vector<std::string> get_property_names() const
    {
        std::vector<std::string> names;
        std::ranges::transform(intProperties_, std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::ranges::transform(realProperties_, std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::ranges::transform(boolProperties_, std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::ranges::transform(stringProperties_, std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        return names;
    }

    void add_listener(std::unique_ptr<property_listener> l)
    {
        listeners_.emplace_back(std::move(l));
    }

private:
    std::vector<std::unique_ptr<property_listener>> listeners_;
    std::unordered_map<std::string, property_t<int>> intProperties_;
    std::unordered_map<std::string, property_t<bool>> boolProperties_;
    std::unordered_map<std::string, property_t<double>> realProperties_;
    std::unordered_map<std::string, property_t<std::string>> stringProperties_;
};

} // namespace ecos

#endif // ECOS_PROPERTY_HPP
