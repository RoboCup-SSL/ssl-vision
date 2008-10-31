CONFIG -= warn_off
CONFIG += debug
CONFIG += qt
CONFIG += console

SHARED_DIR = shared

INCLUDEPATH += $${SHARED_DIR}

unix {
  DEFINES+=UNIX
  DEFINES+=LINUX
  DEFINES+=_GNU_SOURCE
}



