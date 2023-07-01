
#ifndef PROXY_PROXY_FMU_FMU_HPP
#define PROXY_PROXY_FMU_FMU_HPP

#include "remote_info.hpp"

#include "ecos/fmi/fmu.hpp"
#include "ecos/fmi/model_description.hpp"
#include "ecos/fmi/slave.hpp"

#include <optional>

namespace ecos::proxyfmu
{

class proxy_fmu : public fmilibcpp::fmu
{

private:
    const std::filesystem::path fmuPath_;
    const fmilibcpp::model_description modelDescription_;

    const std::optional<remote_info> remote_;

public:
    explicit proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote = std::nullopt);

    [[nodiscard]] const fmilibcpp::model_description& get_model_description() const override;

    std::unique_ptr<fmilibcpp::slave> new_instance(const std::string& instanceName) override;

    ~proxy_fmu() override = default;
};

} // namespace proxyfmu::client


#endif // PROXY_PROXY_FMU_FMU_HPP
