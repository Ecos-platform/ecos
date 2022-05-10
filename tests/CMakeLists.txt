include_directories("${CMAKE_SOURCE_DIR}/src")
link_libraries(vico CONAN_PKG::fmilibcpp Catch2::Catch2)

add_executable(test_identity test_identity.cpp)
add_test(NAME test_identity COMMAND test_identity)

add_executable(test_controlled_temp test_controlled_temp.cpp)
add_test(NAME test_controlled_temp COMMAND test_controlled_temp)

add_executable(test_variable_identifier test_variable_identifier.cpp)
add_test(NAME test_variable_identifier COMMAND test_variable_identifier)

add_executable(test_property test_property.cpp)
add_test(NAME test_property COMMAND test_property)

add_executable(basic_test basic_test.cpp)
add_test(NAME basic_test COMMAND basic_test)

add_executable(quarter_truck quarter_truck.cpp)
add_test(NAME quarter_truck COMMAND quarter_truck)
