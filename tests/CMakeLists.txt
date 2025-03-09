
function(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE libecos Catch2::Catch2WithMain)
    if (ECOS_WITH_PROXYFMU)
        target_link_libraries(${name} PRIVATE simple_socket) # why is this needed?
    endif ()
    target_include_directories(${name} PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()


add_subdirectory(ecos)
add_subdirectory(fmilibcpp)
#add_subdirectory(reference_fmus) #disabled due to CI issue

if (ECOS_WITH_PROXYFMU)
    add_subdirectory(proxyfmu)
endif ()
