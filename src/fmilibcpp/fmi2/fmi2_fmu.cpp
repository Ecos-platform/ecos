
#include "fmi2_fmu.hpp"

#include "fmi2_model_description.hpp"
#include "fmi2_slave.hpp"

#include <fmilib.h>

namespace fmilibcpp
{

fmi2_fmu::fmi2_fmu(std::shared_ptr<fmicontext> ctx, std::shared_ptr<ecos::temp_dir> tmpDir, bool fmiLogging)
    : handle_(fmi2_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str(), nullptr))
    , ctx_(std::move(ctx))
    , fmiLogging_(fmiLogging)
    , md_(create_model_description(handle_))
    , tmpDir_(std::move(tmpDir))
{
    const auto kind = fmi2_import_get_fmu_kind(handle_);
    if (!(kind == fmi2_fmu_kind_cs || kind == fmi2_fmu_kind_me_and_cs)) {
        throw std::runtime_error("FMU does not support Co-simulation!");
    }
}

const model_description& fmi2_fmu::get_model_description() const
{
    return md_;
}

std::unique_ptr<slave> fmi2_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi2_slave>(ctx_, instanceName, md_, tmpDir_, fmiLogging_);
}

fmi2_fmu::~fmi2_fmu()
{
    fmi2_import_free(handle_);
}


} // namespace fmilibcpp
