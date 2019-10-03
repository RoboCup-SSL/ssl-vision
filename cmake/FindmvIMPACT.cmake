# for now this script only searches in standard directories

set( mvIMPACTAcquire_DIR_WIN32 "C:/Program Files/MATRIX VISION/mvIMPACT Acquire" )
set( mvIMPACTAcquire_DIR_LINUX "/opt/mvIMPACT_Acquire" )

if( DEFINED ENV{MVIMPACT_ACQUIRE_DIR} )
  set( mvIMPACTAcquire_DIR_WIN32 $ENV{MVIMPACT_ACQUIRE_DIR} )
endif()

set( mvIMPACT_FOUND false )

if( WIN32 )
  if( EXISTS ${mvIMPACTAcquire_DIR_WIN32} )
    set( mvIMPACT_FOUND true )
	set( mvIMPACT_INCLUDE_DIR ${mvIMPACTAcquire_DIR_WIN32} )
	set( mvIMPACT_LIB_DIR ${mvIMPACTAcquire_DIR_WIN32}/lib/win/x64 )
	set( mvIMPACT_LIBS mvDeviceManager )
  endif()
else()
  if( EXISTS ${mvIMPACTAcquire_DIR_LINUX} )
    execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCH)
    set( mvIMPACT_FOUND true )
	set( mvIMPACT_INCLUDE_DIR ${mvIMPACTAcquire_DIR_LINUX} )
	set( mvIMPACT_LIB_DIR ${mvIMPACTAcquire_DIR_LINUX}/lib/${ARCH} )
	set( mvIMPACT_LIBS mvDeviceManager )
  endif()

  link_directories( ${mvIMPACT_LIB_DIR} )
endif()

if( NOT mvIMPACT_FIND_QUIETLY )
    if( mvIMPACT_FOUND )
      message(STATUS "mvIMPACT found: ${mvIMPACT_INCLUDE_DIR}")
    else()
      message(STATUS "mvIMPACT NOT found")
    endif()
endif()