
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

class properties {

public:

    void applySets()
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
    }

    void applyGets()
    {
        for (auto& [name, p] : realProperties_) {
            p->applyGet();
        }
        for (auto& [name, p] : intProperties_) {
            p->applyGet();
        }
        for (auto& [name, p] : stringProperties_) {
            p->applyGet();
        }
        for (auto& [name, p] : boolProperties_) {
            p->applyGet();
        }
    }


    property_t<double>* get_real_property(const std::string& name)
    {
        if (realProperties_.count(name))
        {
            auto& property = realProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<int>* get_int_property(const std::string& name)
    {
        if (intProperties_.count(name))
        {
            auto& property = intProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<std::string>* get_string_property(const std::string& name)
    {
        if (stringProperties_.count(name))
        {
            auto& property = stringProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    property_t<bool>* get_bool_property(const std::string& name)
    {
        if (boolProperties_.count(name))
        {
            auto& property = boolProperties_[name];
            return property.get();
        }
        return nullptr;
    }

    void add_real_property(const std::string& name, std::unique_ptr<property_t<double>> p) {
        realProperties_[name] = std::move(p);
    }

    void add_int_property(const std::string& name, std::unique_ptr<property_t<int>> p) {
        intProperties_[name] = std::move(p);
    }

    void add_string_property(const std::string& name, std::unique_ptr<property_t<std::string>> p) {
        stringProperties_[name] = std::move(p);
    }

    void add_bool_property(const std::string& name, std::unique_ptr<property_t<bool>> p) {
        boolProperties_[name] = std::move(p);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<property_t<int>>> intProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<bool>>> boolProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<double>>> realProperties_;
    std::unordered_map<std::string, std::unique_ptr<property_t<std::string>>> stringProperties_;

};

} // namespace vico

#endif // VICO_PROPERTY_HPP
