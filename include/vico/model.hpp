
#ifndef VICO_MODEL_HPP
#define VICO_MODEL_HPP

#include <memory>
#include "model_instance.hpp"

namespace vico
{

class model {

public:
    virtual std::unique_ptr<model_instance> instantiate(std::string instanceName) = 0;

    virtual ~model() = default;
};

}

#endif // VICO_MODEL_HPP