#define BOOST_TEST_MODULE test_property

#include <vico/property.hpp>

#include <boost/test/unit_test.hpp>

using namespace vico;

BOOST_AUTO_TEST_CASE(test_property)
{

    {
        int value = -100;
        property_t<int> p(
            "p",
            [&] { return value; },
            [&](auto v) { value = v; });

        BOOST_CHECK_EQUAL(value, p.get_value());
        p.set_value(value - 1);
        BOOST_CHECK_EQUAL(p.get_value(), -101);
        BOOST_CHECK_EQUAL(value, -101);
    }

    {
        double value = -100;
        property_t<double> p1(
            "p",
            [&] { return value; },
            [&](auto v) { value = v; });

        BOOST_CHECK_CLOSE(value, p1.get_value(), 0.0001);
        p1.set_value(value - 1);
        BOOST_CHECK_CLOSE(p1.get_value(), -101, 0.0001);
        BOOST_CHECK_CLOSE(value, -101, 0.0001);
    }
}
