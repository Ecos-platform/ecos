
#ifndef VICO_SIMULATION_STRUCTURE_HPP
#define VICO_SIMULATION_STRUCTURE_HPP

#include "vico/model.hpp"

#include <vector>
#include <variant>

namespace vico
{

template <class T>
struct modifier {

    virtual T invoke(T &value) = 0;

    virtual ~modifier() = default;

};



template <class T>
struct connector_t {

};

template <class T>
struct connection_t
{

    const std::string source;
    const std::string target;

};

using int_connection = connection_t<int>;
using real_connection = connection_t<double>;
using string_connection = connection_t<std::string>;
using bool_connection = connection_t<bool>;

using connection = std::variant<int_connection, real_connection, string_connection, bool_connection>;

class simulation_structure
{

public:

    void make_connection(const std::string &source, const std::string &target);

private:
    std::vector<connection> connections_;
    std::vector<std::shared_ptr<model>> models_;

};

} // namespace vico

#endif // VICO_SIMULATION_STRUCTURE_HPP
