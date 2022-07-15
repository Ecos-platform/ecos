
#ifndef ECOS_CONNECTION_HPP
#define ECOS_CONNECTION_HPP

#include "property.hpp"

#include <functional>
#include <stdexcept>
#include <utility>

namespace ecos
{

struct connection
{
    virtual void transferData() = 0;

    virtual ~connection() = default;
};

template<class T, class E>
struct connection_te : public connection
{
    property_t<T>* source;
    property_t<E>* sink;
    std::optional<std::function<E(const T&)>> modifier;

    connection_te(property_t<T>* source, property_t<E>* sink, std::function<E(const T&)> mod)
        : source(source)
        , sink(sink)
        , modifier(std::move(mod))
    { }

    void transferData() override
    {
        T value = source->get_value();
        if (modifier) {
            E mod = modifier.value()(value);
            sink->set_value(mod);
        } else {
            throw std::runtime_error("Modifier required!");
        }
    }

protected:
    connection_te(property_t<T>* source, property_t<T>* sink)
        : source(source)
        , sink(sink)
    { }
};

template<class T>
struct connection_t : public connection_te<T, T>
{

    connection_t(property_t<T>* source, property_t<T>* sink)
        : connection_te<T, T>(source, sink)
    { }

    void transferData() override
    {
        T value = this->source->get_value();
        this->sink->set_value(value);
    }
};

class real_connection : public connection_t<double>
{
public:
    std::optional<std::function<double(double)>> modifier = std::nullopt;

    real_connection(property_t<double>* source, property_t<double>* sink)
        : connection_t<double>(source, sink)
    { }

    void transferData() override
    {
        double value = source->get_value();
        if (modifier) {
            value = modifier.value()(value);
        }
        sink->set_value(value);
    }

    void set_modifier(std::optional<std::function<double(double)>> mod)
    {
        modifier = std::move(mod);
    }
};
using int_connection = connection_t<int>;
using bool_connection = connection_t<bool>;
using string_connection = connection_t<std::string>;


} // namespace ecos

#endif // ECOS_CONNECTION_HPP
