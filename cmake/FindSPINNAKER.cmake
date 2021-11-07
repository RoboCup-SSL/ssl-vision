unset(SPINNAKER_FOUND)
unset(SPINNAKER_INCLUDE_DIRS)
unset(SPINNAKER_LIBS)

find_path(SPINNAKER_INCLUDE_DIRS NAMES
        Spinnaker.h
        HINTS
        /opt/spinnaker/include/
        /usr/include/spinnaker/
        /usr/local/include/spinnaker/)

find_library(SPINNAKER_LIBS NAMES Spinnaker
        HINTS
        /opt/spinnaker/lib
        /usr/lib
        /usr/local/lib)

if (SPINNAKER_INCLUDE_DIRS AND SPINNAKER_LIBS)
    set(SPINNAKER_FOUND 1)
endif (SPINNAKER_INCLUDE_DIRS AND SPINNAKER_LIBS)