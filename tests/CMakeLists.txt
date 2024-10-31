
function(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE libecos simple_socket Catch2::Catch2WithMain)
    target_include_directories(${name} PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
    add_test(NAME ${name} COMMAND ${name})
endfunction()


add_subdirectory(ecos)
add_subdirectory(proxyfmu)
add_subdirectory(fmilibcpp)
add_subdirectory(misc)

### Download reference FMUs

# Variables
set(REFERENCE_FMUS_VERSION "0.0.36")
set(ZIP_URL "https://github.com/modelica/Reference-FMUs/releases/download/v${REFERENCE_FMUS_VERSION}/Reference-FMUs-${REFERENCE_FMUS_VERSION}.zip") # URL to download
set(ZIP_FILE "${CMAKE_BINARY_DIR}/data/fmus/file.zip") # Download location
set(UNZIP_DIR "${CMAKE_BINARY_DIR}/data/fmus/") # Extraction directory

file(DOWNLOAD ${ZIP_URL} ${ZIP_FILE} SHOW_PROGRESS)

execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${UNZIP_DIR}
)

if (WIN32)
    execute_process(
            COMMAND tar -xf ${ZIP_FILE} -C ${UNZIP_DIR}
            RESULT_VARIABLE unzip_result
    )
else ()
    execute_process(
            COMMAND unzip -o ${ZIP_FILE} -d ${UNZIP_DIR}
            RESULT_VARIABLE unzip_result
    )
endif ()

if(NOT unzip_result EQUAL "0")
    message(FATAL_ERROR "Unzipping failed with error code ${unzip_result}")
else()
    message(STATUS "Download and unzipping completed successfully!")
endif()

# cleanyp
file(GLOB fmusim_dirs "${UNZIP_DIR}/fmusim*")
foreach(dir ${fmusim_dirs})
    if(IS_DIRECTORY ${dir})
        file(REMOVE_RECURSE ${dir})
        message(TRACE "Deleted folder: ${dir}")
    endif()
endforeach()
