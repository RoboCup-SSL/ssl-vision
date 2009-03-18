#all opengl-related classes

PREFIX = $${SHARED_DIR}/net

INCLUDEPATH += $${PREFIX}

SOURCES += $${PREFIX}/netraw.cpp
HEADERS += $${PREFIX}/netraw.h

SOURCES += $${PREFIX}/robocup_ssl_server.cpp
HEADERS += $${PREFIX}/robocup_ssl_server.h

SOURCES += $${PREFIX}/robocup_ssl_client.cpp
HEADERS += $${PREFIX}/robocup_ssl_client.h

