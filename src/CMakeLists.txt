
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
        "ecos/fmi/proxy/proxy_model.hpp"
        "ecos/fmi/proxy/proxy_model_sub_resolver.hpp"

        "ecos/ssp/ssp.hpp"

        "ecos/util/temp_dir.hpp"
        "ecos/util/unzipper.hpp"
        "ecos/util/uuid.hpp"

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

        "proxyfmu/opcodes.hpp"
        "proxyfmu/process_helper.hpp"
        "proxyfmu/proxy_fmu.hpp"
        "proxyfmu/proxy_slave.hpp"
        "proxyfmu/remote_info.hpp"

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

        "ecos/listeners/csv_writer.cpp"
        "ecos/listeners/simulation_listener.cpp"

        "ecos/ssp/ssp.cpp"
        "ecos/ssp/ssp_loader.cpp"

        "ecos/structure/simulation_structure.cpp"

        "proxyfmu/proxy_fmu.cpp"
        "proxyfmu/proxy_slave.cpp"

)

# Generate sources from .in files
set(generatedFiles "${generatedSourcesDir}/ecos/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ecos/lib_info.cpp.in"
        "${generatedFiles}"
        @ONLY
)

add_library(logger OBJECT "ecos/logger/logger.cpp")
set_target_properties(logger PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(logger PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(logger
        PUBLIC
        fmt::fmt
        PRIVATE
        spdlog::spdlog
)

add_library(util OBJECT
        "ecos/util/temp_dir.cpp"
        "ecos/util/unzipper.cpp"
        "ecos/util/uuid.cpp"
)
set_target_properties(util PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(util PRIVATE libzip::zip)
target_include_directories(util
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src")

add_library(fmilibcpp OBJECT
        "fmilibcpp/fmu.cpp"
        "fmilibcpp/fmi1/fmi1_fmu.cpp"
        "fmilibcpp/fmi1/fmi1_slave.cpp"
        "fmilibcpp/fmi1/fmi1_model_description.cpp"
        "fmilibcpp/fmi2/fmi2_fmu.cpp"
        "fmilibcpp/fmi2/fmi2_slave.cpp"
        "fmilibcpp/fmi2/fmi2_model_description.cpp"
)
set_target_properties(fmilibcpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(fmilibcpp PRIVATE fmi4c fmt::fmt)
target_include_directories(fmilibcpp
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src")

add_library(libecos ${publicHeadersFull} ${privateHeaders} ${sources} ${generatedFiles})
set_target_properties(libecos PROPERTIES PREFIX "")
set_target_properties(libecos PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(libecos PUBLIC "cxx_std_20")
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
        simple_socket
        msgpack-cxx
        libzip::zip
        spdlog::spdlog
        pugixml::pugixml
        fmi4c
        "$<TARGET_OBJECTS:util>"
        "$<TARGET_OBJECTS:logger>"
        "$<TARGET_OBJECTS:fmilibcpp>"
)
if (UNIX)
    target_link_libraries(libecos INTERFACE stdc++fs tbb)
endif ()

if (ECOS_BUILD_CLIB)
    add_library(libecosc SHARED "${CMAKE_CURRENT_SOURCE_DIR}/ecos/ecos.cpp")
    set_target_properties(libecosc PROPERTIES PREFIX "")
    set_property(TARGET libecosc PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(libecosc
            PUBLIC
            libecos)
endif ()

# Make sure executable is up to date
add_dependencies(libecos proxyfmu)
