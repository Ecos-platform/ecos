
#ifndef ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP

#include "ecos/fmi/model_description.hpp"

#include <fmilib.h>

namespace ecos::fmi
{

model_description create_model_description(fmi2_import_t* handle);

} // namespace fmi

#endif // ECOS_FMI_FMI2_MODEL_DESCRIPTION_HPP
