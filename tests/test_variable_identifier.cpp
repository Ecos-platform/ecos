#define BOOST_TEST_MODULE test_variable_identifier

#include <vico/model.hpp>
#include <vico/variable_identifier.hpp>
#include <boost/test/unit_test.hpp>

using namespace vico;

BOOST_AUTO_TEST_CASE(test_variable_identifier)
{

    std::string str("instanceName.nested.variableName");

    auto v = variable_identifier::parse(str);

    BOOST_CHECK_EQUAL("instanceName", v.instanceName);
    BOOST_CHECK_EQUAL("nested.variableName", v.variableName);
}
