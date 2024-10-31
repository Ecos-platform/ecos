
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include <fmi4c.h>

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
        fmi4c_freeFmu(ctx_);
    }
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMICONTEXT_HPP
