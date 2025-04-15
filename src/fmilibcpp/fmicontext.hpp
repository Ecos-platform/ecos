
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include "util/temp_dir.hpp"

#include <fmi4c.h>

#include <memory>

namespace fmilibcpp
{

class fmicontext
{

public:
    explicit fmicontext(fmuHandle* handle, std::unique_ptr<ecos::temp_dir> unzippedFmu)
        : fmuHandle_(handle)
        , unzippedFmu_(std::move(unzippedFmu))
    { }

    fmicontext(const fmicontext& rhs) = delete;
    fmicontext& operator=(const fmicontext& rhs) = delete;
    fmicontext(fmicontext&& rhs) = delete;
    fmicontext& operator=(fmicontext&& rhs) = delete;

    fmuHandle* get() const { return fmuHandle_; }

    ~fmicontext()
    {
        fmi4c_freeFmu(fmuHandle_);
    }

private:
    fmuHandle* fmuHandle_;
    std::unique_ptr<ecos::temp_dir> unzippedFmu_;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMICONTEXT_HPP
