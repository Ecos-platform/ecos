#include "fmi1_fmu.hpp"

#include "fmi1_model_description.hpp"
#include "fmi1_slave.hpp"

#include <fmilib.h>


namespace fmilibcpp
{

fmi1_fmu::fmi1_fmu(std::unique_ptr<fmicontext> ctx, std::shared_ptr<ecos::temp_dir> tmpDir, bool fmiLogging)
    : ctx_(std::move(ctx))
    , handle_(fmi1_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str()))
    , tmpDir_(std::move(tmpDir))
    , md_(create_model_description(handle_))
    , fmiLogging_(fmiLogging)
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

std::unique_ptr<slave> fmi1_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi1_slave>(ctx_, instanceName, md_, tmpDir_, fmiLogging_);
}

fmi1_fmu::~fmi1_fmu()
{
    fmi1_import_free(handle_);
}


} // namespace fmilibcpp