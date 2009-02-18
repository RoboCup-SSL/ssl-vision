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

SOURCES += $${PREFIX}/plugins/plugin_colorcalib.cpp
HEADERS += $${PREFIX}/plugins/plugin_colorcalib.h

SOURCES += $${PREFIX}/plugins/plugin_colorthreshold.cpp
HEADERS += $${PREFIX}/plugins/plugin_colorthreshold.h

SOURCES += $${PREFIX}/plugins/plugin_runlength_encode.cpp
HEADERS += $${PREFIX}/plugins/plugin_runlength_encode.h

SOURCES += $${PREFIX}/plugins/plugin_find_blobs.cpp
HEADERS += $${PREFIX}/plugins/plugin_find_blobs.h

SOURCES += $${PREFIX}/plugins/plugin_detect_balls.cpp
HEADERS += $${PREFIX}/plugins/plugin_detect_balls.h

SOURCES += $${PREFIX}/plugins/plugin_visualize.cpp
HEADERS += $${PREFIX}/plugins/plugin_visualize.h

############################
### SINGLE-CAMERA STACKS ###

SOURCES += $${PREFIX}/stacks/visionstack.cpp
HEADERS += $${PREFIX}/stacks/visionstack.h

SOURCES += $${PREFIX}/stacks/stack_robocup_ssl.cpp
HEADERS += $${PREFIX}/stacks/stack_robocup_ssl.h

HEADERS += $${PREFIX}/stacks/stacks.h


###########################
### MULTI-CAMERA STACKS ###

SOURCES += $${PREFIX}/stacks/multivisionstack.cpp
HEADERS += $${PREFIX}/stacks/multivisionstack.h

SOURCES += $${PREFIX}/stacks/multistack_robocup_ssl.cpp
HEADERS += $${PREFIX}/stacks/multistack_robocup_ssl.h

HEADERS += $${PREFIX}/stacks/multistacks.h
