
add_subdirectory(proxyfmu)

set(publicHeaders

        "ecos/connection.hpp"
        "ecos/lib_info.hpp"
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

        "ecos/logger/logger.hpp"

        "ecos/structure/simulation_structure.hpp"

        "ecos/ssp/ssp_loader.hpp"

        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${PROJECT_SOURCE_DIR}/include/${header}")
endforeach ()

set(privateHeaders

        "ecos/fmi/fmi_model.hpp"
        "ecos/fmi/fmi_model_instance.hpp"
        "ecos/fmi/fmi_model_sub_resolver.hpp"
        "ecos/fmi/proxy/proxy_fmu.hpp"
        "ecos/fmi/proxy/proxy_slave.hpp"
        "ecos/fmi/proxy/proxy_model.hpp"
        "ecos/fmi/proxy/proxy_model_sub_resolver.hpp"

        "fmilibcpp/buffered_slave.hpp"
        "fmilibcpp/fmicontext.hpp"
        "fmilibcpp/fmu.hpp"
        "fmilibcpp/model_description.hpp"
        "fmilibcpp/scalar_variable.hpp"
        "fmilibcpp/slave.hpp"

        "fmilibcpp/fmi1/fmi1_fmu.hpp"
        "fmilibcpp/fmi1/fmi1_slave.hpp"
        "fmilibcpp/fmi1/fmi1_model_description.hpp"

        "fmilibcpp/fmi2/fmi2_fmu.hpp"
        "fmilibcpp/fmi2/fmi2_slave.hpp"
        "fmilibcpp/fmi2/fmi2_model_description.hpp"

        "ecos/ssp/ssp.hpp"

        "ecos/util/temp_dir.hpp"
        "ecos/util/unzipper.hpp"
        "ecos/util/uuid.hpp"

        )

set(sources

        "ecos/model_resolver.cpp"
        "ecos/simulation.cpp"
        "ecos/simulation_runner.cpp"

        "ecos/scenario/scenario.cpp"
        "ecos/scenario/scenario_loader_xml.cpp"

        "ecos/algorithm/fixed_step_algorithm.cpp"

        "ecos/fmi/fmi_model_sub_resolver.cpp"

        "ecos/fmi/proxy/proxy_model_sub_resolver.cpp"
        "ecos/fmi/proxy/proxy_fmu.cpp"
        "ecos/fmi/proxy/proxy_slave.cpp"

        "ecos/listeners/csv_writer.cpp"
        "ecos/listeners/simulation_listener.cpp"

        "ecos/logger/logger.cpp"

        "ecos/ssp/ssp.cpp"
        "ecos/ssp/ssp_loader.cpp"

        "ecos/structure/simulation_structure.cpp"

        "ecos/util/temp_dir.cpp"
        "ecos/util/unzipper.cpp"
        "ecos/util/uuid.cpp"

        "fmilibcpp/fmu.cpp"
        "fmilibcpp/fmi1/fmi1_fmu.cpp"
        "fmilibcpp/fmi1/fmi1_slave.cpp"
        "fmilibcpp/fmi1/fmi1_model_description.cpp"
        "fmilibcpp/fmi2/fmi2_fmu.cpp"
        "fmilibcpp/fmi2/fmi2_slave.cpp"
        "fmilibcpp/fmi2/fmi2_model_description.cpp"

)

# Generate sources from .in files
set(generatedFiles "${generatedSourcesDir}/ecos/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ecos/lib_info.cpp.in"
        "${generatedFiles}"
        @ONLY
)

add_library(libecos ${publicHeadersFull} ${privateHeaders} ${sources} ${generatedFiles})
set_property(TARGET libecos PROPERTY POSITION_INDEPENDENT_CODE ON)
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
        libzip::zip
        spdlog::spdlog
        thrift::thrift
        pugixml::pugixml
        unofficial::fmilib::fmilib
        "$<TARGET_OBJECTS:proxyfmu-service>"
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

# Make sure executable is up to date
#add_dependencies(libecos proxyfmu)
