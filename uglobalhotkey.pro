QT = core gui
unix {
    QT += gui-private
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UGlobalHotkey
TEMPLATE = lib
CONFIG += c++11

# Switch ABI to export (vs import, which is default)
DEFINES += UGLOBALHOTKEY_LIBRARY

header_files.path = /usr/local/include/$$TARGET
header_files.files = hotkeymap.h uexception.h uglobal.h uglobalhotkeys.h ukeysequence.h 
INSTALLS += header_files

library_files.path = /usr/local/lib/
library_files.files = libUGlobalHotkey.so.1.0.0
library_files.extra = ln -sf libUGlobalHotkey.so.1.0.0 $$library_files.path/libUGlobalHotkey.so.1.0;
library_files.extra += ln -sf libUGlobalHotkey.so.1.0 $$library_files.path/libUGlobalHotkey.so.1;
library_files.extra += ln -sf libUGlobalHotkey.so.1 $$library_files.path/libUGlobalHotkey.so;
INSTALLS += library_files

include(uglobalhotkey-headers.pri)
include(uglobalhotkey-sources.pri)
include(uglobalhotkey-libs.pri)
