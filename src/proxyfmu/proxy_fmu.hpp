
#ifndef PROXY_PROXY_FMU_FMU_HPP
#define PROXY_PROXY_FMU_FMU_HPP

#include "remote_info.hpp"

#include "fmilibcpp/fmu.hpp"
#include "fmilibcpp/model_description.hpp"
#include "fmilibcpp/slave.hpp"

#include <optional>

namespace ecos::proxy
{

class proxy_fmu : public fmilibcpp::fmu
{

public:
    explicit proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote = std::nullopt);

    [[nodiscard]] const fmilibcpp::model_description& get_model_description() const override;

    std::unique_ptr<fmilibcpp::slave> new_instance(const std::string& instanceName) override;

    ~proxy_fmu() override = default;


private:
    const std::filesystem::path fmuPath_;
    const fmilibcpp::model_description modelDescription_;

    const std::optional<remote_info> remote_;
};

} // namespace ecos::proxy


#endif // PROXY_PROXY_FMU_FMU_HPP
