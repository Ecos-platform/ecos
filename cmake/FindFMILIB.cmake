# Find FMILIB
#
# Find the native FMILIB headers and libraries.
#
#   FMILIB_INCLUDE_DIRS   - where to find fmilib.h
#   FMILIB_LIBRARIES      - List of libraries when using FMILIB.
#   FMILIB_FOUND          - True if FMILIB found.
#
find_path(FMILIB_INCLUDE_DIR NAMES fmilib.h)
mark_as_advanced(FMILIB_INCLUDE_DIR)

find_library(FMILIB_LIBRARY NAMES fmilib fmilib_shared)
mark_as_advanced(FMILIB_LIBRARY)

find_program(FMILIB_EXECUTABLE NAMES FMILIB)
mark_as_advanced(FMILIB_EXECUTABLE)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FMILIB
        REQUIRED_VARS FMILIB_LIBRARY FMILIB_INCLUDE_DIR)

if (FMILIB_FOUND)

  set(FMILIB_INCLUDE_DIRS ${FMILIB_INCLUDE_DIR})

  if (NOT FMILIB_LIBRARIES)
    set(FMILIB_LIBRARIES ${FMILIB_LIBRARY})
  endif ()

  if (NOT TARGET FMILIB::FMILIB)
    add_library(FMILIB::FMILIB UNKNOWN IMPORTED)
    set_target_properties(FMILIB::FMILIB PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FMILIB_INCLUDE_DIR})
    set_property(TARGET FMILIB::FMILIB APPEND PROPERTY
            IMPORTED_LOCATION "${FMILIB_LIBRARY}")
  endif ()

endif ()