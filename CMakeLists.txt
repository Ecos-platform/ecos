cmake_minimum_required(VERSION 3.15)
file(STRINGS "${CMAKE_SOURCE_DIR}/version.txt" projectVersion)
project("vico" VERSION ${projectVersion})
message("Current ${PROJECT_NAME} version: ${CMAKE_PROJECT_VERSION}\n")

# ==============================================================================
# Build settings
# ==============================================================================

option(BUILD_SHARED_LIBS OFF)
option(VICO_BUILD_TESTS "Build tests" ON)
option(VICO_BUILD_EXAMPLES "Build examples" OFF)

# ==============================================================================
# Global internal configuration
# ==============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Prepare for export and installation.
set(VICO_HEADER_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(VICO_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

set(VICO_INSTALL_DESTINATIONS
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib"
        RUNTIME DESTINATION "bin"
        INCLUDES DESTINATION "${VICO_HEADER_INSTALL_DIR}")
set(VICO_EXPORT_TARGET "${PROJECT_NAME}-targets")


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

    # Force add remotes
    conan_add_remote(NAME ais URL https://ais.jfrog.io/artifactory/api/conan/ais-conan-local)
    conan_add_remote(NAME osp URL https://osp.jfrog.io/artifactory/api/conan/conan-local)

    conan_cmake_run(
            CONANFILE conanfile.py
            BUILD missing
            BASIC_SETUP CMAKE_TARGETS)

endif ()

# ==============================================================================
# Targets
# ==============================================================================

add_subdirectory(src)

if (VICO_BUILD_EXAMPLES OR VICO_BUILD_TESTS)
    file(COPY data/fmus DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    if (VICO_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()

    if (VICO_BUILD_TESTS)

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
