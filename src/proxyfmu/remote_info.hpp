
#ifndef PROXYFMU_REMOTE_INFO_HPP
#define PROXYFMU_REMOTE_INFO_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace ecos::proxy
{

class remote_info
{
public:
    remote_info(std::string host, int16_t port)
        : host_(std::move(host))
        , port_(port)
    { }

    [[nodiscard]] std::string host() const
    {
        return host_;
    }

    [[nodiscard]] int16_t port() const
    {
        return port_;
    }

    static remote_info parse(const std::string& address)
    {
        // Find the position of the colon that separates host and port
        std::size_t colon_pos = address.find(':');
        if (colon_pos == std::string::npos) {
            throw std::invalid_argument("Invalid address format, expected 'host:port'");
        }

        // Extract host (part before the colon)
        std::string host = address.substr(0, colon_pos);

        // Extract port (part after the colon) and convert to int16_t
        std::string port_str = address.substr(colon_pos + 1);
        const auto port = static_cast<int16_t>(std::stoi(port_str));  // Convert to int

        return {host, port};
    }

private:
    std::string host_;
    int16_t port_;
};

} // namespace proxyfmu

#endif // PROXYFMU_REMOTE_INFO_HPP
