set(publicHeaderDir ${CMAKE_SOURCE_DIR}/include)

# ==============================================================================
# common headers
# ==============================================================================

set(commonPublicHeaders
        "vico/util/temp_dir.hpp"
        "vico/util/fs_portability.hpp"
        )

set(commonPublicHeadersFull)
foreach (header IN LISTS commonPublicHeaders)
  list(APPEND commonPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

# ==============================================================================
# fmilib wrapper
# ==============================================================================

set(fmilibwrapperPublicHeaders
        "vico/fmi/fmu.hpp"
        "vico/fmi/slave.hpp"
        "vico/fmi/scalar_variable.hpp"
        "vico/fmi/model_description.hpp"
        )

set(fmilibwrapperPrivateHeaders
        "vico/fmi/fmicontext.hpp"

        "vico/fmi/fmi1/fmi1_fmu.hpp"
        "vico/fmi/fmi1/fmi1_model_description.hpp"
        "vico/fmi/fmi1/fmi1_slave.hpp"

        "vico/fmi/fmi2/fmi2_fmu.hpp"
        "vico/fmi/fmi2/fmi2_model_description.hpp"
        "vico/fmi/fmi2/fmi2_slave.hpp"

        "vico/util/fixed_range_random_generator.hpp"
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

set(fmilibwrapperPublicHeadersFull)
foreach (header IN LISTS fmiPublicHeaders)
  list(APPEND fmiPublicHeadersFull "${publicHeaderDir}/${header}")
endforeach ()

add_library(fmilibwrapper ${commonPublicHeadersFull} ${fmilibwrapperPublicHeadersFull} ${fmilibwrapperPrivateHeaders} ${fmilibwrapperSources})
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

