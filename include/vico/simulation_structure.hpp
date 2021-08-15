
#ifndef VICO_SIMULATION_STRUCTURE_HPP
#define VICO_SIMULATION_STRUCTURE_HPP

#include "vico/model.hpp"

#include <vector>
#include <unordered_map>
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

struct model_instance_template {

    std::string instanceName;
    std::shared_ptr<model> model;

    std::unique_ptr<model_instance> instantiate() {

        return model->new_instance(instanceName);
    }
};

class simulation_structure
{

public:

    void add_model(const std::string &name, std::shared_ptr<model> model);

    void make_connection(const std::string &source, const std::string &target);

private:
    std::vector<connection> connections_;
    std::vector<model_instance_template> models_;

    friend class simulation;

};

} // namespace vico

#endif // VICO_SIMULATION_STRUCTURE_HPP
