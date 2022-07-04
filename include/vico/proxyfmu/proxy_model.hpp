

#ifndef VICO_PROXY_MODEL_HPP
#define VICO_PROXY_MODEL_HPP

#include "vico/model.hpp"
#include "proxyfmu/client/proxy_fmu.hpp"
#include "vico/util/fs_portability.hpp"

namespace vico
{

class proxy_model : public model
{
public:
    proxy_model(const fs::path& fmuPath)
    {

    }

};

} // namespace vico

#endif // VICO_PROXY_MODEL_HPP
