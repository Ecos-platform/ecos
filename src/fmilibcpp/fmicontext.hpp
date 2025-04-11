
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include <fmi4c.h>

#include <memory>

#include "ecos/util/temp_dir.hpp"

namespace fmilibcpp
{

class fmicontext
{

public:
    fmiHandle* handle_;

    explicit fmicontext(fmiHandle* handle, std::unique_ptr<ecos::temp_dir> unzippedFmu)
        : handle_(handle), unzippedFmu_(std::move(unzippedFmu))
    { }

    fmicontext(const fmicontext& rhs) = delete;
    fmicontext& operator=(const fmicontext& rhs) = delete;
    fmicontext(fmicontext&& rhs) = delete;
    fmicontext& operator=(fmicontext&& rhs) = delete;

    ~fmicontext()
    {
        fmi4c_freeFmu(handle_);
    }

private:
    std::unique_ptr<ecos::temp_dir> unzippedFmu_;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMICONTEXT_HPP
