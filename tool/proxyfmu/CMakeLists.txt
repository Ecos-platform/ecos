
add_executable(proxyfmu
        proxyfmu.cpp
        "handlers/fmu_service_handler.cpp"
        "handlers/boot_service_handler.cpp"
        "$<TARGET_OBJECTS:proxyfmu-service>"
        "${generatedSourcesDir}/ecos/lib_info.cpp")
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
        )
if (UNIX)
  target_link_libraries(proxyfmu PRIVATE pthread)
endif ()
