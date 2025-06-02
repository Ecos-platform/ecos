
#ifndef ECOS_CSV_WRITER_HPP
#define ECOS_CSV_WRITER_HPP

#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace ecos
{

/// Configuration for the CSV writer.
struct csv_config
{

    void load(const std::filesystem::path& configPath);

    void register_variable(variable_identifier v);

    void clear_on_reset(bool flag);

    size_t& decimation_factor();

    [[nodiscard]] bool should_log(const variable_identifier& identifier) const;

private:
    bool clear_on_reset_{true};
    size_t decimationFactor_ = 1;
    std::vector<variable_identifier> variable_register;

    csv_config() = default;

    void report(const std::vector<variable_identifier>& ids) const;

    friend class csv_writer;
};

/**
 * \brief CSV writer for logging simulation data.
 *
 * This class listens to simulation events and writes specified variables to a CSV file.
 */
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

    /// Returns the path to the output CSV file.
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
