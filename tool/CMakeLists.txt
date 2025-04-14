
if (ECOS_BUILD_CLI)
    add_subdirectory(cli)
endif ()

if (ECOS_WITH_PROXYFMU)
    add_subdirectory(proxyfmu)
endif ()
