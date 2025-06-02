
#ifndef ECOS_PLOTTER_HPP
#define ECOS_PLOTTER_HPP

#include <ecos/variable_identifier.hpp>

#include <filesystem>
#include <optional>
#include <variant>
#include <vector>

namespace ecos
{

struct TLinearTransformation
{
    double offset = 0.0;
    double factor = 1.0;
};

struct TVariable
{
    std::string name;
    std::optional<TLinearTransformation> linearTransformation;
};

struct TComponent
{
    std::string name;
    std::vector<TVariable> variables;
};

struct TTimeSeries
{
    std::vector<TComponent> components;
};

struct TTimeSeriesChart
{
    std::string title;
    std::string label;
    TTimeSeries series;
};

struct TXYSeries
{
    std::string name;
    variable_identifier x;
    variable_identifier y;
    std::optional<std::string> marker;
};

struct TXYSeriesChart
{
    std::string title;
    std::string xLabel;
    std::string yLabel;
    std::vector<TXYSeries> series;
};

// Using a variant-like approach to store either XY or Time series chart
struct TChart
{
    enum class Type
    {
        XYSeries,
        TimeSeries
    } type;
    std::variant<TXYSeriesChart, TTimeSeriesChart> data;

    TChart(const TXYSeriesChart& chart)
        : type(Type::XYSeries)
        , data(chart)
    { }

    TChart(const TTimeSeriesChart& chart)
        : type(Type::TimeSeries)
        , data(chart)
    { }
};

struct TChartConfig
{

    void addChart(TChart chart);

    [[nodiscard]] std::string toXML() const;

private:
    std::vector<TChart> charts;
};

// Function to plot a CSV file with the given chart configuration
void plot_csv(const std::filesystem::path& csvFile, const TChartConfig& config);

// Function to plot a CSV file with a configuration file.
void plot_csv(const std::filesystem::path& csvFile, const std::filesystem::path& plotConfig);

} // namespace ecos

#endif // ECOS_PLOTTER_HPP
