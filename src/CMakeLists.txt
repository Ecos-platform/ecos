
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

        "ecos/ssp/ssp_loader.hpp"

        "ecos/structure/simulation_structure.hpp"

        "ecos/util/plotter.hpp"
)

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
    list(APPEND commonPublicHeadersFull "${PROJECT_SOURCE_DIR}/include/${header}")
endforeach ()

set(privateHeaders

        "ecos/fmi/fmi_model.hpp"
        "ecos/fmi/fmi_model_instance.hpp"

        "ecos/resolvers/file_model_sub_resolver.hpp"
        "ecos/resolvers/url_model_sub_resolver.hpp"

        "ecos/ssp/ssp.hpp"

        "util/temp_dir.hpp"
        "util/unzipper.hpp"
        "util/uuid.hpp"

)

if (ECOS_WITH_PROXYFMU)
    list(APPEND privateHeaders

            "ecos/fmi/proxy/proxy_model.hpp"

            "ecos/resolvers/proxy_model_sub_resolver.hpp"

            "proxyfmu/opcodes.hpp"
            "proxyfmu/process_helper.hpp"
            "proxyfmu/proxy_fmu.hpp"
            "proxyfmu/proxy_slave.hpp"
            "proxyfmu/remote_info.hpp"

    )
endif ()

set(sources

        "ecos/model_resolver.cpp"
        "ecos/simulation.cpp"
        "ecos/simulation_runner.cpp"

        "ecos/scenario/scenario.cpp"
        "ecos/scenario/scenario_loader_xml.cpp"

        "ecos/algorithm/fixed_step_algorithm.cpp"

        "ecos/resolvers/file_model_sub_resolver.cpp"
        "ecos/resolvers/url_model_sub_resolver.cpp"

        "ecos/listeners/csv_writer.cpp"
        "ecos/listeners/simulation_listener.cpp"

        "ecos/ssp/ssp.cpp"
        "ecos/ssp/ssp_loader.cpp"

        "ecos/structure/simulation_structure.cpp"

        "ecos/logger/logger.cpp"

        "ecos/util/plotter.cpp"

)

if (ECOS_WITH_PROXYFMU)
    list(APPEND sources

            "ecos/resolvers/proxy_model_sub_resolver.cpp"

            "proxyfmu/proxy_fmu.cpp"
            "proxyfmu/proxy_slave.cpp"

            "external/flatbuffers/flatbuffers/util.cpp"
    )
endif ()

# Generate sources from .in files
set(generatedFiles "${GENERATED_SRC_DIR}/ecos/lib_info.cpp")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ecos/lib_info.cpp.in"
        "${generatedFiles}"
        @ONLY
)

add_library(fmilibcpp OBJECT

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

        "fmilibcpp/fmu.cpp"

        "fmilibcpp/fmi1/fmi1_fmu.cpp"
        "fmilibcpp/fmi1/fmi1_slave.cpp"
        "fmilibcpp/fmi1/fmi1_model_description.cpp"

        "fmilibcpp/fmi2/fmi2_fmu.cpp"
        "fmilibcpp/fmi2/fmi2_slave.cpp"
        "fmilibcpp/fmi2/fmi2_model_description.cpp"

        "fmilibcpp/fmi3/fmi3_fmu.cpp"
        "fmilibcpp/fmi3/fmi3_slave.cpp"
        "fmilibcpp/fmi3/fmi3_model_description.cpp"

        "external/pugixml/pugixml.cpp"
)
set_target_properties(fmilibcpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(fmilibcpp PRIVATE fmi4c)
target_include_directories(fmilibcpp
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog"
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src")

add_library(libecos ${publicHeadersFull} ${privateHeaders} ${sources} ${generatedFiles})
set_target_properties(libecos PROPERTIES PREFIX "")
set_target_properties(libecos PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(libecos PUBLIC "cxx_std_20")
target_include_directories(libecos
        PUBLIC
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml>"
        PRIVATE
        "${GENERATED_SRC_DIR}"
        "${THIRD_PARTY_INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(libecos
        PRIVATE
        fmi4c
        "$<TARGET_OBJECTS:fmilibcpp>"
)
if (UNIX)
    target_link_libraries(libecos INTERFACE stdc++fs tbb)
endif ()

if (ECOS_WITH_PROXYFMU)

    target_compile_definitions(libecos PRIVATE "ECOS_WITH_PROXYFMU")

    target_include_directories(libecos
            PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers"
    )

    target_link_libraries(libecos
            PRIVATE
            simple_socket
    )

    # Make sure executable is up to date
    add_dependencies(libecos proxyfmu)
endif ()

if (ECOS_BUILD_CLIB)
    add_library(libecosc SHARED "${CMAKE_CURRENT_SOURCE_DIR}/ecos/ecos.cpp")
    set_target_properties(libecosc PROPERTIES PREFIX "")
    set_property(TARGET libecosc PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(libecosc PUBLIC libecos)

    add_custom_command(TARGET libecosc POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libecosc>
            ${PY_BINARY_DIR}
            COMMENT "Copying libecosc to ${PY_BINARY_DIR}"
    )

endif ()
