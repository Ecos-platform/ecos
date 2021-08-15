set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

# ==============================================================================
# common headers
# ==============================================================================

set(publicHeaders
        "vico/fmu.hpp"
        "vico/slave.hpp"
        "vico/scalar_variable.hpp"
        "vico/model_description.hpp"
        "vico/simulation.hpp"

        "vico/util/temp_dir.hpp"
        "vico/util/fs_portability.hpp"
        "vico/util/uuid.hpp"
        )

set(publicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
  list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

# ==============================================================================
# fmilib wrapper
# ==============================================================================

set(fmilibwrapperPrivateHeaders
        "vico/fmi/fmicontext.hpp"

        "vico/fmi/fmi1/fmi1_fmu.hpp"
        "vico/fmi/fmi1/fmi1_model_description.hpp"
        "vico/fmi/fmi1/fmi1_slave.hpp"

        "vico/fmi/fmi2/fmi2_fmu.hpp"
        "vico/fmi/fmi2/fmi2_model_description.hpp"
        "vico/fmi/fmi2/fmi2_slave.hpp"

        "vico/util/random.hpp"
        )

set(fmilibwrapperSources
        "vico/fmi/fmu.cpp"

        "vico/fmi/fmi1/fmi1_fmu.cpp"
        "vico/fmi/fmi1/fmi1_slave.cpp"
        "vico/fmi/fmi1/fmi1_model_description.cpp"

        "vico/fmi/fmi2/fmi2_fmu.cpp"
        "vico/fmi/fmi2/fmi2_slave.cpp"
        "vico/fmi/fmi2/fmi2_model_description.cpp"

        "vico/util/temp_dir.cpp"
        )

add_library(fmilibwrapper ${publicHeadersFull} ${fmilibwrapperPublicHeadersFull} ${fmilibwrapperPrivateHeaders} ${fmilibwrapperSources})
target_compile_features(fmilibwrapper PUBLIC "cxx_std_17")
#set_target_properties(fmilibwrapper PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(fmilibwrapper
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
target_link_libraries(fmilibwrapper PRIVATE FMILIB::FMILIB)
if (UNIX)
  target_link_libraries(fmilibwrapper INTERFACE stdc++fs PRIVATE dl)
endif ()


# ==============================================================================
# Installation rules
# ==============================================================================

install(
        TARGETS fmilibwrapper
        EXPORT "${VICO_EXPORT_TARGET}"
        ${VICO_INSTALL_DESTINATIONS}
)
install(
        DIRECTORY "${publicHeaderDir}/VICO"
        DESTINATION "${VICO_HEADER_INSTALL_DIR}"
)

