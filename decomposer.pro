QT       += core gui widgets

TARGET = decomposer
TEMPLATE = app
CONFIG += c++11

win32 {
    QMAKE_LIBS += -lwinmm

    SOURCES += \
        src/devices/MIDIinput_winmm.cpp \
        src/devices/MIDIoutput_winmm.cpp
}

SOURCES += \
    src/main.cpp\
    src/mainwindow.cpp \
    src/devices/MIDIdefs.cpp \
    src/devices/MIDIoutput.cpp \
    src/InstrumentPanel.cpp \
    src/DevicePanel.cpp \
    src/Instrument.cpp

HEADERS  += \
    src/mainwindow.h \
    src/devices/MIDIinput.h \
    src/devices/MIDIoutput.h \
    src/devices/MIDIdevice.h \
    src/devices/MIDIdefs.h \
    src/InstrumentPanel.h \
    src/Instrument.h \
    src/DevicePanel.h

FORMS    += \
    src/mainwindow.ui \
    src/InstrumentPanel.ui \
    src/DevicePanel.ui
