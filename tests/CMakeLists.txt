include_directories("${CMAKE_SOURCE_DIR}/src")
link_libraries(libecos Catch2::Catch2)
add_compile_definitions(DATA_FOLDER="${CMAKE_SOURCE_DIR}/data")

add_executable(test_identity test_identity.cpp)
add_test(NAME test_identity COMMAND test_identity)

add_executable(test_controlled_temp test_controlled_temp.cpp)
add_test(NAME test_controlled_temp COMMAND test_controlled_temp)

add_executable(test_variable_identifier test_variable_identifier.cpp)
add_test(NAME test_variable_identifier COMMAND test_variable_identifier)

add_executable(test_connection test_connection.cpp)
add_test(NAME test_connection COMMAND test_connection)

add_executable(test_property test_property.cpp)
add_test(NAME test_property COMMAND test_property)

add_executable(test_runner test_runner.cpp)
add_test(NAME test_runner COMMAND test_runner)

add_executable(test_unzipper test_unzipper.cpp)
add_test(NAME test_unzipper COMMAND test_unzipper)
target_link_libraries(test_unzipper PRIVATE libzip::zip)

if (MSVC AND ECOS_BUILD_CLIB)
    add_executable(test_clib test_clib.cpp)
    add_test(NAME test_clib COMMAND test_clib)
    target_link_libraries(test_clib PUBLIC libecosc)
endif()
