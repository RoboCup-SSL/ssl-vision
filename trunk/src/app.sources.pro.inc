#directories of sources of the vision app
PREFIX = app

INCLUDEPATH += $${PREFIX}
INCLUDEPATH += $${PREFIX}/gui
INCLUDEPATH += $${PREFIX}/plugins
INCLUDEPATH += $${PREFIX}/stacks

###################
### APPLICATION ###

SOURCES += $${PREFIX}/main.cpp

SOURCES += $${PREFIX}/capture_thread.cpp
HEADERS += $${PREFIX}/capture_thread.h

SOURCES += $${PREFIX}/framedata.cpp
HEADERS += $${PREFIX}/framedata.h

HEADERS += $${PREFIX}/capturestats.h

HEADERS += $${PREFIX}/videostats.h

#########################
### QT-DESIGNER FORMS ###

FORMS += $${PREFIX}/gui/mainwindow.ui

FORMS += $${PREFIX}/gui/videowidget.ui

###########
### GUI ###

SOURCES += $${PREFIX}/gui/mainwindow.cpp
HEADERS += $${PREFIX}/gui/mainwindow.h

SOURCES += $${PREFIX}/gui/lutwidget.cpp
HEADERS += $${PREFIX}/gui/lutwidget.h

SOURCES += $${PREFIX}/gui/videowidget.cpp
HEADERS += $${PREFIX}/gui/videowidget.h

SOURCES += $${PREFIX}/gui/colorpicker.cpp
HEADERS += $${PREFIX}/gui/colorpicker.h

SOURCES += $${PREFIX}/gui/glwidget.cpp
HEADERS += $${PREFIX}/gui/glwidget.h

SOURCES += $${PREFIX}/gui/renderoptions.cpp
HEADERS += $${PREFIX}/gui/renderoptions.h

SOURCES += $${PREFIX}/gui/glLUTwidget.cpp
HEADERS += $${PREFIX}/gui/glLUTwidget.h

SOURCES += $${PREFIX}/gui/realtimedisplaywidget.cpp
HEADERS += $${PREFIX}/gui/realtimedisplaywidget.h

###############
### PLUGINS ###

SOURCES += $${PREFIX}/plugins/visionplugin.cpp
HEADERS += $${PREFIX}/plugins/visionplugin.h

##############
### STACKS ###

SOURCES += $${PREFIX}/stacks/visionstack.cpp
HEADERS += $${PREFIX}/stacks/visionstack.h

SOURCES += $${PREFIX}/stacks/stacks.cpp
HEADERS += $${PREFIX}/stacks/stacks.h

SOURCES += $${PREFIX}/stacks/multivisionstack.cpp
HEADERS += $${PREFIX}/stacks/multivisionstack.h

SOURCES += $${PREFIX}/stacks/multistacks.cpp
HEADERS += $${PREFIX}/stacks/multistacks.h

