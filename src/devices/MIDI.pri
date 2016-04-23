#
# Qt MIDI backend for Decomposer
#

SOURCES += \
    src/devices/MIDIdefs.cpp \
    src/devices/MIDIoutput.cpp

HEADERS += \
    src/devices/MIDIinput.h \
    src/devices/MIDIoutput.h \
    src/devices/MIDIdevice.h \
    src/devices/MIDIdefs.h

win32 {
    QMAKE_LIBS += -lwinmm

    SOURCES += \
        src/devices/MIDIinput_winmm.cpp \
        src/devices/MIDIoutput_winmm.cpp
}

!win32 {
    error(MIDI device support not implemented for this platform)
}
