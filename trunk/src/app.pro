#This is the main project file to build the vision server/gui

#read the global configuration file
include( config.pro.inc )

#where to place built objects
OBJECTS_DIR = ../build/app/obj

#where to place temporary moc sources
MOC_DIR = ../build/app/moc

#where to place auto-generated UI files
UI_DIR = ../build/app/ui

#where to place intermediate resource files
RCC_DIR = ../build/app/resources

#add libeigen include path
INCLUDEPATH += /usr/include/eigen2

#add libdc1394
LIBS += -ldc1394

#add jpeg support
LIBS += -ljpeg

#add google protocol buffers
LIBS += -lprotobuf

#add opengl support
LIBS += -lGL -lGLU

#enable gl
QT     += opengl
#enable networking
QT     += network

#where to build the vision executive
TARGET = ../bin/vision

#resources
RESOURCES += $${SHARED_DIR}/vartypes/gui/icons/icons.qrc
RESOURCES += app/gui/icons/icons_gui.qrc

#include shared sources
include ( $${SHARED_DIR}/sources.pro.inc )

#include actual list of source files for application
include ( app.sources.pro.inc )

DEPENDPATH = INCLUDEPATH

