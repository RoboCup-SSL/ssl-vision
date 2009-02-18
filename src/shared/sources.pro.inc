#all shared sources
#assumes correct compiler setup and definition of the following variable:
#SHARED_DIR

include( $${SHARED_DIR}/capture/sources.pro.inc )
include( $${SHARED_DIR}/vartypes/sources.pro.inc )
include( $${SHARED_DIR}/util/sources.pro.inc )
include( $${SHARED_DIR}/gl/sources.pro.inc )
include( $${SHARED_DIR}/cmvision/sources.pro.inc )
include( $${SHARED_DIR}/proto/sources.pro.inc )
