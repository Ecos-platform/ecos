link_directories(${Boost_INCLUDE_DIRS})
link_libraries(fmilibwrapper ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(test_identity test_identity.cpp)
add_test(NAME test_identity COMMAND test_identity)

add_executable(test_controlled_temp test_controlled_temp.cpp)
add_test(NAME test_controlled_temp COMMAND test_controlled_temp)
