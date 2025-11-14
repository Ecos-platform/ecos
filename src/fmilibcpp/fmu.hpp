
#ifndef ECOS_FMI_FMU_FMU_HPP
#define ECOS_FMI_FMU_FMU_HPP

#include "model_description.hpp"
#include "slave.hpp"

#include <filesystem>
#include <memory>

namespace fmilibcpp
{

class fmu
{

public:
    [[nodiscard]] virtual const model_description& get_model_description() const = 0;

    virtual std::unique_ptr<slave> new_instance(const std::string& instanceName) = 0;

    virtual ~fmu() = default;
};


std::unique_ptr<fmu> loadFmu(const std::filesystem::path& fmuPath);

} // namespace fmilibcpp

#endif // ECOS_FMI_FMU_FMU_HPP
