
#ifndef ECOS_PROPERTY_HPP
#define ECOS_PROPERTY_HPP

#include "ecos/variable_identifier.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecos
{

struct property
{
    const variable_identifier id;

    explicit property(variable_identifier id)
        : id(std::move(id))
    { }

    virtual void applySet() = 0;

    virtual ~property() = default;
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

    T get_value()
    {
        auto value = getter();
        if (outputModifier_) {
            value = outputModifier_->operator()(value);
        }
        return value;
    }

    void set_value(const T& value)
    {
        cachedSet = value;
    }

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

    void set_output_modifier(std::function<T(const T&)> modifier)
    {
        outputModifier_ = std::move(modifier);
    }

    static std::unique_ptr<property_t<T>> create(
        const variable_identifier& id,
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
    {
        return std::make_unique<property_t<T>>(id, getter, setter);
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
        for (auto& [name, p] : realProperties_) {
            p->applySet();
        }
        for (auto& [name, p] : intProperties_) {
            p->applySet();
        }
        for (auto& [name, p] : stringProperties_) {
            p->applySet();
        }
        for (auto& [name, p] : boolProperties_) {
            p->applySet();
        }

        for (auto& l : listeners_) {
            l->post_sets();
        }
    }

    void apply_gets()
    {
        for (auto& l : listeners_) {
            l->pre_gets();
        }
    }

    property_t<double>* get_real_property(const std::string& name)
    {
        if (realProperties_.count(name)) {
            auto& property = realProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<int>* get_int_property(const std::string& name)
    {
        if (intProperties_.count(name)) {
            auto& property = intProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<std::string>* get_string_property(const std::string& name)
    {
        if (stringProperties_.count(name)) {
            auto& property = stringProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<bool>* get_bool_property(const std::string& name)
    {
        if (boolProperties_.count(name)) {
            auto& property = boolProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    const std::unordered_map<std::string, std::unique_ptr<property_t<double>>>& get_reals()
    {
        return realProperties_;
    }

    const std::unordered_map<std::string, std::unique_ptr<property_t<int>>>& get_integers()
    {
        return intProperties_;
    }

    const std::unordered_map<std::string, std::unique_ptr<property_t<bool>>>& get_booleans()
    {
        return boolProperties_;
    }

    const std::unordered_map<std::string, std::unique_ptr<property_t<std::string>>>& get_strings()
    {
        return stringProperties_;
    }

    void add_real_property(const std::string& name, std::unique_ptr<property_t<double>> p)
    {
        realProperties_[name] = std::move(p);
    }

    void add_int_property(const std::string& name, std::unique_ptr<property_t<int>> p)
    {
        intProperties_[name] = std::move(p);
    }

    void add_string_property(const std::string& name, std::unique_ptr<property_t<std::string>> p)
    {
        stringProperties_[name] = std::move(p);
    }

    void add_bool_property(const std::string& name, std::unique_ptr<property_t<bool>> p)
    {
        boolProperties_[name] = std::move(p);
    }

    [[nodiscard]] bool hasProperty(const std::string& name) const
    {
        const auto& names = get_property_names();
        return std::find_if(names.begin(), names.end(), [name](const auto& n) {
            return n == name;
        }) != std::end(names);
    }

    [[nodiscard]] std::vector<std::string> get_property_names() const
    {
        std::vector<std::string> names;
        std::transform(intProperties_.begin(), intProperties_.end(), std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::transform(realProperties_.begin(), realProperties_.end(), std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::transform(boolProperties_.begin(), boolProperties_.end(), std::back_inserter(names), [](auto& pair) {
            return pair.first;
        });
        std::transform(stringProperties_.begin(), stringProperties_.end(), std::back_inserter(names), [](auto& pair) {
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
    std::unordered_map<std::string, std::unique_ptr<property_t<int>>> intProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<bool>>> boolProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<double>>> realProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<std::string>>> stringProperties_;
};

} // namespace ecos

#endif // ECOS_PROPERTY_HPP
