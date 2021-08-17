
#ifndef VICO_PROPERTY_HPP
#define VICO_PROPERTY_HPP

#include <functional>
#include <utility>
#include <variant>

namespace vico
{

template<class T>
struct property_t
{

    property_t(std::string name, const std::function<T()>& getter)
        : name(std::move(name))
        , getter(getter)
    { }

    property_t(std::string name, const std::function<T()>& getter, const std::function<void(T&)>& setter)
        : name(std::move(name))
        , getter(getter)
        , setter(setter)
    { }

    T get_value()
    {
        return getter();
    }

    void set_value(T value)
    {
        setter(value);
    }

private:
    std::string name;
    std::function<T()> getter;
    std::function<void(T&)> setter = [](T& value) {};
};

using property = std::variant<int, double, std::string, bool>;

} // namespace vico

#endif // VICO_PROPERTY_HPP
