#all files below belong to the vartype system:

PREFIX = $${SHARED_DIR}/vartypes

INCLUDEPATH += $${PREFIX}
INCLUDEPATH += $${PREFIX}/primitives
INCLUDEPATH += $${PREFIX}/xml
INCLUDEPATH += $${PREFIX}/gui
INCLUDEPATH += $${PREFIX}/enhanced

SOURCES += $${PREFIX}/VarTypes.cpp
HEADERS += $${PREFIX}/VarTypes.h

SOURCES += $${PREFIX}/VarXML.cpp
HEADERS += $${PREFIX}/VarXML.h

SOURCES += $${PREFIX}/VarBase64.cpp
HEADERS += $${PREFIX}/VarBase64.h

SOURCES += $${PREFIX}/primitives/VarData.cpp
HEADERS += $${PREFIX}/primitives/VarData.h

SOURCES += $${PREFIX}/primitives/VarBool.cpp
HEADERS += $${PREFIX}/primitives/VarBool.h

SOURCES += $${PREFIX}/primitives/VarDouble.cpp
HEADERS += $${PREFIX}/primitives/VarDouble.h

SOURCES += $${PREFIX}/primitives/VarInt.cpp
HEADERS += $${PREFIX}/primitives/VarInt.h

SOURCES += $${PREFIX}/primitives/VarList.cpp
HEADERS += $${PREFIX}/primitives/VarList.h

SOURCES += $${PREFIX}/primitives/VarExternal.cpp
HEADERS += $${PREFIX}/primitives/VarExternal.h

SOURCES += $${PREFIX}/primitives/VarString.cpp
HEADERS += $${PREFIX}/primitives/VarString.h

SOURCES += $${PREFIX}/primitives/VarStringEnum.cpp
HEADERS += $${PREFIX}/primitives/VarStringEnum.h

SOURCES += $${PREFIX}/primitives/VarBlob.cpp
HEADERS += $${PREFIX}/primitives/VarBlob.h

SOURCES += $${PREFIX}/primitives/VarQWidget.cpp
HEADERS += $${PREFIX}/primitives/VarQWidget.h

SOURCES += $${PREFIX}/primitives/VarTrigger.cpp
HEADERS += $${PREFIX}/primitives/VarTrigger.h

SOURCES += $${PREFIX}/xml/xmlParser.cpp
HEADERS += $${PREFIX}/xml/xmlParser.h

SOURCES += $${PREFIX}/gui/VarTreeView.cpp
HEADERS += $${PREFIX}/gui/VarTreeView.h

SOURCES += $${PREFIX}/gui/VarItem.cpp
HEADERS += $${PREFIX}/gui/VarItem.h

SOURCES += $${PREFIX}/gui/VarItemDelegate.cpp
HEADERS += $${PREFIX}/gui/VarItemDelegate.h

SOURCES += $${PREFIX}/gui/VarTreeModel.cpp
HEADERS += $${PREFIX}/gui/VarTreeModel.h

SOURCES += $${PREFIX}/gui/VarTreeViewOptions.cpp
HEADERS += $${PREFIX}/gui/VarTreeViewOptions.h

