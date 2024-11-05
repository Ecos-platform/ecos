
add_executable(ecos main.cpp)
target_link_libraries(ecos
        PRIVATE
        libecos
)
target_include_directories(ecos
        PRIVATE
        "${THIRD_PARTY_INCLUDE_DIR}"
)
