
set(sources
        "proxyfmu.cpp"
        "${PROJECT_SOURCE_DIR}/src/ecos/util/temp_dir.cpp"
        "${PROJECT_SOURCE_DIR}/src/ecos/util/unzipper.cpp"
        "${PROJECT_SOURCE_DIR}/src/ecos/util/uuid.cpp"
        "${PROJECT_SOURCE_DIR}/src/ecos/logger/logger.cpp"
        "${generatedSourcesDir}/ecos/lib_info.cpp"
)

add_executable(proxyfmu ${sources})
target_include_directories(proxyfmu
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/external/spdlog"
        "${SUBPROCESS_INCLUDE_DIR}"
        "${CLI11_INCLUDE_DIR}"
        "${generatedSourcesDir}")
target_link_libraries(proxyfmu
        PRIVATE
        fmi4c
        simple_socket
        flatbuffers::flatbuffers
        "$<TARGET_OBJECTS:fmilibcpp>"
)
if (UNIX)
    target_link_libraries(proxyfmu PRIVATE pthread)
endif ()
