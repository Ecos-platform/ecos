
#ifndef ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP

#include "fmilibcpp/model_description.hpp"

#include <fmi4c.h>

namespace fmilibcpp
{

model_description create_fmi2_model_description(fmuHandle* handle);

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
