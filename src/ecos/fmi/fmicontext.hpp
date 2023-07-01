
#ifndef ECOS_FMI_FMICONTEXT_HPP
#define ECOS_FMI_FMICONTEXT_HPP

#include <cstring>
#include <fmilib.h>
#include <memory>

namespace
{

void logger_callback(
    jm_callbacks*,
    jm_string module,
    jm_log_level_enu_t logLevel,
    jm_string message)
{
    printf("module = %s, log level = %d: %s\n", module, logLevel, message);
}

void noop_logger_callback(
    jm_callbacks*,
    jm_string,
    jm_log_level_enu_t,
    jm_string)
{
}


std::unique_ptr<jm_callbacks> make_callbacks(bool logging)
{
    auto callbacks = std::make_unique<jm_callbacks>();
    std::memset(callbacks.get(), 0, sizeof(jm_callbacks));
    callbacks->malloc = std::malloc;
    callbacks->calloc = std::calloc;
    callbacks->realloc = std::realloc;
    callbacks->free = std::free;
    if (logging) {
        callbacks->logger = &logger_callback;
    } else {
        callbacks->logger = &noop_logger_callback;
    }
    callbacks->log_level = jm_log_level_error;
    callbacks->context = nullptr;
    std::memset(callbacks->errMessageBuffer, 0, JM_MAX_ERROR_MESSAGE_SIZE);
    return callbacks;
}

} // namespace

namespace fmilibcpp
{

class fmicontext
{
private:
    std::unique_ptr<jm_callbacks> callbacks_;

public:
    fmi_import_context_t* ctx_;

    explicit fmicontext(bool logging = true)
        : callbacks_(make_callbacks(logging))
        , ctx_(fmi_import_allocate_context(callbacks_.get()))
    { }

    ~fmicontext()
    {
        fmi_import_free_context(ctx_);
    }
};

} // namespace fmi

#endif // ECOS_FMI_FMICONTEXT_HPP
