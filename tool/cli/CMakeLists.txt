
add_executable(ecos main.cpp)
target_link_libraries(ecos
        PRIVATE
        libecos
)
target_include_directories(ecos
        PRIVATE
        "${THIRD_PARTY_INCLUDE_DIR}"
)

add_custom_command(TARGET ecos POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ecos>
        ${PY_BINARY_DIR}
        COMMENT "Copying ecos to ${PY_BINARY_DIR}"
)
