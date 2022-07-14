

#ifndef ECOS_CSV_WRITER_HPP
#define ECOS_CSV_WRITER_HPP

#include "simulation_listener.hpp"

#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <fstream>
#include <optional>

namespace ecos
{

struct csv_config
{

    void log_variable(variable_identifier v)
    {
        variablesToLog_.emplace_back(std::move(v));
    }

    [[nodiscard]] bool shouldLogInstance(const std::string& instanceName) const;

    [[nodiscard]] bool shouldLogVariable(const std::string& variableName) const;

    void verify(const std::vector<variable_identifier>& ids);

    static csv_config parse(const std::filesystem::path& configPath);

private:
    std::vector<variable_identifier> variablesToLog_;
};

class csv_writer : public simulation_listener
{

public:
    explicit csv_writer(const std::filesystem::path& path, std::optional<csv_config> config = std::nullopt);

    explicit csv_writer(const std::filesystem::path& path, const std::filesystem::path& configPath);

    void enable_plotting(const std::filesystem::path& plotConfig);

    void pre_init(simulation& sim) override;

    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;

private:
    std::ofstream outFile_;
    std::filesystem::path path_;
    std::optional<csv_config> config_;
    std::optional<std::filesystem::path> plotConfig_;
};

} // namespace ecos

#endif // ECOS_CSV_WRITER_HPP
