
add_executable(ecos main.cpp)
target_link_libraries(ecos PRIVATE libecos CONAN_PKG::boost)

install(
        TARGETS ecos
        DESTINATION ${ECOS_INSTALL_DESTINATIONS}
)
