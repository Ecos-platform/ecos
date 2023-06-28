
add_executable(ecos main.cpp)
target_link_libraries(ecos
        PRIVATE
        libecos
        CLI11::CLI11
        )

install(
        TARGETS ecos
        DESTINATION ecos-targets}
)
