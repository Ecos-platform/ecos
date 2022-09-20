

#ifndef ECOS_CSV_WRITER_HPP
#define ECOS_CSV_WRITER_HPP

#include "simulation_listener.hpp"

#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

namespace ecos
{

struct csv_config
{

    void load(const std::filesystem::path& configPath);

    void register_variable(variable_identifier v)
    {
        variable_register.emplace_back(std::move(v));
    }

    void enable_plotting(const std::filesystem::path& plotConfig);

    [[nodiscard]] bool shouldLogInstance(const std::string& instanceName) const;

    [[nodiscard]] bool shouldLogVariable(const std::string& variableName) const;

    void verify(const std::vector<variable_identifier>& ids);

private:
    std::optional<std::filesystem::path> plotConfig_;
    std::vector<variable_identifier> variable_register;

    csv_config() = default;

    friend class csv_writer;
};

class csv_writer : public simulation_listener
{

public:
    explicit csv_writer(const std::filesystem::path& path);

    csv_config& config() {
        return config_;
    }

    void pre_init(simulation& sim) override;

    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;

private:
    csv_config config_;
    std::ofstream outFile_;
    std::filesystem::path path_;

};

} // namespace ecos

#endif // ECOS_CSV_WRITER_HPP
