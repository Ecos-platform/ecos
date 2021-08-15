
#ifndef VICO_FMI1_FMU_HPP
#define VICO_FMI1_FMU_HPP

#include "../fmicontext.hpp"

#include "vico/model.hpp"
#include "vico/util/temp_dir.hpp"

#include <fmilib.h>

namespace vico
{

class fmi1_fmu : public model
{

private:
    fmi1_import_t* handle_;
    std::shared_ptr<fmicontext> ctx_;

    const model_description md_;
    std::shared_ptr<temp_dir> tmpDir_;

public:
    fmi1_fmu(std::unique_ptr<fmicontext> ctx, std::shared_ptr<temp_dir> tmpDir);

    [[nodiscard]] const model_description& get_model_description() const override;

    std::unique_ptr<model_instance> new_instance(const std::string& instanceName) override;

    ~fmi1_fmu() override;
};

} // namespace vico

#endif // VICO_FMI1_FMU_HPP