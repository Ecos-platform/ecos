
#ifndef ECOS_FMI_FMI2_FMU_HPP
#define ECOS_FMI_FMI2_FMU_HPP

#include "fmilibcpp/fmicontext.hpp"
#include "fmilibcpp/fmu.hpp"

#include "ecos/util/temp_dir.hpp"

#include <fmilib.h>

namespace fmilibcpp
{

class fmi2_fmu : public fmu
{

private:
    fmi2_import_t* handle_;
    std::shared_ptr<fmicontext> ctx_;

    const bool fmiLogging_;
    const model_description md_;
    std::shared_ptr<ecos::temp_dir> tmpDir_;


public:
    fmi2_fmu(std::shared_ptr<fmicontext> ctx, std::shared_ptr<ecos::temp_dir> tmpDir, bool fmiLogging = true);

    [[nodiscard]] const model_description& get_model_description() const override;

    std::unique_ptr<slave> new_instance(const std::string& instanceName) override;

    ~fmi2_fmu() override;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI2_FMU_HPP