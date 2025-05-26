
#ifndef ECOS_MODEL_HPP
#define ECOS_MODEL_HPP

#include "ecos/model_instance.hpp"

#include <memory>
#include <optional>

namespace ecos
{

/**
 * \brief Abstract base class for models.
 *
 * A model is akin to an FMU in the context of co-simulation.
 *
 * This class defines the interface for creating model instances, which can be used in simulations.
 * Derived classes should implement the instantiate method to create specific model instances.
 */
class model
{

public:
    virtual std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint = std::nullopt) = 0;

    virtual ~model() = default;
};

} // namespace ecos

#endif // ECOS_MODEL_HPP
