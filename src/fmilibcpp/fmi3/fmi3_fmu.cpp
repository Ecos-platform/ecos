
#include "fmi3_fmu.hpp"

#include "fmi3_model_description.hpp"
#include "fmi3_slave.hpp"

#include <fmi4c.h>

namespace fmilibcpp
{

fmi3_fmu::fmi3_fmu(std::unique_ptr<fmicontext> ctx, bool fmiLogging)
    : ctx_(std::move(ctx))
    , fmiLogging_(fmiLogging)
    , md_(create_fmi3_model_description(ctx_->get()))
{
    if (!fmi3_supportsCoSimulation(ctx_->get())) {
        throw std::runtime_error("FMU does not support Co-simulation!");
    }
}

const model_description& fmi3_fmu::get_model_description() const
{
    return md_;
}

std::unique_ptr<slave> fmi3_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<fmi3_slave>(ctx_, instanceName, md_, fmiLogging_);
}

fmi3_fmu::~fmi3_fmu() = default;


} // namespace fmilibcpp
