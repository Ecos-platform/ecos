set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

set(publicHeaders
        "vico/algorithm.hpp"
        "vico/connection.hpp"
        "vico/model.hpp"
        "vico/modifier.hpp"
        "vico/model_description.hpp"
        "vico/model_instance.hpp"
        "vico/property.hpp"
        "vico/scalar_variable.hpp"
        "vico/simulation.hpp"
        "vico/simulation_listener.hpp"

        "vico/structure/simulation_structure.hpp"

        "vico/util/temp_dir.hpp"
        "vico/util/fs_portability.hpp"
        "vico/util/uuid.hpp"
        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
  list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

set(privateHeaders
        "vico/structure/variable_identifier.hpp"

        "vico/fmi/fmicontext.hpp"

        "vico/fmi/fmi1/fmi1_fmu.hpp"
        "vico/fmi/fmi1/fmi1_model_description.hpp"
        "vico/fmi/fmi1/fmi1_slave.hpp"

        "vico/fmi/fmi2/fmi2_fmu.hpp"
        "vico/fmi/fmi2/fmi2_model_description.hpp"
        "vico/fmi/fmi2/fmi2_slave.hpp"

        "vico/util/random.hpp"
        )

set(sources
        "vico/simulation.cpp"
        "vico/structure/simulation_structure.cpp"
        "vico/structure/variable_identifier.cpp"

        "vico/fmi/fmu.cpp"

        "vico/fmi/fmi1/fmi1_fmu.cpp"
        "vico/fmi/fmi1/fmi1_slave.cpp"
        "vico/fmi/fmi1/fmi1_model_description.cpp"

        "vico/fmi/fmi2/fmi2_fmu.cpp"
        "vico/fmi/fmi2/fmi2_slave.cpp"
        "vico/fmi/fmi2/fmi2_model_description.cpp"

        "vico/util/temp_dir.cpp"
        "vico/util/uuid.cpp"
        )

add_library(vico ${publicHeadersFull} ${publicHeadersFull} ${privateHeaders} ${sources})
target_compile_features(vico PUBLIC "cxx_std_17")
#set_target_properties(fmilibwrapper PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(vico
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(vico PRIVATE FMILIB::FMILIB)
if (UNIX)
  target_link_libraries(vico INTERFACE stdc++fs PRIVATE dl)
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

