
#ifndef VICO_FMICONTEXT_HPP
#define VICO_FMICONTEXT_HPP

#include <cstring>
#include <fmilib.h>
#include <memory>

namespace
{

void logger_callback(
    jm_callbacks* /*callbacks*/,
    jm_string module,
    jm_log_level_enu_t logLevel,
    jm_string message)
{
    printf("module = %s, log level = %d: %s\n", module, logLevel, message);
}


std::unique_ptr<jm_callbacks> make_callbacks()
{
    auto callbacks = std::make_unique<jm_callbacks>();
    std::memset(callbacks.get(), 0, sizeof(jm_callbacks));
    callbacks->malloc = std::malloc;
    callbacks->calloc = std::calloc;
    callbacks->realloc = std::realloc;
    callbacks->free = std::free;
    callbacks->logger = &logger_callback;
    callbacks->log_level = jm_log_level_error;
    callbacks->context = nullptr;
    std::memset(callbacks->errMessageBuffer, 0, JM_MAX_ERROR_MESSAGE_SIZE);
    return callbacks;
}

} // namespace

namespace vico
{

class fmicontext
{
private:
    std::unique_ptr<jm_callbacks> callbacks_;

public:
    fmi_import_context_t* ctx_;

    fmicontext()
        : callbacks_(make_callbacks())
        , ctx_(fmi_import_allocate_context(callbacks_.get()))
    { }

    ~fmicontext()
    {
        fmi_import_free_context(ctx_);
    }
};

} // namespace vico

#endif // VICO_FMICONTEXT_HPP
