#all classes for capturing video:

PREFIX = $${SHARED_DIR}/capture

INCLUDEPATH += $${PREFIX}

SOURCES += $${PREFIX}/capturedc1394v2.cpp
HEADERS += $${PREFIX}/capturedc1394v2.h

SOURCES += $${PREFIX}/captureinterface.cpp
HEADERS += $${PREFIX}/captureinterface.h

