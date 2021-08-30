
#ifndef VICO_PROPERTY_HPP
#define VICO_PROPERTY_HPP

#include "vico/util/visitor.hpp"

#include <functional>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

namespace vico
{

struct property
{

    virtual void updateConnections() = 0;

    virtual ~property() = default;
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

    void addModifier(std::function<T(const T&)> modifier)
    {
        modifiers_.emplace_back(std::move(modifier));
    }

    void updateConnections() override
    {
        for (auto& sink : sinks_) {
            auto p = sink.first;
            if (p) {
                auto mod = sink.second;
                T originalValue = get_value();
                if (mod) {
                    p->set_value(mod.value()(originalValue));
                } else {
                    p->set_value(originalValue);
                }
            }
        }
    }

    void addSink(property_t<T>* sink, std::optional<std::function<T(const T&)>> modifier)
    {
        sinks_.emplace_back(std::make_pair(sink, modifier));
    }

    static std::shared_ptr<property_t<T>> create(
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
    {

        return std::make_shared<property_t<T>>(getter, setter);
    }

private:
    std::function<T()> getter = [] { return T(); };
    std::optional<std::function<void(const T&)>> setter = std::nullopt;

    std::vector<std::pair<property_t<T>*, std::optional<std::function<T(const T&)>>>> sinks_;
    std::vector<std::function<T(const T&)>> modifiers_;
};

} // namespace vico

#endif // VICO_PROPERTY_HPP
