#directories of sources of the vision client
PREFIX = .

INCLUDEPATH += $${PREFIX}

###################
### APPLICATION ###

SOURCES += $${PREFIX}/main.cpp

INCLUDEPATH += $${SHARED_DIR}/util

HEADERS += $${SHARED_DIR}/util/util.h

HEADERS += $${SHARED_DIR}/util/timer.h

INCLUDEPATH += $${SHARED_DIR}/net

SOURCES += $${SHARED_DIR}/net/netraw.cpp
HEADERS += $${SHARED_DIR}/net/netraw.h

SOURCES += $${SHARED_DIR}/net/robocup_ssl_client.cpp
HEADERS += $${SHARED_DIR}/net/robocup_ssl_client.h
