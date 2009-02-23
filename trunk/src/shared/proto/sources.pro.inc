#messages defined through google protocol buffers (as compiled by protoc)

include( $${SHARED_DIR}/proto/proto.pro.inc )

PREFIX = $${SHARED_DIR}/proto/cpp

INCLUDEPATH += $${PREFIX}

SOURCES += $${PREFIX}/*.pb.cc
HEADERS += $${PREFIX}/*.pb.h

