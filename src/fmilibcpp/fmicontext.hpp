
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include <fmi4c.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace fmilibcpp
{

class fmicontext
{

public:
    fmiHandle* ctx_;

    explicit fmicontext(fmiHandle* handle)
        : ctx_(handle)
    { }

    ~fmicontext()
    {
        std::string unzippedLoc = fmi4c_getUnzippedLocation(ctx_);
        fmi4c_freeFmu(ctx_);

        if (std::filesystem::exists(unzippedLoc)) {
            std::error_code success;
            std::filesystem::remove_all(unzippedLoc, success);
            if (!success) {
                std::cout << "[ecos] Removed lingering folder: " << unzippedLoc << std::endl;
            }

        }
    }
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMICONTEXT_HPP
