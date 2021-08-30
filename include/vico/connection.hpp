
#ifndef VICO_CONNECTION_HPP
#define VICO_CONNECTION_HPP

#include "property.hpp"

#include <optional>

namespace vico
{

//struct connection
//{
//
//    virtual void transferData() = 0;
//
//    virtual ~connection() = default;
//};

template<class T>
struct connection_t// : connection
{

//    property_t<T> source;
//    std::shared_ptr<property_t<T>> sink;

//    connection_t(property_t<T> source, const property_t<T> sink)
//        : source(std::move(source))
//        , sinks({sink})
//    {
//        source->addSink(sink);
//    }
//
//    connection_t(property_t<T>& source, const std::vector<property_t<T>>& sinks)
//        : source(source)
//        , sinks(sinks)
//    {
//
//        for (auto& sink : sinks) {
//            source->addSink(sink);
//        }
//    }

};

} // namespace vico

#endif // VICO_CONNECTION_HPP
