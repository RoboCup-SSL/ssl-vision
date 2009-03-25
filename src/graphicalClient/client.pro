# This is the main project file to build the vision server/gui
# read the global configuration file
include( ../config.pro.inc )

# where to place built objects
OBJECTS_DIR = ../../build/client/obj

# where to place temporary moc sources
MOC_DIR = ../../build/client/moc

# where to place auto-generated UI files
UI_DIR = ../../build/client/ui

# where to place intermediate resource files
RCC_DIR = ../../build/client/resources

# add google protocol buffers
LIBS += -lprotobuf

# add opengl support
LIBS += -lGL \
    -lGLU

# enable gl
QT += opengl

# enable networking
QT += network

# where to build the client executive
TARGET = ../../bin/graphicalClient

# resources
RESOURCES += $${SHARED_DIR}/vartypes/gui/icons/icons.qrc
RESOURCES += ../app/gui/icons/icons_gui.qrc

# include shared sources
include( $${SHARED_DIR}/proto/sources.pro.inc )

# include actual list of source files for application
include ( client.sources.pro.inc )
DEPENDPATH = INCLUDEPATH
HEADERS += ClientThreading.h \
    ../shared/util/field_default_constants.h
SOURCES += ClientThreading.cpp
