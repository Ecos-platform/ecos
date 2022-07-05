#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/connection.hpp>

using namespace vico;

TEST_CASE("test_real_connection")
{

    double sourceValue = 0;
    double sinkValue = -1;
    property_t<double> source([&] { return ++sourceValue; });
    property_t<double> sink([&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    real_connection c{&source, &sink};

    source.applyGet();
    c.transferData();
    sink.applySet();

    REQUIRE(sourceValue == Approx(1));
    REQUIRE(sourceValue == sinkValue);
}

TEST_CASE("test_int_connection")
{

    int sourceValue = 0;
    int sinkValue = -1;
    property_t<int> source([&] { return ++sourceValue; });
    property_t<int> sink([&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    int_connection c{&source, &sink};

    source.applyGet();
    c.transferData();
    sink.applySet();

    REQUIRE(sourceValue == 1);
    REQUIRE(sourceValue == sinkValue);
}

TEST_CASE("test_bool_connection")
{

    bool sourceValue = false;
    bool sinkValue = false;
    property_t<bool> source([&] {
        sourceValue = !sourceValue;
        return sourceValue;
    });
    property_t<bool> sink([&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    bool_connection c{&source, &sink};

    source.applyGet();
    c.transferData();
    sink.applySet();

    REQUIRE(sourceValue == true);
    REQUIRE(sourceValue == sinkValue);
}

TEST_CASE("test_string_connection")
{

    std::string sourceValue = "0";
    std::string sinkValue;
    property_t<std::string> source([&] {
        sourceValue = std::to_string(std::stoi(sourceValue) + 1);
        return sourceValue;
    });
    property_t<std::string> sink([&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    string_connection c{&source, &sink};

    source.applyGet();
    c.transferData();
    sink.applySet();

    REQUIRE(sourceValue == "1");
    REQUIRE(sourceValue == sinkValue);
}

TEST_CASE("test_double_string_connection")
{

    double sourceValue = 0;
    std::string sinkValue;
    property_t<double> source([&] {
        return ++sourceValue;
    });
    property_t<std::string> sink([&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    connection_te<double, std::string> c{&source, &sink, [](double value) {return std::to_string(value);}};

    source.applyGet();
    c.transferData();
    sink.applySet();

    REQUIRE(sourceValue == Approx(1));
    REQUIRE(std::stoi(sinkValue) == 1);
}
