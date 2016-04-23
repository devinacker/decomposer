QT       += core gui widgets

TARGET = decomposer
TEMPLATE = app
CONFIG += c++11

CONFIG(debug, debug|release) {
    DESTDIR = debug
}
CONFIG(release, debug|release) {
    DESTDIR = release
}

OBJECTS_DIR = obj/$$DESTDIR
MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR

include(src/devices/MIDI.pri)

SOURCES += \
    src/main.cpp\
    src/mainwindow.cpp \
    src/InstrumentPanel.cpp \
    src/DevicePanel.cpp \
    src/Instrument.cpp

HEADERS  += \
    src/mainwindow.h \
    src/InstrumentPanel.h \
    src/Instrument.h \
    src/DevicePanel.h

FORMS    += \
    src/mainwindow.ui \
    src/InstrumentPanel.ui \
    src/DevicePanel.ui
