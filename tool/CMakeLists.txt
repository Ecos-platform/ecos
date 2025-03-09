
add_subdirectory(cli)
if (ECOS_WITH_PROXYFMU)
    add_subdirectory(proxyfmu)
endif ()
