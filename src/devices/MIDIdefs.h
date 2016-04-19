#ifndef MIDIDEFS_H
#define MIDIDEFS_H

#define MIDI_LSB(n)         (n & 0x7F)
#define MIDI_MSB(n)         ((n >> 7) & 0x7F)
#define MIDI_WORD(m, l)     ((m << 7) | (l & 0x7F))

/*
 * MIDI event/status bytes
 */
#define EVENT_NOTEOFF(c)    (0x80 | c)
#define EVENT_NOTEON(c)     (0x90 | c)
#define EVENT_AFTERTOUCH(c) (0xA0 | c)
#define EVENT_CONTROL(c)    (0xB0 | c)
#define EVENT_PROGRAM(c)    (0xC0 | c)
#define EVENT_PRESSURE(c)   (0xD0 | c)
#define EVENT_PITCH(c)      (0xE0 | c)

#define EVENT_SYSEX_START   0xF0
#define EVENT_SYSEX_END     0xF7

#define EVENT_MTC_QTRFRAME  0xF1
#define EVENT_SONG_POSITION 0xF2
#define EVENT_SONG_SELECT   0xF3
#define EVENT_TUNE_REQUEST  0xF6

#define EVENT_MIDI_CLOCK    0xF8
#define EVENT_MIDI_TICK     0xF9
#define EVENT_MIDI_START    0xFA
#define EVENT_MIDI_CONTINUE 0xFB
#define EVENT_MIDI_STOP     0xFC
#define EVENT_ACTIVE_SENSE  0xFE
#define EVENT_RESET         0xFF

/*
 * Reroute a MIDI event to a different channel
 */
#define EVENT_REROUTE(e, c) ((e < 0xF0) ? (e & 0xF0) | c : e)

/*
 * MIDI control values
 */
#define CC_BANK_MSB          0
#define CC_BANK_LSB          32
#define CC_MODWHEEL_MSB      1
#define CC_MODWHEEL_LSB      33
#define CC_BREATH_CTRL_MSB   2
#define CC_BREATH_CTRL_LSB   34
#define CC_FOOT_PEDAL_MSB    4
#define CC_FOOT_PEDAL_LSB    36
#define CC_PORTAMENTO_TIME_MSB 5
#define CC_PORTAMENTO_TIME_LSB 37
#define CC_DATA_ENTRY_MSB      6
#define CC_DATA_ENTRY_LSB      38
#define CC_VOLUME_MSB          7
#define CC_VOLUME_LSB          39
#define CC_BALANCE_MSB         8
#define CC_BALANCE_LSB         40
#define CC_PAN_MSB             10
#define CC_PAN_LSB             42
#define CC_EXPRESSION_MSB      11
#define CC_EXPRESSION_LSB      43
#define CC_EFFECT1_MSB         12
#define CC_EFFECT1_LSB         44
#define CC_EFFECT2_MSB         13
#define CC_EFFECT2_LSB         45
#define CC_SLIDER1             16
#define CC_SLIDER2             17
#define CC_SLIDER3             18
#define CC_SLIDER4             19
#define CC_HOLD_PEDAL          64
#define CC_PORTAMENTO          65
#define CC_SOSTENUTO_PEDAL     66
#define CC_SOFT_PEDAL          67
#define CC_LEGATO_PEDAL        68
#define CC_HOLD2_PEDAL         69
#define CC_SOUND_VARIATION     70
#define CC_SOUND_TIMBRE        71
#define CC_SOUND_RELEASE       72
#define CC_SOUND_ATTACK        73
#define CC_SOUND_BRIGHTNESS    74
#define CC_SOUND_CONTROL6      75
#define CC_SOUND_CONTROL7      76
#define CC_SOUND_CONTROL8      77
#define CC_SOUND_CONTROL9      78
#define CC_SOUND_CONTROL10     79
#define CC_BUTTON1             80
#define CC_BUTTON2             81
#define CC_BUTTON3             82
#define CC_BUTTON4             83
#define CC_EFFECTS_LEVEL       91
#define CC_TREMOLO_LEVEL       92
#define CC_CHORUS_LEVEL        93
#define CC_CELESTE_LEVEL       94
#define CC_PHASER_LEVEL        95
#define CC_DATA_BUTTON_INC     96
#define CC_DATA_BUTTON_DEC     97
#define CC_NRPN_LSB            98
#define CC_NRPN_MSB            99
#define CC_RPN_LSB             100
#define CC_RPN_MSB             101
#define CC_ALL_SOUND_OFF       120
#define CC_ALL_CONTROLLERS_OFF 121
#define CC_LOCAL_KEYBOARD      122
#define CC_ALL_NOTES_OFF       123
#define CC_OMNI_OFF            124
#define CC_OMNI_ON             125
#define CC_MONO_MODE           126
#define CC_POLY_MODE           127

/*
 * Registered parameter number (RPN) values
 */

#define RPN_PITCH_BEND_RANGE 0
#define RPN_MASTER_FINETUNE  1
#define RPN_MASTER_TUNE      2
#define RPN_TUNING_PROGRAM   3
#define RPN_TUNING_BANK      4
#define RPN_MOD_RANGE        5
#define RPN_RESET            0x3FFF

#include <QString>
#include <QMap>

namespace MIDI
{
	typedef QMap<uint, QString> StringMap;

	QString getControlName(uint);
}

#endif // MIDIDEFS_H
