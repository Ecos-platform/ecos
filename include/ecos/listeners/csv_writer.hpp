
#ifndef ECOS_CSV_WRITER_HPP
#define ECOS_CSV_WRITER_HPP

#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace ecos
{

struct csv_config
{

    void load(const std::filesystem::path& configPath);

    void register_variable(variable_identifier v);

    void clear_on_reset(bool flag);

    size_t& decimation_factor();

    [[nodiscard]] bool shouldLogInstance(const std::string& instanceName) const;

    [[nodiscard]] bool shouldLogVariable(const std::string& variableName) const;

    void verify(const std::vector<variable_identifier>& ids) const;

private:
    bool clear_on_reset_{true};
    size_t decimationFactor_ = 1;
    std::vector<variable_identifier> variable_register;

    csv_config() = default;

    friend class csv_writer;
};

class csv_writer : public simulation_listener
{

public:
    explicit csv_writer(const std::filesystem::path& path);

    csv_config& config()
    {
        return config_;
    }

    void pre_init(simulation& sim) override;

    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;

    void on_reset() override;

    [[nodiscard]] std::filesystem::path output_path() const
    {
        return path_;
    }

private:
    bool headerWritten{false};
    csv_config config_;
    std::ofstream outFile_;
    std::filesystem::path path_;

    void write_header(const simulation& sim);
};

} // namespace ecos

#endif // ECOS_CSV_WRITER_HPP
