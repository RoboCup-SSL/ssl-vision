get_filename_component(shared_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

include_directories(${shared_dir}/capture)
include_directories(${shared_dir}/cmvision)
include_directories(${shared_dir}/cmpattern)
include_directories(${shared_dir}/gl)
include_directories(${shared_dir}/net)
include_directories(${shared_dir}/util)
include_directories(${shared_dir}/vartypes)
include_directories(${shared_dir}/vartypes/primitives)
include_directories(${shared_dir}/vartypes/enhanced)
include_directories(${shared_dir}/vartypes/gui)
include_directories(${shared_dir}/vartypes/xml)

if(USE_APRILTAG)
  include_directories(${shared_dir}/apriltag)
endif()

set (SHARED_SRCS
	${shared_dir}/capture/capturefromfile.cpp
	${shared_dir}/capture/capture_generator.cpp
	${shared_dir}/capture/captureinterface.cpp

	${shared_dir}/cmpattern/cmpattern_pattern.cpp
	${shared_dir}/cmpattern/cmpattern_team.cpp
	${shared_dir}/cmpattern/cmpattern_teamdetector.cpp

	${shared_dir}/cmvision/cmvision_histogram.cpp
	${shared_dir}/cmvision/cmvision_region.cpp
	${shared_dir}/cmvision/cmvision_threshold.cpp

	${shared_dir}/gl/glcamera.cpp
	${shared_dir}/gl/globject.cpp

	${shared_dir}/net/netraw.cpp
	${shared_dir}/net/robocup_ssl_client.cpp
	${shared_dir}/net/robocup_ssl_server.cpp

	${shared_dir}/util/affinity_manager.cpp
	${shared_dir}/util/camera_calibration.cpp
	${shared_dir}/util/conversions.cpp
	${shared_dir}/util/global_random.cpp
	${shared_dir}/util/image.cpp
	${shared_dir}/util/image_io.cpp
	${shared_dir}/util/lut3d.cpp
	${shared_dir}/util/qgetopt.cpp
	${shared_dir}/util/random.cpp
	${shared_dir}/util/rawimage.cpp
	${shared_dir}/util/ringbuffer.cpp
	${shared_dir}/util/texture.cpp
  ${shared_dir}/util/framelimiter.cpp
	${shared_dir}/util/initial_color_calibrator.cpp

	${shared_dir}/vartypes/VarBase64.cpp
	${shared_dir}/vartypes/VarNotifier.cpp
	${shared_dir}/vartypes/VarTypes.cpp
	${shared_dir}/vartypes/VarXML.cpp
  ${shared_dir}/vartypes/VarTypesInstance.cpp
  ${shared_dir}/vartypes/VarTypesFactory.cpp
	${shared_dir}/vartypes/xml/xmlParser.cpp

  ${shared_dir}/vartypes/primitives/VarStringVal.cpp
  ${shared_dir}/vartypes/primitives/VarBoolVal.cpp
  ${shared_dir}/vartypes/primitives/VarVal.cpp
  ${shared_dir}/vartypes/primitives/VarIntVal.cpp
  ${shared_dir}/vartypes/primitives/VarDoubleVal.cpp

  ${shared_dir}/vartypes/primitives/VarType.cpp
  ${shared_dir}/vartypes/primitives/VarBlob.cpp
  ${shared_dir}/vartypes/primitives/VarBool.cpp
  ${shared_dir}/vartypes/primitives/VarDouble.cpp
  ${shared_dir}/vartypes/primitives/VarExternal.cpp
  ${shared_dir}/vartypes/primitives/VarInt.cpp
  ${shared_dir}/vartypes/primitives/VarList.cpp
  ${shared_dir}/vartypes/primitives/VarQWidget.cpp
  ${shared_dir}/vartypes/primitives/VarSelection.cpp
  ${shared_dir}/vartypes/primitives/VarString.cpp
  ${shared_dir}/vartypes/primitives/VarStringEnum.cpp
  ${shared_dir}/vartypes/primitives/VarTrigger.cpp

  #${shared_dir}/vartypes/primitives/VarProtoBufferVal.cpp
  #${shared_dir}/vartypes/primitives/VarProtoBuffer.cpp

	${shared_dir}/vartypes/gui/VarItem.cpp
	${shared_dir}/vartypes/gui/VarItemDelegate.cpp
	${shared_dir}/vartypes/gui/VarTreeModel.cpp
	${shared_dir}/vartypes/gui/VarTreeView.cpp
	${shared_dir}/vartypes/gui/VarTreeViewOptions.cpp

	#${shared_dir}/vartypes/gui/experimental/GraphScene.cpp
	#${shared_dir}/vartypes/gui/experimental/TimeControlWidget.cpp
	#${shared_dir}/vartypes/gui/experimental/TimeGraphContainer.cpp
	#${shared_dir}/vartypes/gui/experimental/ValueVis.cpp
	#${shared_dir}/vartypes/gui/experimental/VisContainer.cpp

	#${shared_dir}/vartypes/enhanced/TimeLine.cpp
	#${shared_dir}/vartypes/enhanced/TimeVar.cpp

  ${shared_dir}/util/helpers.cpp
  ${shared_dir}/util/field.cpp
  ${shared_dir}/util/team_tags.cpp
)

if(USE_APRILTAG)
  set(SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/apriltag/tagCustom20h7.c)
endif()

#only ones that need to be moc'ed
set (SHARED_HEADERS
	${shared_dir}/capture/capturefromfile.h
  ${shared_dir}/capture/capture_generator.h

	${shared_dir}/cmpattern/cmpattern_team.h
	${shared_dir}/cmpattern/cmpattern_teamdetector.h

	${shared_dir}/util/field.h
	${shared_dir}/util/team_tags.h

	${shared_dir}/vartypes/VarNotifier.h

  ${shared_dir}/vartypes/primitives/VarType.h
	${shared_dir}/vartypes/primitives/VarBlob.h
	${shared_dir}/vartypes/primitives/VarBool.h
	${shared_dir}/vartypes/primitives/VarDouble.h
	${shared_dir}/vartypes/primitives/VarExternal.h
	${shared_dir}/vartypes/primitives/VarInt.h
	${shared_dir}/vartypes/primitives/VarList.h
	${shared_dir}/vartypes/primitives/VarQWidget.h
	${shared_dir}/vartypes/primitives/VarSelection.h
	${shared_dir}/vartypes/primitives/VarString.h
	${shared_dir}/vartypes/primitives/VarStringEnum.h
	${shared_dir}/vartypes/primitives/VarTrigger.h
  #${shared_dir}/vartypes/primitives/VarProtoBuffer.h

	${shared_dir}/vartypes/gui/VarItem.h
	${shared_dir}/vartypes/gui/VarItemDelegate.h
	${shared_dir}/vartypes/gui/VarTreeModel.h
	${shared_dir}/vartypes/gui/VarTreeView.h
)

if(USE_APRILTAG)
  set(SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/apriltag/tagCustom20h7.h)
endif()


set (SHARED_RC
	${shared_dir}/vartypes/gui/icons/icons.qrc
)

### protocol buffer generation code

set (PROTO_PATH ${shared_dir}/proto)

set (PROTO_FILES
	messages_robocup_ssl_detection
	messages_robocup_ssl_geometry
	messages_robocup_ssl_wrapper
	messages_robocup_ssl_refbox_log
  messages_robocup_ssl_geometry_legacy
  messages_robocup_ssl_wrapper_legacy
)

set (CC_PROTO)
set (H_PROTO)

foreach (file ${PROTO_FILES})
	set (cc_file ${file}.pb.cc)
	set (h_file ${file}.pb.h)
	set (proto_file ${file}.proto)

	add_custom_command(OUTPUT ${cc_file} ${h_file}
			COMMAND protoc --cpp_out=${PROJECT_BINARY_DIR} --proto_path=${PROTO_PATH} ${PROTO_PATH}/${proto_file}
			DEPENDS ${PROTO_PATH}/${proto_file}
			VERBATIM
	)

	list (APPEND CC_PROTO ${PROJECT_BINARY_DIR}/${cc_file})
	list (APPEND H_PROTO ${PROJECT_BINARY_DIR}/${h_file})
endforeach()

add_custom_target(GenerateProto
	    DEPENDS ${CC_PROTO} ${H_PROTO}
	    VERBATIM
	)
