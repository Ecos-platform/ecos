
#ifndef VICO_FMI_FMU_HPP
#define VICO_FMI_FMU_HPP

#include "vico/fmi/model_description.hpp"
#include "vico/fmi/slave.hpp"
#include "vico/util/fs_portability.hpp"

#include <memory>

namespace vico
{

class fmu
{

public:
    [[nodiscard]] virtual const model_description& get_model_description() const = 0;

    virtual std::unique_ptr<slave> new_instance(const std::string& instanceName) = 0;

    virtual ~fmu() = default;
};


std::unique_ptr<fmu> loadFmu(const filesystem::path& fmuPath);

} // namespace vico

#endif // VICO_FMI_FMU_HPP
