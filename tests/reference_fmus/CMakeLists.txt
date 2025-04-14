
### Download reference FMUs
set(REFERENCE_FMUS_VERSION "0.0.38")
set(ZIP_URL "https://github.com/modelica/Reference-FMUs/releases/download/v${REFERENCE_FMUS_VERSION}/Reference-FMUs-${REFERENCE_FMUS_VERSION}.zip") # URL to download
set(ZIP_FILE "${CMAKE_BINARY_DIR}/data/fmus/file.zip") # Download location
set(UNZIP_DIR "${CMAKE_BINARY_DIR}/data/fmus/") # Extraction directory

if(NOT EXISTS ${UNZIP_DIR})

    file(GLOB DIR_CONTENT "${UNZIP_DIR}/*")
    if(NOT DIR_CONTENT)

        message("Downloading and extracting reference FMUs..")
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

        message("Cleaning up downloaded files..")

        # cleanyp
        file(GLOB fmusim_dirs "${UNZIP_DIR}/fmusim*")
        foreach(dir ${fmusim_dirs})
            if(IS_DIRECTORY ${dir})
                file(REMOVE_RECURSE ${dir})
                message(TRACE "Deleted folder: ${dir}")
            endif()
        endforeach()

        file(REMOVE "${UNZIP_DIR}/file.zip")
        file(REMOVE "${UNZIP_DIR}/3.0/Clocks.fmu")

        message("Done..")

    endif ()
endif ()

add_executable(test_reference_fmus test_reference_fmus.cpp)
add_test(NAME test_reference_fmus COMMAND test_reference_fmus)
target_link_libraries(test_reference_fmus PRIVATE libecos Catch2::Catch2WithMain)
target_compile_definitions(test_reference_fmus PRIVATE "REF_FMU_FOLDER=\"${UNZIP_DIR}\"")
