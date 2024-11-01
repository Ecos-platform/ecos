
set(sources
        "proxyfmu.cpp"
        "${generatedSourcesDir}/ecos/lib_info.cpp"
)

add_executable(proxyfmu ${sources})
target_include_directories(proxyfmu
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${SUBPROCESS_INCLUDE_DIR}"
        "${generatedSourcesDir}")
target_link_libraries(proxyfmu
        PRIVATE
        fmt::fmt
        CLI11::CLI11
        spdlog::spdlog
        flatbuffers::flatbuffers
        simple_socket
        fmi4c
        "$<TARGET_OBJECTS:util>"
        "$<TARGET_OBJECTS:logger>"
        "$<TARGET_OBJECTS:fmilibcpp>"
)
if (UNIX)
    target_link_libraries(proxyfmu PRIVATE pthread)
endif ()
