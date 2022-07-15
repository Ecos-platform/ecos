
add_executable(ecos-cli main.cpp)
target_link_libraries(ecos-cli PRIVATE ecos CONAN_PKG::boost)
