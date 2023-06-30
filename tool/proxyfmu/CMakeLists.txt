
set(sources
        "proxyfmu.cpp"
        "handlers/fmu_service_handler.cpp"
        "handlers/boot_service_handler.cpp"

        "${generatedSourcesDir}/ecos/lib_info.cpp"
        )

add_executable(proxyfmu ${sources})
target_include_directories(proxyfmu
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${SUBPROCESS_INCLUDE_DIR}"
        "${generatedSourcesDir}")
target_link_libraries(proxyfmu
        PRIVATE
        libecos
        CLI11::CLI11
        thrift::thrift
        "$<TARGET_OBJECTS:proxyfmu-service>"
        )
if (UNIX)
  target_link_libraries(proxyfmu PRIVATE pthread)
endif ()
