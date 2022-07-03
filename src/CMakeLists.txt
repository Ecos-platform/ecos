set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

set(publicHeaders

        "vico/fmi/algorithm.hpp"
        "vico/fmi/fmi_system.hpp"

        "vico/property.hpp"
        "vico/simulation.hpp"
        "vico/simulation_listener.hpp"

        "vico/structure/simulation_structure.hpp"
        "vico/structure/variable_identifier.hpp"

        "vico/util/temp_dir.hpp"
        "vico/util/fs_portability.hpp"
        "vico/util/uuid.hpp"
        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

set(privateHeaders

        "../include/vico/structure/variable_identifier.hpp"

        "vico/util/random.hpp"
        "vico/util/unzipper.hpp"

        )

set(sources

        "vico/simulation.cpp"
        "vico/structure/simulation_structure.cpp"

        "vico/ssp/ssp_loader.cpp"

        "vico/util/temp_dir.cpp"
        "vico/util/uuid.cpp"
        )

add_library(vico ${publicHeadersFull} ${publicHeadersFull} ${privateHeaders} ${sources})
target_compile_features(vico PUBLIC "cxx_std_17")
target_include_directories(vico
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(vico PUBLIC CONAN_PKG::fmilibcpp CONAN_PKG::simple-ssp-parser PRIVATE CONAN_PKG::pugixml CONAN_PKG::libzip)
if (UNIX)
    target_link_libraries(vico INTERFACE stdc++fs)
endif ()


# ==============================================================================
# Installation rules
# ==============================================================================

install(
        TARGETS vico
        EXPORT "${VICO_EXPORT_TARGET}"
        ${VICO_INSTALL_DESTINATIONS}
)
install(
        DIRECTORY "${publicHeaderDir}/vico"
        DESTINATION "${VICO_HEADER_INSTALL_DIR}"
)

