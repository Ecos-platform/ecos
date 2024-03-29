cmake_minimum_required(VERSION 3.17)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
file(STRINGS "${CMAKE_SOURCE_DIR}/version.txt" projectVersion)
project("libecos" VERSION ${projectVersion})
message("Current ${PROJECT_NAME} version: ${CMAKE_PROJECT_VERSION}\n")


# ==============================================================================
# Build settings
# ==============================================================================

option(BUILD_SHARED_LIBS OFF)
option(ECOS_BUILD_TESTS "Build tests" OFF)
option(ECOS_BUILD_EXAMPLES "Build examples" OFF)
option(ECOS_BUILD_CLI "Build CLI" ON)
option(ECOS_BUILD_CLIB "Build C library" ON)


# ==============================================================================
# Global internal configuration
# ==============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(generatedSourcesDir "${CMAKE_BINARY_DIR}/generated")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

# ==============================================================================
# Dependencies
# ==============================================================================

set(SUBPROCESS_INCLUDE_DIR "${CMAKE_BINARY_DIR}/thirdparty")
file(DOWNLOAD https://raw.githubusercontent.com/Ecos-platform/subprocess.h/master/subprocess.h
        "${SUBPROCESS_INCLUDE_DIR}/subprocess/subprocess.h")

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(Thrift CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(unofficial-fmilib CONFIG REQUIRED)

# ==============================================================================
# Targets
# ==============================================================================

add_subdirectory(src)
add_subdirectory(tool)

if (ECOS_BUILD_EXAMPLES OR ECOS_BUILD_TESTS)

    if (ECOS_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()

    if (ECOS_BUILD_TESTS)

        include(FetchContent)
        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.3.2
        )
        FetchContent_MakeAvailable(Catch2)

        enable_testing()
        add_subdirectory(tests)
    endif ()

endif ()


# ==============================================================================
# Exports and remaining installation
# ==============================================================================

configure_package_config_file(cmake/config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/ecos
        NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config-version.cmake
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ecos-config-version.cmake
        DESTINATION
        ${CMAKE_INSTALL_DATADIR}/ecos)

install(
        TARGETS libecos libecosc ecos proxyfmu
        EXPORT ecos-targets
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ecos-targets
        NAMESPACE ecos::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/ecos)
