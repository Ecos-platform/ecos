
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
        if (!cachedGet) {
            applyGet();
        }
        return *cachedGet;
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
    std::optional<T> cachedGet;
    std::optional<T> cachedSet;

    std::function<T()> getter;
    std::optional<std::function<void(const T&)>> setter = std::nullopt;

    std::optional<std::function<T(const T&)>> inputModifier_;
    std::optional<std::function<T(const T&)>> outputModifier_;
};

} // namespace vico

#endif // VICO_PROPERTY_HPP
