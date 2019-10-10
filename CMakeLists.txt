project(ssl-vision)
set(OpenGL_GL_PREFERENCE LEGACY)
set(USE_QT5 FALSE CACHE BOOL "Use Qt5 instead of Qt4")
set(USE_DC1394 TRUE CACHE BOOL "Compile with DC1394 driver (firewire cameras)")
set(USE_SPINNAKER FALSE CACHE BOOL "Compile with Spinnaker driver (FLIR cameras)")
set(USE_mvIMPACT FALSE CACHE BOOL "Compile with mvImpact driver (bluefox USB2 + USB3 cameras)")
set(USE_PYLON FALSE CACHE BOOL "Compile with pylon driver (Basler cameras)")
set(USE_FLYCAP FALSE CACHE BOOL "Compile with flycap driver (FLIR cameras, predecessor of Spinnaker)")
set(USE_V4L FALSE CACHE BOOL "Compile with Video4Linux support (generic webcams)")
set(USE_SPLITTER FALSE CACHE BOOL "Compile with Camera splitter support (virtual cameras with part of a full image)")
set(USE_APRILTAG FALSE CACHE BOOL "Compile with apriltag detection for robots")

if(USE_DC1394 AND USE_mvIMPACT)
	message(FATAL_ERROR "DC1394 and mvImpact are not compatible: mvImpact crashes when creating device manager")
endif()

if(USE_QT5)
	cmake_minimum_required(VERSION 2.8.9)
	if(POLICY CMP0020)
	    cmake_policy(SET CMP0020 NEW) # remove if CMake >= 2.8.11 required
	endif()
	if(POLICY CMP0043) # compatibility with CMake 3.0.1
	    cmake_policy(SET CMP0043 OLD)
	endif()
	if(POLICY CMP0071) # compatibility with CMake 3.10.0
	    cmake_policy(SET CMP0071 OLD)
	endif()
else()
	cmake_minimum_required(VERSION 2.6)
endif()

#defines
add_definitions(-D UNIX)
add_definitions(-D LINUX)
add_definitions(-D _GNU_SOURCE)
add_definitions(-std=c++11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Find third-party libraries
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

find_package(Protobuf REQUIRED)
if(EXISTS ${PROTOBUF_PROTOC_EXECUTABLE})
	message(STATUS "Found PROTOBUF Compiler: ${PROTOBUF_PROTOC_EXECUTABLE}")
else()
	message(FATAL_ERROR "Could not find PROTOBUF Compiler")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(Eigen3 REQUIRED eigen3)

if(USE_QT5)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(CMAKE_AUTOMOC ON)
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Widgets REQUIRED)
	#find_package(Qt5Network REQUIRED)
	find_package(Qt5OpenGL REQUIRED)
	# replaced by automoc
	macro(qt4_wrap_cpp VARNAME)
		set(${VARNAME} "")
	endmacro()
	# wrap functions
	macro(qt4_wrap_ui)
		qt5_wrap_ui(${ARGN})
	endmacro()
	macro(qt4_add_resources)
		qt5_add_resources(${ARGN})
	endmacro()
	set(QT_LIBRARIES "")
else()
	find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui QtOpenGL QtNetwork REQUIRED)
	include ( ${QT_USE_FILE} )
endif()

include_directories(
	${PROJECT_BINARY_DIR}
	${Eigen3_INCLUDE_DIRS}
	${PROTOBUF_INCLUDE_DIRS}
)

include(src/shared/CMakeLists.txt.inc)

# Note: Bluefox SDK is not compatible with some components of OpenCV. Take care when enabling more.
find_package( OpenCV REQUIRED COMPONENTS core imgproc imgcodecs )
if( OpenCV_FOUND )
    message(STATUS "OpenCV ${OpenCV_VERSION} found")
    if(OpenCV_VERSION_MAJOR LESS 3)
        message(FATAL_ERROR "OpenCV major version >= 3 is required")
    endif()
    include_directories(${OpenCV_INCLUDE_DIRS})
    add_definitions(-D OPENCV)
else()
    message(FATAL_ERROR "OpenCV NOT found")
endif()

if( USE_mvIMPACT )
	find_package(mvIMPACT REQUIRED)
	include_directories(${mvIMPACT_INCLUDE_DIR})
	add_definitions(-D MVIMPACT3)
	add_definitions(-D MVIMPACT2)
	set (SHARED_SRCS ${SHARED_SRCS}
		  ${shared_dir}/capture/capture_bluefox2.cpp
		  ${shared_dir}/capture/capture_bluefox3.cpp
		)
	set (SHARED_HEADERS ${SHARED_HEADERS}
		  ${shared_dir}/capture/capture_bluefox2.h
		  ${shared_dir}/capture/capture_bluefox3.h
		)
	message(STATUS "mvIMPACT enabled")
else()
	message(STATUS "mvIMPACT disabled")
endif()

if( USE_FLYCAP )
	set(FLYCAP_HINTS "/usr/include/flycapture")
	find_package(FLYCAP REQUIRED)
	include_directories(${FLYCAP_INCLUDE_DIRS})
	add_definitions(-D FLYCAP)
	set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capture_flycap.cpp)
	set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capture_flycap.h)
	set (FLYCAP_LIBS flycapture)
	message(STATUS "Flycap enabled")
else()
  message(STATUS "Flycap disabled")
endif()

if(USE_DC1394)
	pkg_check_modules(DC1394 REQUIRED libdc1394-2)
	add_definitions(-D DC1394)
	set (SHARED_SRCS ${SHARED_SRCS}
			${shared_dir}/capture/capturedc1394v2.cpp
			)
	set (SHARED_HEADERS ${SHARED_HEADERS}
			${shared_dir}/capture/capturedc1394v2.h
			)
	message(STATUS "DC1394 enabled")
else()
	add_definitions(-D NO_DC1394_CONVERSIONS)
	set(DC1394_LIBRARIES "")
	message(STATUS "DC1394 disabled")
endif()

if( USE_PYLON )
	find_package( PYLON5 REQUIRED )
	include_directories(SYSTEM ${PYLON5_INCLUDE_DIRS})
	add_definitions(-DPYLON5)
	set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capture_basler.cpp)
	set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capture_basler.h)
	message(STATUS "Pylon5 enabled")
else()
	message(STATUS "Pylon5 disabled")
endif()

if( USE_SPINNAKER )
	find_package( SPINNAKER REQUIRED )
	add_definitions( -DSPINNAKER )
	include_directories( ${SPINNAKER_INCLUDE_DIRS} )
	set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capture_spinnaker.cpp)
	set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capture_spinnaker.h)
	message(STATUS "SPINNAKER enabled")
else()
	set( SPINNAKER_LIBS "" )
	message(STATUS "SPINNAKER disabled")
endif()

if( USE_V4L )
	add_definitions( -DV4L )
	set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capturev4l.cpp)
	set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capturev4l.h)
	message(STATUS "Video 4 Linux enabled")
else()
	message(STATUS "Video 4 Linux disabled")
endif()

if( USE_SPLITTER )
	add_definitions( -DCAMERA_SPLITTER )
	set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capture_splitter.cpp)
	set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capture_splitter.h)
	set (SRCS ${SRCS}
			src/app/plugins/plugin_distribute.cpp
			src/app/stacks/DistributorStack.cpp
		)

	message(STATUS "Camera splitter enabled")
else()
	message(STATUS "Camera splitter disabled")
endif()

if(USE_APRILTAG)
  find_package(apriltag REQUIRED)
  add_definitions(-DAPRILTAG)
  set(SRCS ${SRCS} src/app/plugins/plugin_apriltag.cpp)
  message(STATUS "apriltag enabled")
else()
  message(STATUS "apriltag disabled")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/app)
include_directories(${PROJECT_SOURCE_DIR}/src/app/gui)
include_directories(${PROJECT_SOURCE_DIR}/src/app/plugins)
include_directories(${PROJECT_SOURCE_DIR}/src/app/stacks)

set (SRCS ${SRCS}
	src/app/capture_thread.cpp
	src/app/framedata.cpp
	src/app/main.cpp

	src/app/gui/automatedcolorcalibwidget.cpp
	src/app/gui/cameracalibwidget.cpp
	src/app/gui/colorpicker.cpp
	src/app/gui/glLUTwidget.cpp
	src/app/gui/glwidget.cpp
	src/app/gui/lutwidget.cpp
	src/app/gui/mainwindow.cpp
	src/app/gui/realtimedisplaywidget.cpp
	src/app/gui/renderoptions.cpp
	src/app/gui/videowidget.cpp
	src/app/gui/jog_dial.cpp

	src/app/plugins/plugin_cameracalib.cpp
	src/app/plugins/plugin_colorcalib.cpp
	src/app/plugins/plugin_colorthreshold.cpp
	src/app/plugins/plugin_detect_balls.cpp
	src/app/plugins/plugin_detect_robots.cpp
	src/app/plugins/plugin_find_blobs.cpp
	src/app/plugins/plugin_greyscale.cpp
	src/app/plugins/plugin_publishgeometry.cpp
	src/app/plugins/plugin_legacypublishgeometry.cpp
	src/app/plugins/plugin_runlength_encode.cpp
	src/app/plugins/plugin_sslnetworkoutput.cpp
	src/app/plugins/plugin_legacysslnetworkoutput.cpp
	src/app/plugins/plugin_visualize.cpp
	src/app/plugins/plugin_dvr.cpp
	src/app/plugins/plugin_auto_color_calibration.cpp
	src/app/plugins/visionplugin.cpp

	src/app/stacks/multistack_robocup_ssl.cpp
	src/app/stacks/multivisionstack.cpp
	src/app/stacks/stack_robocup_ssl.cpp
	src/app/stacks/visionstack.cpp

	${OPTIONAL_SRCS}
)

qt4_wrap_cpp (MOC_SRCS
	src/app/capture_thread.h

	src/app/gui/automatedcolorcalibwidget.h
	src/app/gui/cameracalibwidget.h
	src/app/gui/glLUTwidget.h
	src/app/gui/glwidget.h
	src/app/gui/lutwidget.h
	src/app/gui/mainwindow.h
	src/app/gui/videowidget.h
	src/app/gui/jog_dial.h

	src/shared/util/lut3d.h

	src/app/plugins/plugin_dvr.h
	src/app/plugins/plugin_publishgeometry.h
	src/app/plugins/plugin_legacypublishgeometry.h
	src/app/plugins/visionplugin.h
	src/app/plugins/plugin_colorcalib.h
	src/app/plugins/plugin_colorthreshold.h
	src/app/plugins/plugin_auto_color_calibration.h

	src/app/stacks/multistack_robocup_ssl.h

	${OPTIONAL_HEADERS}
)

qt4_wrap_ui (UI_SRCS
	src/app/gui/mainwindow.ui
	src/app/gui/videowidget.ui
	${OPTIONAL_UI_SRCS}
)

qt4_add_resources(RC_SRCS
	src/app/gui/icons/icons_gui.qrc
	${SHARED_RC}
)

## generate moc files for graphicalClient
qt4_wrap_cpp(LCLIENT_MOC_SRCS
    src/logClient/CentralWindow.h
    src/logClient/ClientThreading.h
    src/logClient/GraphicsPrimitives.h
    src/logClient/LogControl.h
)

## generate moc files for graphicalClient2
qt4_wrap_cpp(GCLIENT_MOC_SRCS
    src/graphicalClient/soccerview.h
)

## shared qt wrappings

qt4_wrap_cpp (SHARED_MOC_SRCS
	${SHARED_HEADERS}
)

qt4_add_resources(SHARED_RC_SRCS
	${SHARED_RC}
)

set (arch ${CMAKE_SYSTEM_PROCESSOR})

message (STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message (STATUS "Arch: ${arch}")

if (${arch} MATCHES "x86_64")
	#set compiler flags here for x86_64
	set (cc_flags "")
elseif (${arch} MATCHES "x86")
	#set compiler flags here for x86
	set (cc_flags "")
endif()

# make sure to use C++98 and enable warnings
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi")
# enable warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#flags to set in debug mode
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wl,--no-as-needed")

#flags to set in release mode
set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native -Wl,--no-as-needed")

message( STATUS "CC Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message (STATUS "Custom CC Flags: ${cc_flags}")

add_definitions(${cc_flags})

## build the common code
add_library(sslvision ${SHARED_MOC_SRCS} ${SHARED_RC_SRCS} ${CC_PROTO} ${SHARED_SRCS})
add_dependencies(sslvision GenerateProto)

set (libs ${QT_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${PROTOBUF_LIBRARIES}
	${DC1394_LIBRARIES}
	${mvIMPACT_LIBS}
	${PYLON5_LIBRARIES}
	${OpenCV_LIBS}
	${FLYCAP_LIBS}
	${SPINNAKER_LIBS}
)
target_link_libraries(sslvision ${libs})
if(USE_APRILTAG)
  target_link_libraries(sslvision apriltag)
endif()
if(USE_QT5)
	qt5_use_modules(sslvision Widgets)
endif()
set (libs ${libs} sslvision)

## build the main app
set (target vision)
add_executable(${target} ${UI_SRCS} ${MOC_SRCS} ${RC_SRCS} ${SRCS})
target_link_libraries(${target} ${libs})
if (USE_APRILTAG)
  target_link_libraries(${target} apriltag)
endif()
if(USE_QT5)
	qt5_use_modules(${target} Widgets OpenGL)
endif()

##build non graphical client
set (client client)
add_executable(${client} src/client/main.cpp )
target_link_libraries(${client} ${libs})
if(USE_QT5)
	qt5_use_modules(${client} Core)
endif()

##build logging client
set (lclient logClient)
add_executable(${lclient} ${LCLIENT_MOC_SRCS}
	src/logClient/main.cpp
	src/logClient/CentralWindow.cpp
	src/logClient/ClientThreading.cpp
	src/logClient/GraphicsPrimitives.cpp
	src/logClient/LogControl.cpp
)
target_link_libraries(${lclient} ${libs})
if(USE_QT5)
	qt5_use_modules(${lclient} Widgets OpenGL)
endif()

##build graphical client
set (gclient graphicalClient)
add_executable(${gclient} ${GCLIENT_MOC_SRCS}
  src/graphicalClient/main.cpp
  src/graphicalClient/soccerview.cpp
  src/graphicalClient/gltext.cpp
)
target_link_libraries(${gclient} ${libs})
if(USE_QT5)
	qt5_use_modules(${gclient} Widgets OpenGL)
endif()
