QT             += core gui widgets
SOURCES        += notes.cpp \
                  geometry.cpp
HEADERS        += notes.h \
                  geometry.h
RESOURCES      += notes.qrc
CONFIG         += c++11
win32: LIBS    += -lUser32
win32: RC_ICONS = notes.ico
linux: LIBS    += -lX11
