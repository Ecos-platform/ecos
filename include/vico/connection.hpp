
#ifndef VICO_CONNECTION_HPP
#define VICO_CONNECTION_HPP

#include "property.hpp"

namespace vico {


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



}

#endif // VICO_CONNECTION_HPP
