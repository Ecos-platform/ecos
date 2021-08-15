#include "fmi1_fmu.hpp"

#include "vico/fmi/fmi1/fmi1_model_description.hpp"
#include "vico/fmi/fmi1/fmi1_slave.hpp"

#include <fmilib.h>


using namespace vico;

fmi1_fmu::fmi1_fmu(std::unique_ptr<fmicontext> ctx, std::shared_ptr<temp_dir> tmpDir)
    : ctx_(std::move(ctx))
    , handle_(fmi1_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str()))
    , tmpDir_(std::move(tmpDir))
    , md_(create_model_description(handle_))
{
    auto kind = fmi1_import_get_fmu_kind(handle_);
    if (kind != fmi1_fmu_kind_enu_cs_standalone && kind != fmi1_fmu_kind_enu_cs_tool) {
        throw std::runtime_error("FMU does not support Co-simulation!");
    }
}

const model_description& fmi1_fmu::get_model_description() const
{
    return md_;
}

std::unique_ptr<model_instance> fmi1_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi1_slave>(ctx_, instanceName, md_, tmpDir_);
}

fmi1_fmu::~fmi1_fmu()
{
    fmi1_import_free(handle_);
}
