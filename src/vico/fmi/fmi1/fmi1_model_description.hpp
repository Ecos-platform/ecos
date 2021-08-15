
#ifndef VICO_FMI1_MODEL_DESCRIPTION_HPP
#define VICO_FMI1_MODEL_DESCRIPTION_HPP

#include "vico/fmi/model_description.hpp"

#include <fmilib.h>

namespace vico
{

model_description create_model_description(fmi1_import_t* handle);

} // namespace vico

#endif // VICO_FMI1_MODEL_DESCRIPTION_HPP
