
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include <fmi4c.h>

#include <filesystem>
#include <string>

#include "ecos/logger/logger.hpp"

namespace fmilibcpp
{

class fmicontext
{

public:
    fmiHandle* ctx_;

    explicit fmicontext(fmiHandle* handle)
        : ctx_(handle)
    { }

    fmicontext(const fmicontext& rhs) = delete;
    fmicontext& operator=(const fmicontext& rhs) = delete;
    fmicontext(fmicontext&& rhs) = delete;
    fmicontext& operator=(fmicontext&& rhs) = delete;

    ~fmicontext()
    {
        std::string unzippedLoc = fmi4c_getUnzippedLocation(ctx_);
        fmi4c_freeFmu(ctx_);

        if (std::filesystem::exists(unzippedLoc)) {
            std::error_code success;
            std::filesystem::remove_all(unzippedLoc, success);
            if (!success) {
                spdlog::debug("Removed lingering folder: {}", unzippedLoc);
            }
        }
    }
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMICONTEXT_HPP
