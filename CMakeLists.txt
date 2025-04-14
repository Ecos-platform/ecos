cmake_minimum_required(VERSION 3.19)
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/version.txt" PROJECT_VERSION)
project("libecos" VERSION ${PROJECT_VERSION})
message("Current ${PROJECT_NAME} version: ${CMAKE_PROJECT_VERSION}\n")


# ==============================================================================
# Build settings
# ==============================================================================

option(BUILD_SHARED_LIBS OFF)
option(ECOS_BUILD_TESTS "Build tests" OFF)
option(ECOS_BUILD_EXAMPLES "Build examples" OFF)
option(ECOS_BUILD_CLI "Build CLI" ON)
option(ECOS_BUILD_CLIB "Build C library (required for ecospy)" ON)
option(ECOS_WITH_PROXYFMU "Build with proxyfmu" ON)


# ==============================================================================
# Global internal configuration
# ==============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(GENERATED_SRC_DIR "${CMAKE_BINARY_DIR}/generated")
set(PY_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ecospy/binaries")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif ()


# ==============================================================================
# Dependencies
# ==============================================================================

set(THIRD_PARTY_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty")
file(DOWNLOAD https://raw.githubusercontent.com/Ecos-platform/subprocess.h/master/subprocess.h
        "${THIRD_PARTY_INCLUDE_DIR}/subprocess/subprocess.h")

if (ECOS_BUILD_CLI)
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v2.4.2/CLI11.hpp
        "${THIRD_PARTY_INCLUDE_DIR}/cli11/CLI11.h")
endif ()

include(FetchContent)

if (ECOS_WITH_PROXYFMU)
    set(SIMPLE_SOCKET_BUILD_TESTS OFF)
    FetchContent_Declare(
            SimpleSocket
            GIT_REPOSITORY https://github.com/markaren/SimpleSocket.git
            GIT_TAG e0e989bdd52137d743a4599e25167d5681a0de00
    )
    FetchContent_MakeAvailable(SimpleSocket)
endif ()

set(FMI4C_BUILD_SHARED OFF)
FetchContent_Declare(
        fmi4c
        GIT_REPOSITORY https://github.com/Ecos-platform/fmi4c.git
        GIT_TAG 7e73f9fb20b925665b2393e7281f65a525792717
)
FetchContent_MakeAvailable(fmi4c)


# ==============================================================================
# Targets
# ==============================================================================

add_subdirectory(src)
add_subdirectory(tool)


if (ECOS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (ECOS_BUILD_TESTS)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
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
        TARGETS libecos
        EXPORT ecos-targets
)
if (ECOS_BUILD_CLI)
    install(
            TARGETS ecos
            EXPORT ecos-targets
    )
endif ()
if (ECOS_BUILD_CLIB)
    install(
            TARGETS libecosc
            EXPORT ecos-targets
    )
endif ()
if (ECOS_WITH_PROXYFMU)
    install(
            TARGETS proxyfmu
            EXPORT ecos-targets
    )
endif ()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ecos-targets
        NAMESPACE ecos::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/ecos)
