
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

if (PROXYFMU_BUILD_TESTS)
  # Copy executable to test directory in order for CTest to function
  add_custom_command(TARGET proxyfmu POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:proxyfmu> ${CMAKE_BINARY_DIR}/tests/$<TARGET_FILE_NAME:proxyfmu>
          )
  add_custom_command(TARGET proxyfmu POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E echo
          Copying $<TARGET_FILE:proxyfmu> into ${CMAKE_BINARY_DIR}/tests/$<TARGET_FILE_NAME:proxyfmu>
          )
endif()
