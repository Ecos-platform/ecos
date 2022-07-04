
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
    std::optional<std::function<T(const T&)>> modifier = std::nullopt;

    connection_t(property_t<T>* source, property_t<T>* sink)
        : source(source)
        , sink(sink)
    { }

    void transferData() override
    {
        T value = source->get_value();
        if (modifier) {
            value = modifier.value()(value);
        }
        sink->set_value(value);
    }

    void set_modifier(std::optional<std::function<T(const T&)>> mod)
    {
        modifier = std::move(mod);
    }
};


} // namespace vico

#endif // VICO_CONNECTION_HPP
