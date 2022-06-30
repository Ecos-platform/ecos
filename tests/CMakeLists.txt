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

add_executable(test_unzipper test_unzipper.cpp)
add_test(NAME test_unzipper COMMAND test_unzipper)
target_link_libraries(test_unzipper PRIVATE CONAN_PKG::libzip)

add_executable(test_ssp_parser test_ssp_parser.cpp)
add_test(NAME test_ssp_parser COMMAND test_ssp_parser)
target_link_libraries(test_ssp_parser PRIVATE CONAN_PKG::libzip CONAN_PKG::pugixml)
