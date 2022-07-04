

#ifndef VICO_CSV_WRITER_HPP
#define VICO_CSV_WRITER_HPP

#include "simulation_listener.hpp"

#include "vico/util/fs_portability.hpp"
#include "vico/variable_identifier.hpp"

#include <fstream>
#include <optional>

namespace vico
{

struct csv_config
{

    void log_variable(const std::string& v)
    {
        log_variable(variable_identifier{v});
    }

    void log_variable(variable_identifier v)
    {
        variablesToLog_.emplace_back(std::move(v));
    }

    [[nodiscard]] bool shouldLogInstance(const std::string& instanceName) const
    {
        bool log = std::find_if(variablesToLog_.begin(), variablesToLog_.end(), [instanceName](const variable_identifier& v) {
            return v.instanceName == instanceName;
        }) != std::end(variablesToLog_);
        return log;
    }

    [[nodiscard]] bool shouldLogVariable(const std::string& variableName) const
    {
        bool log = std::find_if(variablesToLog_.begin(), variablesToLog_.end(), [variableName](const variable_identifier& v) {
            return v.variableName == variableName;
        }) != std::end(variablesToLog_);
        return log;
    }

private:
    std::vector<variable_identifier> variablesToLog_;
};

class csv_writer : public simulation_listener
{

public:
    explicit csv_writer(const fs::path& path, std::optional<csv_config> config = std::nullopt);

    void pre_init(simulation& sim) override;

    void post_init(simulation& sim) override;

    void post_step(simulation& sim) override;

    void post_terminate(simulation& sim) override;

private:
    std::ofstream outFile_;
    std::optional<csv_config> config_;
};

} // namespace vico

#endif // VICO_CSV_WRITER_HPP
