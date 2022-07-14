#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

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

    REQUIRE(sourceValue == Approx(1));
    REQUIRE(sourceValue == sinkValue);
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

    REQUIRE(sourceValue == 1);
    REQUIRE(sourceValue == sinkValue);
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

    REQUIRE(sourceValue == true);
    REQUIRE(sourceValue == sinkValue);
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

    REQUIRE(sourceValue == "1");
    REQUIRE(sourceValue == sinkValue);
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

    REQUIRE(sourceValue == Approx(1));
    REQUIRE(std::stoi(sinkValue) == 1);
}
