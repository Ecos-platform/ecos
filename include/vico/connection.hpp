
#ifndef VICO_CONNECTION_HPP
#define VICO_CONNECTION_HPP

#include "property.hpp"

namespace vico
{

struct connection
{
    virtual void transferData() = 0;

    virtual ~connection() = default;
};

template<class T>
struct connection_t : public connection
{
    property_t<T>* source;
    property_t<T>* sink;

    connection_t(property_t<T>* source, property_t<T>* sink)
        : source(source)
        , sink(sink)
    { }

    virtual void transferData() override
    {
        T value = source->get_value();
    }

};

class real_connection : public connection_t<double> {
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


} // namespace vico

#endif // VICO_CONNECTION_HPP
