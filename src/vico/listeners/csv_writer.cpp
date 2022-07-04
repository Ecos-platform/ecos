
#include "vico/listeners/csv_writer.hpp"

#include "vico/simulation.hpp"


using namespace vico;

namespace
{

void writeData(std::ofstream& out, simulation& sim)
{
    out << sim.iterations() << ", " << sim.time();

    for (auto& instance : sim.get_instances()) {
        auto& properties = instance->get_properties();

        for (auto& [name, p] : properties.get_reals()) {
            out << ", " << std::to_string(p->get_value());
        }
        for (auto& [name, p] : properties.get_integers()) {
            out << ", " << std::to_string(p->get_value());
        }
        for (auto& [name, p] : properties.get_booleans()) {
            out << ", " << std::noboolalpha << p->get_value();
        }
        for (auto& [name, p] : properties.get_strings()) {
            out << ", " << p->get_value();
        }
    }
    out << "\n";
    out.flush();
}

} // namespace

csv_writer::csv_writer(const fs::path& path)
{
    if (path.extension().string() != ".csv") {
        throw std::runtime_error("File extension must be .csv, was: " + path.extension().string());
    }

    const auto parentPath = fs::absolute(path).parent_path();
    if (!fs::exists(parentPath)) {
        if (!fs::create_directories(parentPath)) {
            throw std::runtime_error("Unable to create missing directories for path: " + fs::absolute(path).string());
        }
    }

    outFile_.open(path.string());
}

void csv_writer::pre_init(simulation& sim)
{
    outFile_ << "iterations, time";

    for (auto& instance : sim.get_instances()) {
        auto& properties = instance->get_properties();

        for (auto& [name, p] : properties.get_reals()) {
            outFile_ << ", " << name << "[REAL]";
        }
        for (auto& [name, p] : properties.get_integers()) {
            outFile_ << ", " << name << "[INT]";
        }
        for (auto& [name, p] : properties.get_booleans()) {
            outFile_ << ", " << name << "[BOOL]";
        }
        for (auto& [name, p] : properties.get_strings()) {
            outFile_ << ", " << name << "[STR]";
        }
    }

    outFile_ << "\n";
    outFile_.flush();
}

void csv_writer::post_init(simulation& sim)
{
    writeData(outFile_, sim);
}

void csv_writer::post_step(simulation& sim)
{
    writeData(outFile_, sim);
}

void csv_writer::post_terminate(simulation& sim)
{
    outFile_.close();
}
