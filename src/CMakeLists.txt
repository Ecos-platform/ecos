set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

set(publicHeaders

        "ecos/connection.hpp"
        "ecos/model.hpp"
        "ecos/model_instance.hpp"
        "ecos/model_resolver.hpp"
        "ecos/property.hpp"
        "ecos/scenario.hpp"
        "ecos/scenario_loader.hpp"
        "ecos/simulation.hpp"
        "ecos/simulation_runner.hpp"
        "ecos/variable_identifier.hpp"

        "ecos/algorithm/algorithm.hpp"
        "ecos/algorithm/fixed_step_algorithm.hpp"

        "ecos/listeners/simulation_listener.hpp"
        "ecos/listeners/csv_writer.hpp"

        "ecos/structure/simulation_structure.hpp"

        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

set(privateHeaders

        "ecos/fmi/fmi_model.hpp"
        "ecos/fmi/fmi_model_instance.hpp"
        "ecos/fmi/fmi_model_sub_resolver.hpp"
        "ecos/fmi/proxy/proxy_model.hpp"
        "ecos/fmi/proxy/proxy_model_sub_resolver.hpp"

        "ecos/util/temp_dir.hpp"
        "ecos/util/unzipper.hpp"
        "ecos/util/uuid.hpp"

        )

set(sources

        "ecos/model_resolver.cpp"
        "ecos/scenario/scenario.cpp"
        "ecos/scenario/scenario_loader_xml.cpp"
        "ecos/simulation.cpp"
        "ecos/simulation_runner.cpp"
        "ecos/algorithm/fixed_step_algorithm.cpp"
        "ecos/fmi/fmi_model_sub_resolver.cpp"
        "ecos/fmi/proxy/proxy_model_sub_resolver.cpp"
        "ecos/listeners/csv_writer.cpp"
        "ecos/listeners/simulation_listener.cpp"
        "ecos/ssp/ssp_loader.cpp"
        "ecos/structure/simulation_structure.cpp"
        "ecos/util/temp_dir.cpp"
        "ecos/util/unzipper.cpp"
        "ecos/util/uuid.cpp"

        )

add_library(libecos ${publicHeadersFull} ${publicHeadersFull} ${privateHeaders} ${sources})
target_compile_features(libecos PUBLIC "cxx_std_17")
target_include_directories(libecos
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(libecos
        PRIVATE
        CONAN_PKG::fmilibcpp
        CONAN_PKG::proxyfmu
        CONAN_PKG::spdlog
        CONAN_PKG::pugixml
        CONAN_PKG::libzip
        CONAN_PKG::simple-ssp-parser)
if (UNIX)
    target_link_libraries(ecos INTERFACE stdc++fs)
endif ()


# ==============================================================================
# Installation rules
# ==============================================================================

install(
        TARGETS libecos
        EXPORT "${ECOS_EXPORT_TARGET}"
        ${ECOS_INSTALL_DESTINATIONS}
)
install(
        DIRECTORY "${publicHeaderDir}/libecos"
        DESTINATION "${ECOS_HEADER_INSTALL_DIR}"
)
install(
        FILES "${CMAKE_SOURCE_DIR}/resources/plot/ecos_plotter.py"
        DESTINATION "${CMAKE_BINARY_DIR}/bin"
)

