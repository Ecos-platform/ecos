#include "fmi1_fmu.hpp"

#include "fmi1_model_description.hpp"
#include "fmi1_slave.hpp"

namespace fmilibcpp
{

fmi1_fmu::fmi1_fmu(std::unique_ptr<fmicontext> ctx, bool fmiLogging)
    : handle_(ctx->handle_)
    , ctx_(std::move(ctx))
    , fmiLogging_(fmiLogging)
    , md_(create_fmi1_model_description(handle_))
{
    const auto kind = fmi1_getType(handle_);
    if (kind != fmi1CoSimulationTool && kind != fmi1CoSimulationStandAlone) {
        throw std::runtime_error("FMU does not support Co-simulation!");
    }
}

const model_description& fmi1_fmu::get_model_description() const
{
    return md_;
}

std::unique_ptr<slave> fmi1_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi1_slave>(ctx_, instanceName, md_, fmiLogging_);
}

fmi1_fmu::~fmi1_fmu() = default;


} // namespace fmilibcpp
