
#ifndef VICO_MODEL_HPP
#define VICO_MODEL_HPP

#include "model_instance.hpp"

#include <memory>

namespace vico
{

class model
{

public:
    virtual std::unique_ptr<model_instance> instantiate(const std::string& instanceName) = 0;

    virtual ~model() = default;
};

} // namespace vico

#endif // VICO_MODEL_HPP
