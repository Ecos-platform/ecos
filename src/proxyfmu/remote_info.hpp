
#ifndef PROXYFMU_REMOTE_INFO_HPP
#define PROXYFMU_REMOTE_INFO_HPP

#include <string>
#include <utility>

namespace proxyfmu
{

class remote_info
{
public:
    remote_info(std::string host, int port)
        : host_(std::move(host))
        , port_(port)
    { }

    [[nodiscard]] std::string host() const
    {
        return host_;
    }

    [[nodiscard]] int port() const
    {
        return port_;
    }

private:
    std::string host_;
    int port_;
};

} // namespace proxyfmu

#endif // PROXYFMU_REMOTE_INFO_HPP
