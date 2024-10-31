
function(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE libecos simple_socket Catch2::Catch2WithMain)
    target_include_directories(${name} PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()


add_subdirectory(ecos)
add_subdirectory(proxyfmu)
add_subdirectory(fmilibcpp)
add_subdirectory(misc)
#add_subdirectory(reference_fmus)
