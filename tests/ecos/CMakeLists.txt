
add_test_executable(test_identity)
add_test_executable(test_controlled_temp)
add_test_executable(test_variable_identifier)
add_test_executable(test_connection)
add_test_executable(test_property)
add_test_executable(test_runner)
add_test_executable(test_unzipper)
target_link_libraries(test_unzipper PRIVATE libzip::zip)

if (MSVC AND ECOS_BUILD_CLIB)
    add_test_executable(test_clib)
    target_link_libraries(test_clib PUBLIC libecosc)
endif()