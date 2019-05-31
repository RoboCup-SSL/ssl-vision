# FindFlycap.cmake - Find flycap api
# Modified from FindEigen.cmake by alexs.mac@gmail.com  (Alex Stewart)
#
# This module defines the following variables:
#
# FLYCAP_FOUND: TRUE if flycap is found.
# FLYCAP_INCLUDE_DIRS: Include directories for flycap.
#
# The following variables control the behaviour of this module:
#
# FLYCAP_HINTS: List of additional directories in which to
#                             search for flycap.
#
# The following variables are also defined by this module, but in line with
# CMake recommended FindPackage() module style should NOT be referenced directly
# by callers (use the plural variables detailed above instead).  These variables
# do however affect the behaviour of the module via FIND_[PATH/LIBRARY]() which
# are NOT re-called (i.e. search for library is not repeated) if these variables
# are set with valid values _in the CMake cache_. This means that if these
# variables are set directly in the cache, either by the user in the CMake GUI,
# or by the user passing -DVAR=VALUE directives to CMake when called (which
# explicitly defines a cache variable), then they will be used verbatim,
# bypassing the HINTS variables and other hard-coded search locations.
#
# FLYCAP_INCLUDE_DIR: Include directory for flycap, not including the
#                       include directory of any dependencies.

# Called if we failed to find flycap or any of it's required dependencies,
# unsets all public (designed to be used externally) variables and reports
# error message at priority depending upon [REQUIRED/QUIET/<NONE>] argument.
macro(FLYCAP_REPORT_NOT_FOUND REASON_MSG)
    unset(FLYCAP_FOUND)
    unset(FLYCAP_INCLUDE_DIRS)
    # Make results of search visible in the CMake GUI if flycap has not
    # been found so that user does not have to toggle to advanced view.
    mark_as_advanced(CLEAR FLYCAP_INCLUDE_DIR)
    # Note <package>_FIND_[REQUIRED/QUIETLY] variables defined by FindPackage()
    # use the camelcase library name, not uppercase.
    if(FLYCAP_FIND_QUIETLY)
        message(STATUS "Failed to find flycap - " ${REASON_MSG} ${ARGN})
    elseif(FLYCAP_FIND_REQUIRED)
        message(FATAL_ERROR "Failed to find flycap - " ${REASON_MSG} ${ARGN})
    else()
        # Neither QUIETLY nor REQUIRED, use no priority which emits a message
        # but continues configuration and allows generation.
#        message("-- Failed to find mvimpact - " ${REASON_MSG} ${ARGN})
    endif()

    return()
endmacro(FLYCAP_REPORT_NOT_FOUND)

# Check general hints
# if(FLYCAP_HINTS AND EXISTS ${FLYCAP_HINTS})
#     set(FLYCAP_INCLUDE_DIR_HINTS ${FLYCAP_HINTS}/include)
#     set(FLYCAP_LIBRARY_DIR_HINTS ${FLYCAP_HINTS}/lib)
# endif()

# Mark internally as found, then verify. FLYCAP_REPORT_NOT_FOUND() unsets if
# called.
set(FLYCAP_FOUND TRUE)

# Search supplied hint directories first if supplied.
# Find include directory for mvimpact
find_path(FLYCAP_INCLUDE_DIR
    NAMES FlyCapture2.h
    PATHS ${FLYCAP_HINTS}
    NO_DEFAULT_PATH)
if(NOT FLYCAP_INCLUDE_DIR OR NOT EXISTS ${FLYCAP_INCLUDE_DIR})
    FLYCAP_REPORT_NOT_FOUND(
        "Could not find flycap include directory, set FLYCAP_INCLUDE_DIR to "
        "path to flycap include directory,"
        "e.g. /usr/include/flycap")
else()
  message(STATUS "flycap include dir found: " ${FLYCAP_INCLUDE_DIR})
endif()

# Catch case when caller has set FLYCAP_INCLUDE_DIR in the cache / GUI and
# thus FIND_[PATH/LIBRARY] are not called, but specified locations are
# invalid, otherwise we would report the library as found.
if(FLYCAP_INCLUDE_DIR AND NOT EXISTS ${FLYCAP_INCLUDE_DIR}/FlyCapture2.h)
    FLYCAP_REPORT_NOT_FOUND("Caller defined FLYCAP_INCLUDE_DIR: "
        ${FLYCAP_INCLUDE_DIR}
        " does not contain FlyCapture2.h header.")
endif()

# Set standard CMake FindPackage variables if found.
if(FLYCAP_FOUND)
    set(FLYCAP_INCLUDE_DIRS ${FLYCAP_INCLUDE_DIR})
endif()

# Handle REQUIRED / QUIET optional arguments.
include(FindPackageHandleStandardArgs)
if(FLYCAP_FOUND)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(flycap DEFAULT_MSG
        FLYCAP_INCLUDE_DIRS)
endif()

# Only mark internal variables as advanced if we found flycap, otherwise
# leave it visible in the standard GUI for the user to set manually.
if(FLYCAP_FOUND)
    mark_as_advanced(FORCE FLYCAP_INCLUDE_DIR)
endif()
