QT = core gui
unix {
	QT += gui-private
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UGlobalHotkey
TEMPLATE = lib
QMAKE_CXXFLAGS += -std=c++11

DEFINES += UGLOBALHOTKEY_LIBRARY

HEADERS += \
    ukeysequence.h \
    uglobalhotkeys.h \
    uexception.h \
    hotkeymap.h \
    uglobal.h

SOURCES += \
    ukeysequence.cpp \
    uglobalhotkeys.cpp \
    uexception.cpp \

linux: LIBS += -lxcb -lxcb-keysyms
mac: LIBS += -framework Carbon

windows {
    *-g++* {
        LIBS += -luser32
    }
    *-msvc* {
        LIBS += user32.lib
    }
}
