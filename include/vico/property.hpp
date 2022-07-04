
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

    virtual void applySet() = 0;
    virtual void applyGet() = 0;

//    virtual void updateConnections() = 0;

    virtual ~property() = default;
};


template<class T>
struct property_t : property
{

    explicit property_t(
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
        : getter(getter)
        , setter(setter)
    { }

    T get_value()
    {
        return cachedGet;
    }

    void set_value(const T& value)
    {
        cachedSet = value;
    }

    void applySet() override
    {
        if (setter && cachedSet) {
            auto value = *cachedSet;
            if (inputModifier_) {
                value = inputModifier_->operator()(value);
            }
            *setter(value);
            cachedSet = std::nullopt;
        }
    }

    void applyGet() override
    {
        auto value = getter();
        if (outputModifier_) {
            value = outputModifier_->operator()(value);
        }
        cachedGet = value;
    }

    void set_input_modifier(std::function<T(const T&)> modifier)
    {
        inputModifier_ = std::move(modifier);
    }

    void set_output_modifier(std::function<T(const T&)> modifier)
    {
        outputModifier_ = std::move(modifier);
    }

    //    void updateConnections() override
    //    {
    //        for (connection_t<T>& c : sinks_) {
    //            auto p = c.sink;
    //            if (p) {
    //                auto& mod = c.modifier;
    //                T originalValue = get_value();
    //                if (mod) {
    //                    p->set_value(mod.value()(originalValue));
    //                } else {
    //                    p->set_value(originalValue);
    //                }
    //            }
    //        }
    //    }

    //    connection_t<T>& addSink(property_t<T>* sink)
    //    {
    //        sinks_.emplace_back(connection_t<T>(this, sink));
    //        return sinks_.back();
    //    }

    T operator()()
    {
        return get_value();
    }

    void operator()(const T& value)
    {
        set_value(value);
    }

    static std::unique_ptr<property_t<T>> create(
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
    {
        return std::make_unique<property_t<T>>(getter, setter);
    }

private:
    T cachedGet;
    std::optional<T> cachedSet;

    std::function<T()> getter;
    std::optional<std::function<void(const T&)>> setter = std::nullopt;

    //    std::vector<connection_t<T>> sinks_;
    std::optional<std::function<T(const T&)>> inputModifier_;
    std::optional<std::function<T(const T&)>> outputModifier_;
};

} // namespace vico

#endif // VICO_PROPERTY_HPP
