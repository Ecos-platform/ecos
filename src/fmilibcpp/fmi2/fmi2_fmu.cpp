
#include "fmi2_fmu.hpp"

#include "fmi2_model_description.hpp"
#include "fmi2_slave.hpp"

#include <fmi4c.h>

namespace fmilibcpp
{

fmi2_fmu::fmi2_fmu(std::unique_ptr<fmicontext> ctx, bool fmiLogging)
    : ctx_(std::move(ctx))
    , fmiLogging_(fmiLogging)
    , md_(create_fmi2_model_description(ctx_->get()))
{
    if (!fmi2_getSupportsCoSimulation(ctx_->get())) {
        throw std::runtime_error("FMU does not support Co-simulation!");
    }
}

const model_description& fmi2_fmu::get_model_description() const
{
    return md_;
}

std::unique_ptr<slave> fmi2_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi2_slave>(ctx_, instanceName, md_, fmiLogging_);
}

fmi2_fmu::~fmi2_fmu() = default;


} // namespace fmilibcpp
