
#ifndef ECOS_FMI_FMI1_FMU_HPP
#define ECOS_FMI_FMI1_FMU_HPP

#include "fmilibcpp/fmicontext.hpp"
#include "fmilibcpp/fmu.hpp"

#include <fmi4c.h>

#include <memory>

namespace fmilibcpp
{

class fmi1_fmu : public fmu
{

public:
    explicit fmi1_fmu(std::unique_ptr<fmicontext> ctx, bool fmiLogging = true);

    [[nodiscard]] const model_description& get_model_description() const override;

    [[nodiscard]] std::unique_ptr<slave> new_instance(const std::string& instanceName) override;

    ~fmi1_fmu() override;

private:
    std::shared_ptr<fmicontext> ctx_;

    bool fmiLogging_;
    model_description md_;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI1_FMU_HPP