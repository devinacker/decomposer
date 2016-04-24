#
# Qt MIDI backend for Decomposer
#

SOURCES += \
    $$PWD/MIDIdefs.cpp \
    $$PWD/MIDIoutput.cpp

HEADERS += \
    $$PWD/MIDIinput.h \
    $$PWD/MIDIoutput.h \
    $$PWD/MIDIdevice.h \
    $$PWD/MIDIdefs.h

win32 {
    message(building with WinMM)
    QMAKE_LIBS += -lwinmm

    SOURCES += \
        $$PWD/MIDIinput_winmm.cpp \
        $$PWD/MIDIoutput_winmm.cpp
}

else:unix:!macx {
    message(building with ALSA)
    QMAKE_LIBS += -lasound

    SOURCES +=  \
        $$PWD/MIDIinput_alsa.cpp \
        $$PWD/MIDIoutput_alsa.cpp \
        $$PWD/alsa.cpp

    HEADERS +=  \
        $$PWD/alsa.h
}

else {
    error(MIDI device support not implemented for this platform)
}
