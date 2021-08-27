
#ifndef VICO_CONNECTION_HPP
#define VICO_CONNECTION_HPP

#include "property.hpp"

#include <optional>

namespace vico
{

template<class T>
struct connector
{

    std::shared_ptr<property_t<T>> property_;
    std::optional<std::function<T(const T&)>> modifier_ = std::nullopt;
};


struct connection
{

    virtual void transferData() = 0;

    virtual ~connection() = default;
};

template<class T>
struct connection_t : connection
{

    const connector<T> source;
    const std::vector<connector<T>> sinks;

    void transferData() override
    {

        auto data = source.property_->get_value();
        if (source.modifier_) {
            data = source.modifier_->operator()(data);
        }

        for (auto& sink : sinks) {
            if (!sink.modifier_) {
                sink.property_->set_value(data);
            } else {
                sink.property_->set_value(sink.modifier_->operator()(data));
            }
        }
    }
};

// using int_connection = connection_t<int>;
// using real_connection = connection_t<double>;
// using string_connection = connection_t<std::string>;
// using bool_connection = connection_t<bool>;
//
// using connection = std::variant<int_connection


} // namespace vico

#endif // VICO_CONNECTION_HPP
