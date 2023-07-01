
#ifndef ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP

#include "fmilibcpp/model_description.hpp"

#include <fmilib.h>

namespace fmilibcpp
{

model_description create_model_description(fmi2_import_t* handle);

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
