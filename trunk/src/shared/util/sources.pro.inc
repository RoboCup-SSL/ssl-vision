#all files below belong to the vartype system:

PREFIX = $${SHARED_DIR}/util

INCLUDEPATH += $${PREFIX}

SOURCES += $${PREFIX}/global_random.cpp
HEADERS += $${PREFIX}/global_random.h

SOURCES += $${PREFIX}/image.cpp
HEADERS += $${PREFIX}/image.h

SOURCES += $${PREFIX}/image_io.cpp
HEADERS += $${PREFIX}/image_io.h

SOURCES += $${PREFIX}/qgetopt.cpp
HEADERS += $${PREFIX}/qgetopt.h

SOURCES += $${PREFIX}/random.cpp
HEADERS += $${PREFIX}/random.h

SOURCES += $${PREFIX}/rawimage.cpp
HEADERS += $${PREFIX}/rawimage.h

SOURCES += $${PREFIX}/texture.cpp
HEADERS += $${PREFIX}/texture.h

HEADERS += $${PREFIX}/framecounter.h

HEADERS += $${PREFIX}/quaternion.h

HEADERS += $${PREFIX}/colors.h

HEADERS += $${PREFIX}/timer.h

HEADERS += $${PREFIX}/gvector.h

HEADERS += $${PREFIX}/nvector.h

HEADERS += $${PREFIX}/geomalgo.h

HEADERS += $${PREFIX}/geometry.h

HEADERS += $${PREFIX}/bbox.h

HEADERS += $${PREFIX}/range.h

HEADERS += $${PREFIX}/pose.h

HEADERS += $${PREFIX}/zoom.h

HEADERS += $${PREFIX}/bitflags.h
