
#ifndef ECOS_FMI_FMI1_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_FMI1_MODEL_DESCRIPTION_HPP

#include "ecos/fmi/model_description.hpp"

#include <fmilib.h>

namespace ecos::fmi
{

model_description create_model_description(fmi1_import_t* handle);

} // namespace ecos::fmi

#endif // ECOS_FMI_FMI1_MODEL_DESCRIPTION_HPP
