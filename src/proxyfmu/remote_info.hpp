
#ifndef PROXYFMU_REMOTE_INFO_HPP
#define PROXYFMU_REMOTE_INFO_HPP

#include <string>
#include <utility>
#include <cstdint>

namespace ecos::proxy
{

class remote_info
{
public:
    remote_info(std::string host, uint16_t port)
        : host_(std::move(host))
        , port_(port)
    { }

    [[nodiscard]] std::string host() const
    {
        return host_;
    }

    [[nodiscard]] uint16_t port() const
    {
        return port_;
    }

private:
    std::string host_;
    uint16_t port_;
};

} // namespace proxyfmu

#endif // PROXYFMU_REMOTE_INFO_HPP
