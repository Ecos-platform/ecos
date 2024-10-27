
#ifndef ECOS_FMI_FMI1_FMU_HPP
#define ECOS_FMI_FMI1_FMU_HPP

#include "ecos/util/temp_dir.hpp"

#include "fmilibcpp/fmicontext.hpp"
#include "fmilibcpp/fmu.hpp"

#include <fmi4c.h>

namespace fmilibcpp
{

class fmi1_fmu : public fmu
{

public:
    fmi1_fmu(std::unique_ptr<fmicontext> ctx, std::shared_ptr<ecos::temp_dir> tmpDir, bool fmiLogging = true);

    [[nodiscard]] const model_description& get_model_description() const override;

    [[nodiscard]] std::unique_ptr<slave> new_instance(const std::string& instanceName) override;

    ~fmi1_fmu() override;

private:
    fmiHandle* handle_;
    std::shared_ptr<fmicontext> ctx_;

    const bool fmiLogging_;
    const model_description md_;
    std::shared_ptr<ecos::temp_dir> tmpDir_;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI1_FMU_HPP