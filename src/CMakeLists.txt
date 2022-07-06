set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

set(publicHeaders

        "vico/connection.hpp"
        "vico/model.hpp"
        "vico/model_instance.hpp"
        "vico/property.hpp"
        "vico/simulation.hpp"
        "vico/variable_identifier.hpp"

        "vico/algorithm/algorithm.hpp"
        "vico/algorithm/fixed_step_algorithm.hpp"

        "vico/fmi/fmi_model.hpp"
        "vico/fmi/fmi_model_instance.hpp"

        "vico/proxyfmu/proxy_model.hpp"

        "vico/listeners/simulation_listener.hpp"
        "vico/listeners/csv_writer.hpp"

        "vico/structure/simulation_structure.hpp"

        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

set(privateHeaders

        "vico/util/temp_dir.hpp"
        "vico/util/random.hpp"
        "vico/util/unzipper.hpp"
        "vico/util/uuid.hpp"

        )

set(sources

        "vico/simulation.cpp"
        "vico/listeners/csv_writer.cpp"
        "vico/listeners/simulation_listener.cpp"
        "vico/ssp/ssp_loader.cpp"
        "vico/structure/simulation_structure.cpp"
        "vico/util/temp_dir.cpp"

        )

add_library(vico ${publicHeadersFull} ${publicHeadersFull} ${privateHeaders} ${sources})
target_compile_features(vico PUBLIC "cxx_std_17")
target_include_directories(vico
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(vico PUBLIC CONAN_PKG::fmilibcpp CONAN_PKG::proxyfmu CONAN_PKG::simple-ssp-parser PRIVATE CONAN_PKG::pugixml CONAN_PKG::libzip)
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

