
#ifndef VICO_MODEL_HPP
#define VICO_MODEL_HPP

#include "vico/model_description.hpp"
#include "vico/model_instance.hpp"
#include "vico/util/fs_portability.hpp"

#include <memory>

namespace vico
{

class model
{

public:
    [[nodiscard]] virtual const model_description& get_model_description() const = 0;

    virtual std::unique_ptr<model_instance> new_instance(const std::string& instanceName) = 0;

    virtual ~model() = default;
};


std::unique_ptr<model> loadFmu(const filesystem::path& fmuPath);

} // namespace vico

#endif // VICO_MODEL_HPP
