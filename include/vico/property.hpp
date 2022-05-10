
#ifndef VICO_PROPERTY_HPP
#define VICO_PROPERTY_HPP

#include <functional>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace vico
{

struct property
{

    virtual void updateConnections() = 0;

    virtual ~property() = default;
};

template<class T>
struct property_t;

template<class T>
struct connection_t
{
    property_t<T>* source;
    property_t<T>* sink;
    std::optional<std::function<T(const T&)>> modifier = std::nullopt;

    connection_t(property_t<T>* source, property_t<T>* sink)
        : source(source)
        , sink(sink)
    { }

    void set_modifier(const std::optional<std::function<T(const T&)>>& mod)
    {
        modifier = mod;
    }
};


template<class T>
struct property_t : property
{

    bool applyModifiers = true;

    explicit property_t(
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
        : getter(getter)
        , setter(setter)
    { }

    T get_value()
    {
        auto value = getter();
        if (applyModifiers) {
            for (auto& mod : modifiers_) {
                value = mod(value);
            }
        }
        return value;
    }

    void set_value(const T& value)
    {
        if (setter) {
            if (applyModifiers) {
                auto v = value;
                for (auto& mod : modifiers_) {
                    v = mod(value);
                }
                setter.value()(v);
            } else {
                setter.value()(value);
            }
        }
    }

    void add_modifier(std::function<T(const T&)> modifier)
    {
        modifiers_.emplace_back(std::move(modifier));
    }

    void updateConnections() override
    {
        for (connection_t<T>& c : sinks_) {
            auto p = c.sink;
            if (p) {
                auto& mod = c.modifier;
                T originalValue = get_value();
                if (mod) {
                    p->set_value(mod.value()(originalValue));
                } else {
                    p->set_value(originalValue);
                }
            }
        }
    }

    connection_t<T>& addSink(property_t<T>* sink)
    {
        sinks_.emplace_back(connection_t<T>(this, sink));
        return sinks_.back();
    }

    static std::shared_ptr<property_t<T>> create(
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
    {

        return std::make_shared<property_t<T>>(getter, setter);
    }

    T operator()()
    {
        return get_value();
    }

    void operator()(const T& value)
    {
        set_value(value);
    }

private:
    std::function<T()> getter;
    std::optional<std::function<void(const T&)>> setter = std::nullopt;

    std::vector<connection_t<T>> sinks_;
    std::vector<std::function<T(const T&)>> modifiers_;
};

} // namespace vico

#endif // VICO_PROPERTY_HPP
