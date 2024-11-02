
add_executable(ecos main.cpp)
target_link_libraries(ecos
        PRIVATE
        libecos
)
target_include_directories(ecos
        PRIVATE
        "${CLI11_INCLUDE_DIR}"
)
