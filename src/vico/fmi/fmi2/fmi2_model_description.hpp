
#ifndef VICO_FMI2_MODEL_DESCRIPTION_HPP
#define VICO_FMI2_MODEL_DESCRIPTION_HPP

#include "vico/model_description.hpp"

#include <fmilib.h>

namespace vico
{

model_description create_model_description(fmi2_import_t* handle);

} // namespace vico

#endif // VICO_FMI2_MODEL_DESCRIPTION_HPP
