
set(thriftCppSourcesDir "${generatedSourcesDir}/proxyfmu/thrift")
set(thriftJavaSourcesDir "${generatedSourcesDir}/java")

file(MAKE_DIRECTORY "${generatedSourcesDir}")
file(MAKE_DIRECTORY "${thriftCppSourcesDir}")
file(MAKE_DIRECTORY "${thriftJavaSourcesDir}")

find_program(THRIFT_EXECUTABLE NAMES thrift)

if (NOT THRIFT_EXECUTABLE)
    message(FATAL_ERROR "The thrift compiler was not found. Cannot generate Thrift sources.")
endif ()

set(thriftServiceDefinitions
        "${CMAKE_CURRENT_SOURCE_DIR}/defs.thrift"
        "${CMAKE_CURRENT_SOURCE_DIR}/boot.thrift"
        "${CMAKE_CURRENT_SOURCE_DIR}/service.thrift")

# Generate PROXY-FMU classes from Thrift service definitions
foreach (def ${thriftServiceDefinitions})
    # C++
    execute_process(
            COMMAND "${THRIFT_EXECUTABLE}"
            "--gen" "cpp:no_skeleton"
            "-out" "${thriftCppSourcesDir}"
            "${def}"
    )
    # Java
    execute_process(
            COMMAND "${THRIFT_EXECUTABLE}"
            "--gen" "java:private-members,fullcamel"
            "-out" "${thriftJavaSourcesDir}"
            "${def}"
    )
endforeach ()

set(thriftGenerated
        "${thriftCppSourcesDir}/defs_types.h"
        "${thriftCppSourcesDir}/defs_types.cpp"
        "${thriftCppSourcesDir}/FmuService.h"
        "${thriftCppSourcesDir}/FmuService.cpp"
        "${thriftCppSourcesDir}/BootService.h"
        "${thriftCppSourcesDir}/BootService.cpp"
        )

add_library(proxyfmu-service OBJECT ${thriftGenerated})
target_link_libraries(proxyfmu-service PUBLIC thrift::thrift)
set_target_properties(proxyfmu-service PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ==============================================================================
# Installation rules
# ==============================================================================

install(
        FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/boot.thrift"
        "${CMAKE_CURRENT_SOURCE_DIR}/defs.thrift"
        "${CMAKE_CURRENT_SOURCE_DIR}/service.thrift"
        DESTINATION "thrift/definitions"
)

install(
        DIRECTORY "${CMAKE_BINARY_DIR}/generated/java"
        DESTINATION "thrift"
)

install(
        DIRECTORY "${CMAKE_BINARY_DIR}/generated/proxyfmu/thrift/"
        DESTINATION "thrift/cpp"
)
