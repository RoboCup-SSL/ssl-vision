macro(PYLON5_REPORT_NOT_FOUND REASON_MSG)
    unset(PYLON5_FOUND)
    unset(PYLON5_INCLUDE_DIRS)
    unset(PYLON5_LIBRARIES)
    unset(PYLON5_WORLD_VERSION)
    unset(PYLON5_MAJOR_VERSION)
    unset(PYLON5_MINOR_VERSION)
    # Make results of search visible in the CMake GUI if mvimpact has not
    # been found so that user does not have to toggle to advanced view.
    mark_as_advanced(CLEAR PYLON5_INCLUDE_DIR)
    # Note <package>_FIND_[REQUIRED/QUIETLY] variables defined by FindPackage()
    # use the camelcase library name, not uppercase.
    if(PYLON5_FIND_QUIETLY)
        message(STATUS "Failed to find pylon - " ${REASON_MSG} ${ARGN})
    elseif(PYLON5_FIND_REQUIRED)
        message(FATAL_ERROR "Failed to find pylon - " ${REASON_MSG} ${ARGN})
    else()
        # Neither QUIETLY nor REQUIRED, use no priority which emits a message
        # but continues configuration and allows generation.
        message("-- Failed to find pylon - " ${REASON_MSG} ${ARGN})
    endif()
    
    return()
endmacro(PYLON5_REPORT_NOT_FOUND)

# Search user-installed locations first, so that we prefer user installs
# to system installs where both exist.
get_filename_component(PYLON_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)
list(APPEND PYLON5_CHECK_INCLUDE_DIRS
    /opt/pylon5/include
    )
execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCH)
list(APPEND PYLON5_CHECK_LIBRARY_DIRS
    /opt/pylon5/lib64
    )

# Check general hints
if(PYLON5_HINTS AND EXISTS ${PYLON5_HINTS})
    set(PYLON5_INCLUDE_DIR_HINTS ${PYLON5_HINTS}/include)
    set(PYLON5_LIBRARY_DIR_HINTS ${PYLON5_HINTS}/lib)
endif()

# Search supplied hint directories first if supplied.
# Find include directory for pylon
find_path(PYLON5_INCLUDE_DIR
    NAMES pylon/PylonBase.h
    PATHS ${PYLON5_INCLUDE_DIR_HINTS}
    ${PYLON5_CHECK_INCLUDE_DIRS}
    NO_DEFAULT_PATH)
if(NOT PYLON5_INCLUDE_DIR OR NOT EXISTS ${PYLON5_INCLUDE_DIR})
    PYLON5_REPORT_NOT_FOUND(
        "Could not find mvimpact include directory, set PYLON5_INCLUDE_DIR to "
        "path to pylon include directory,"
        "e.g. /opt/PYLON5_acquire.")
else()
    message(STATUS "pylon include dir found: " ${PYLON5_INCLUDE_DIR})
endif()

# Find library directory for mvimpact
find_library(PYLON5_LIBRARY
    NAMES libpylonbase.so libpylonutility.so libGCBase_gcc_v3_0_Basler_pylon_v5_0.so libGenApi_gcc_v3_0_Basler_pylon_v5_0.so
    PATHS ${PYLON5_LIBRARY_DIR_HINTS}
    ${PYLON5_CHECK_LIBRARY_DIRS}
    NO_DEFAULT_PATH)
if(NOT PYLON5_LIBRARY OR NOT EXISTS ${PYLON5_LIBRARY})
    PYLON5_REPORT_NOT_FOUND(
        "Could not find mvimpact library, set PYLON5_LIBRARY "
        "to full path to mvimpact library direcotory.")
else()
    # TODO: need to fix this hacky solution for getting PYLON5_LIBRARY_DIR
    string(REGEX MATCH ".*/" PYLON5_LIBRARY_DIR ${PYLON5_LIBRARY})
    message(STATUS "pylon library dir found: " ${PYLON5_LIBRARY_DIR})
endif()

# Mark internally as found, then verify. PYLON5_REPORT_NOT_FOUND() unsets if
# called.
set(PYLON5_FOUND TRUE)

# Set standard CMake FindPackage variables if found.
if(PYLON5_FOUND)
    set(PYLON5_INCLUDE_DIRS ${PYLON5_INCLUDE_DIR})
    file(GLOB PYLON5_LIBRARIES ${PYLON5_LIBRARY_DIR}lib*.so)
endif()

# Handle REQUIRED / QUIET optional arguments.
include(FindPackageHandleStandardArgs)
if(PYLON5_FOUND)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Mvimpact DEFAULT_MSG
        PYLON5_INCLUDE_DIRS PYLON5_LIBRARIES)
endif()

# Only mark internal variables as advanced if we found mvimpact, otherwise
# leave it visible in the standard GUI for the user to set manually.
if(PYLON5_FOUND)
    mark_as_advanced(FORCE PYLON5_INCLUDE_DIR PYLON5_LIBRARY)
endif()
