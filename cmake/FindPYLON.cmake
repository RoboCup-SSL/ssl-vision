macro(PYLON_REPORT_NOT_FOUND REASON_MSG)
    unset(PYLON_FOUND)
    unset(PYLON_INCLUDE_DIRS)
    unset(PYLON_LIBRARIES)
    
    if(PYLON_FIND_REQUIRED)
        message(FATAL_ERROR "Failed to find pylon - " ${REASON_MSG} ${ARGN})    
    else()
        message("-- Failed to find pylon - " ${REASON_MSG} ${ARGN})
    endif()
    
endmacro(PYLON_REPORT_NOT_FOUND)



# === FIND PYLON_INCLUDE_DIR (most probably /opt/pylon/include) === #
find_path(PYLON_INCLUDE_DIR
    NAMES pylon/PylonBase.h
    PATHS /opt/pylon/include $ENV{PYLON_ROOT}/include
    NO_DEFAULT_PATH)

if(NOT PYLON_INCLUDE_DIR OR NOT EXISTS ${PYLON_INCLUDE_DIR})
    PYLON_REPORT_NOT_FOUND(
        "Could not find pylon include directory. Set PYLON_INCLUDE_DIR "
        "to full path to pylon include directory,"
        "e.g. -DPYLON_INCLUDE_DIR=/opt/pylon/include/")
else()
    message(STATUS "  pylon include dir found: " ${PYLON_INCLUDE_DIR})
endif()



# === FIND PYLON_LIBRARY (most probably /opt/pylon/lib) === #
find_library(PYLON_LIBRARY
    NAMES libpylonbase.so libpylonutility.so
    PATHS /opt/pylon/lib/ $ENV{PYLON_ROOT}/lib/
    NO_DEFAULT_PATH)

# If directory is neither given nor found, then error
if(NOT PYLON_LIBRARY_DIR)
    if(PYLON_LIBRARY)
        # TODO: need to fix this hacky solution for getting PYLON_LIBRARY_DIR
        string(REGEX MATCH ".*/" PYLON_LIBRARY_DIR ${PYLON_LIBRARY})
    endif()
endif()

if(NOT PYLON_LIBRARY_DIR)
    PYLON_REPORT_NOT_FOUND(
        "Could not find pylon library. Set PYLON_LIBRARY_DIR "
        "to full path to pylon library directory, "
        "e.g. -DPYLON_LIBRARY_DIR=/opt/pylon/lib/")
else()
    message(STATUS "  pylon library dir found: " ${PYLON_LIBRARY_DIR})
endif()



# === SET PYLON_FOUND AND SET PYLON_INCLUDE_DIRS / PYLON_LIBRARIES === #
if(PYLON_INCLUDE_DIR AND PYLON_LIBRARY_DIR)
    set(PYLON_FOUND TRUE)
endif()

set(PYLON_INCLUDE_DIRS ${PYLON_INCLUDE_DIR})
file(GLOB PYLON_LIBRARIES ${PYLON_LIBRARY_DIR}lib*.so)