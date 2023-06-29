#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <ecos/connection.hpp>

using namespace ecos;

TEST_CASE("test_real_connection")
{

    double sourceValue = 0;
    double sinkValue = -1;
    property_t<double> source({"::source"}, [&] { return ++sourceValue; });
    property_t<double> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    real_connection c{&source, &sink};

    c.transferData();
    sink.applySet();

    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(1.));
    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(sinkValue));
}

TEST_CASE("test_int_connection")
{

    int sourceValue = 0;
    int sinkValue = -1;
    property_t<int> source({"::source"}, [&] { return ++sourceValue; });
    property_t<int> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    int_connection c{&source, &sink};

    c.transferData();
    sink.applySet();

    CHECK(sourceValue == 1);
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_bool_connection")
{

    bool sourceValue = false;
    bool sinkValue = false;
    property_t<bool> source({"::source"}, [&] {
        sourceValue = !sourceValue;
        return sourceValue;
    });
    property_t<bool> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    bool_connection c{&source, &sink};

    c.transferData();
    sink.applySet();

    CHECK(sourceValue == true);
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_string_connection")
{

    std::string sourceValue = "0";
    std::string sinkValue;
    property_t<std::string> source({"::source"}, [&] {
        sourceValue = std::to_string(std::stoi(sourceValue) + 1);
        return sourceValue;
    });
    property_t<std::string> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    string_connection c{&source, &sink};

    c.transferData();
    sink.applySet();

    CHECK(sourceValue == "1");
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_double_string_connection")
{

    double sourceValue = 0;
    std::string sinkValue;
    property_t<double> source({"::source"}, [&] {
        return ++sourceValue;
    });
    property_t<std::string> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    connection_te<double, std::string> c{&source, &sink, [](double value) { return std::to_string(value); }};

    c.transferData();
    sink.applySet();

    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(1.));
    CHECK_THAT(std::stoi(sinkValue), Catch::Matchers::WithinRel(1.));
}
