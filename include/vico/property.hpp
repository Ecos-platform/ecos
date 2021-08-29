
#ifndef VICO_PROPERTY_HPP
#define VICO_PROPERTY_HPP

#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace vico
{

template<class T>
struct property_t
{

    property_t() = default;

    property_t(const std::function<T()>& getter, const std::optional<std::function<void(const T&)>>& setter)
        : getter(getter)
        , setter(setter)
    { }

    T get_value()
    {
        return getter();
    }

    void set_value(const T &value)
    {
        if (setter) setter.value()(value);
    }

private:
    std::function<T()> getter = [] { return T(); };
    std::optional<std::function<void(const T&)>> setter = std::nullopt;
};


using int_property = property_t<int>;
using real_property = property_t<double>;
using string_property = property_t<std::string>;
using bool_property = property_t<bool>;

using property = std::variant<int_property, real_property, string_property, bool_property>;

} // namespace vico

#endif // VICO_PROPERTY_HPP
