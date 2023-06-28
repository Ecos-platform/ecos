set(publicHeaderDir ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(proxyfmu)

set(publicHeaders

        "ecos/connection.hpp"
        "ecos/lib_info.hpp"
        "ecos/logger.hpp"
        "ecos/model.hpp"
        "ecos/model_instance.hpp"
        "ecos/model_resolver.hpp"
        "ecos/property.hpp"
        "ecos/scalar.hpp"
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

        "ssp/ssp_parser.hpp"

        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

set(privateHeaders

        "ecos/fmi/fmi_model.hpp"
        "ecos/fmi/fmi_model_instance.hpp"
        "ecos/fmi/fmi_model_sub_resolver.hpp"
        "ecos/fmi/proxy/proxy_fmu.hpp"
        "ecos/fmi/proxy/proxy_slave.hpp"
        "ecos/fmi/proxy/proxy_model.hpp"
        "ecos/fmi/proxy/proxy_model_sub_resolver.hpp"

        "ecos/fmi/fmu.hpp"
        "ecos/fmi/fmi1/fmi1_fmu.hpp"
        "ecos/fmi/fmi1/fmi1_slave.hpp"
        "ecos/fmi/fmi1/fmi1_model_description.hpp"
        "ecos/fmi/fmi2/fmi2_fmu.hpp"
        "ecos/fmi/fmi2/fmi2_slave.hpp"
        "ecos/fmi/fmi2/fmi2_model_description.hpp"

        "ecos/ssp/ssp.hpp"

        "ecos/util/temp_dir.hpp"
        "ecos/util/unzipper.hpp"
        "ecos/util/uuid.hpp"

        )

set(sources

        "ecos/logger.cpp"
        "ecos/model_resolver.cpp"
        "ecos/simulation.cpp"
        "ecos/simulation_runner.cpp"

        "ecos/scenario/scenario.cpp"
        "ecos/scenario/scenario_loader_xml.cpp"

        "ecos/algorithm/fixed_step_algorithm.cpp"

        "ecos/fmi/fmu.cpp"
        "ecos/fmi/fmi1/fmi1_fmu.cpp"
        "ecos/fmi/fmi1/fmi1_slave.cpp"
        "ecos/fmi/fmi1/fmi1_model_description.cpp"
        "ecos/fmi/fmi2/fmi2_fmu.cpp"
        "ecos/fmi/fmi2/fmi2_slave.cpp"
        "ecos/fmi/fmi2/fmi2_model_description.cpp"
        "ecos/fmi/fmi_model_sub_resolver.cpp"

        "ecos/fmi/proxy/proxy_model_sub_resolver.cpp"
        "ecos/fmi/proxy/proxy_fmu.cpp"
        "ecos/fmi/proxy/proxy_slave.cpp"

        "ecos/listeners/csv_writer.cpp"
        "ecos/listeners/simulation_listener.cpp"

        "ecos/ssp/ssp.cpp"
        "ecos/ssp/ssp_loader.cpp"

        "ecos/structure/simulation_structure.cpp"

        "ecos/util/temp_dir.cpp"
        "ecos/util/unzipper.cpp"
        "ecos/util/uuid.cpp"

        )

# Generate sources from .in files
set(generatedFiles "${generatedSourcesDir}/ecos/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ecos/lib_info.cpp.in"
        "${generatedFiles}"
        @ONLY
)

add_library(libecos ${publicHeadersFull} ${publicHeadersFull} ${privateHeaders} ${sources} ${generatedFiles})
#set_property(TARGET libecos PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(libecos PUBLIC "cxx_std_17")
target_include_directories(libecos
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE
        "${generatedSourcesDir}"
        "${SUBPROCESS_INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(libecos
        PUBLIC
        fmt::fmt
        PRIVATE
        thrift::thrift
        pugixml::pugixml
        libzip::zip
        spdlog::spdlog
        unofficial::fmilib::fmilib
        $<TARGET_OBJECTS:proxyfmu-service>
        )
if (UNIX)
    target_link_libraries(libecos INTERFACE stdc++fs tbb)
endif ()

if (ECOS_BUILD_CLIB)
    add_library(libecosc SHARED "${CMAKE_CURRENT_SOURCE_DIR}/ecos/ecos.cpp")
    set_property(TARGET libecosc PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(libecosc
            PUBLIC
            libecos)
endif ()


# ==============================================================================
# Installation rules
# ==============================================================================

install(
        TARGETS libecos
        EXPORT ecos-targets
)
install(
        DIRECTORY "${publicHeaderDir}/ecos"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/ecos"
)
install(
        FILES "${CMAKE_SOURCE_DIR}/resources/plot/ecos_plotter.py"
        DESTINATION "${CMAKE_BINARY_DIR}/bin"
)
