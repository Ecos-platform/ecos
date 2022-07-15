
add_executable(ecos main.cpp)
target_link_libraries(ecos PRIVATE libecos CONAN_PKG::boost CONAN_PKG::spdlog)
