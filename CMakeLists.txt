cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_SOURCE_DIR}/version.txt" projectVersion)
project("libecos" VERSION ${projectVersion})
message("Current ${PROJECT_NAME} version: ${CMAKE_PROJECT_VERSION}\n")


# ==============================================================================
# Build settings
# ==============================================================================

option(BUILD_SHARED_LIBS OFF)
option(ECOS_BUILD_TESTS "Build tests" ON)
option(ECOS_BUILD_EXAMPLES "Build examples" OFF)
option(ECOS_BUILD_CLI "Build CLI" ON)


# ==============================================================================
# Global internal configuration
# ==============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

# Prepare for export and installation.
set(ECOS_HEADER_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(ECOS_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

set(ECOS_INSTALL_DESTINATIONS
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib"
        RUNTIME DESTINATION "bin"
        INCLUDES DESTINATION "${ECOS_HEADER_INSTALL_DIR}")
set(ECOS_EXPORT_TARGET "${PROJECT_NAME}-targets")

file(COPY resources/plot/ecos_plotter.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)


# ==============================================================================
# Dependencies
# ==============================================================================

if (CONAN_EXPORTED) # in conan local cache
    # standard conan installation, deps will be defined in conanfile.py
    # and not necessary to call conan again, conan is already running
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
else ()

    include(cmake/conan.cmake)
    conan_check(REQUIRED)

    conan_cmake_autodetect(settings)

    # Force add remotes
    conan_add_remote(NAME ais URL https://ais.jfrog.io/artifactory/api/conan/ais-conan-local)

    conan_cmake_install(PATH_OR_REFERENCE ..
            BUILD missing
            SETTINGS ${settings})

    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)

endif ()


# ==============================================================================
# Targets
# ==============================================================================

add_subdirectory(src)

if (ECOS_BUILD_CLI)
    add_subdirectory(cli)
endif ()

if (ECOS_BUILD_EXAMPLES OR ECOS_BUILD_TESTS)

    if (ECOS_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()

    if (ECOS_BUILD_TESTS)

        Include(FetchContent)

        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v2.13.8
        )

        FetchContent_MakeAvailable(Catch2)

        enable_testing()
        add_subdirectory(tests)
    endif ()

endif ()
