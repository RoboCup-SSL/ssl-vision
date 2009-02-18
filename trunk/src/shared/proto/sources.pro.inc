#messages defined through google protocol buffers (as compiled by protoc)

PREFIX = $${SHARED_DIR}/proto/cpp

INCLUDEPATH += $${PREFIX}

SOURCES += $${PREFIX}/messages_robocup_ssl_detection.pb.cc
HEADERS += $${PREFIX}/messages_robocup_ssl_detection.pb.h

SOURCES += $${PREFIX}/messages_robocup_ssl_geometry.pb.cc
HEADERS += $${PREFIX}/messages_robocup_ssl_geometry.pb.h
