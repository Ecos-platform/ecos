
set(sources
        "proxyfmu.cpp"
        "${PROJECT_SOURCE_DIR}/src/ecos/logger/logger.cpp"
        "${PROJECT_SOURCE_DIR}/src/external/flatbuffers/flatbuffers/util.cpp"
        "${GENERATED_SRC_DIR}/ecos/lib_info.cpp"
)

add_executable(proxyfmu ${sources})
target_include_directories(proxyfmu
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/external/spdlog"
        "${PROJECT_SOURCE_DIR}/src/external/flatbuffers"
        "${THIRD_PARTY_INCLUDE_DIR}"
        "${GENERATED_SRC_DIR}")
target_link_libraries(proxyfmu
        PRIVATE
        fmi4c
        simple_socket
        "$<TARGET_OBJECTS:fmilibcpp>"
)
if (UNIX)
    target_link_libraries(proxyfmu PRIVATE pthread)
endif ()

add_custom_command(TARGET proxyfmu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:proxyfmu>
        ${PY_BINARY_DIR}
        COMMENT "Copying proxyfmu to ${PY_BINARY_DIR}"
)
