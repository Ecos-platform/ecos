
#ifndef VICO_FMI2_FMU_HPP
#define VICO_FMI2_FMU_HPP

#include "vico/fmi/fmicontext.hpp"
#include "vico/model.hpp"
#include "vico/util/temp_dir.hpp"

#include <fmilib.h>

namespace vico
{

class fmi2_fmu : public model
{

private:
    fmi2_import_t* handle_;
    std::shared_ptr<fmicontext> ctx_;

    const model_description md_;
    std::shared_ptr<temp_dir> tmpDir_;

public:
    fmi2_fmu(std::shared_ptr<fmicontext> ctx, std::shared_ptr<temp_dir> tmpDir);

    [[nodiscard]] const model_description& get_model_description() const override;

    std::unique_ptr<model_instance> new_instance(const std::string& instanceName) override;

    ~fmi2_fmu() override;
};

} // namespace vico

#endif // VICO_FMI2_FMU_HPP