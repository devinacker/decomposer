#include "MIDIdefs.h"

using MIDI::StringMap;

static QString fromStringMap(const StringMap& map, uint num) {
	if (map.contains(num))
		return map.value(num);

	return "unknown";
}

static const StringMap controlNames =
{
	{0, "Bank select"},
	{1, "Modulation wheel"},
	{2, "Breath controller"},
	{4, "Foot pedal"},
	{5, "Portamento time"},
	{6, "Data entry (MSB)"},
	{7, "Volume"},
	{8, "Balance"},
	{10, "Pan position"},
	{11, "Expression"},
	{12, "Effect control 1"},
	{13, "Effect control 2"},
	{16, "Slider 1"},
	{17, "Slider 2"},
	{18, "Slider 3"},
	{19, "Slider 4"},

	{32, "Bank select (fine)"},
	{33, "Modulation wheel (fine)"},
	{34, "Breath controller (fine)"},
	{36, "Foot pedal (fine)"},
	{37, "Portamento time (fine)"},
	{38, "Data entry (LSB)"},
	{39, "Volume (fine)"},
	{40, "Balance (fine)"},
	{42, "Pan position (fine)"},
	{43, "Expression (fine)"},
	{44, "Effect control 1 (fine)"},
	{45, "Effect control 2 (fine)"},

	{64, "Hold pedal (on/off)"},
	{65, "Portamento (on/off)"},
	{66, "Sostenuto pedal (on/off)"},
	{67, "Soft pedal (on/off)"},
	{68, "Legato pedal (on/off)"},
	{69, "Hold 2 pedal (on/off)"},

	{70, "Sound variation"},
	{71, "Sound timbre"},
	{72, "Sound release time"},
	{73, "Sound attack time"},
	{74, "Sound brightness"},
	{75, "Sound control 6"},
	{76, "Sound control 7"},
	{77, "Sound control 8"},
	{78, "Sound control 9"},
	{79, "Sound control 10"},
	{80, "Button 1 (on/off)"},
	{81, "Button 1 (on/off)"},
	{82, "Button 1 (on/off)"},
	{83, "Button 1 (on/off)"},

	{91, "Reverb/delay level"},
	{92, "Tremolo level"},
	{93, "Chorus level"},
	{94, "Celeste level"},
	{95, "Phaser level"},

	{96, "Data button increment"},
	{97, "Data button decrement"},
	{98, "Non-registered param (LSB)"},
	{99, "Non-registered param (MSB)"},
	{100, "Registered param (LSB)"},
	{101, "Registered param (MSB)"},

	{120, "All sound off"},
	{121, "All controllers off"},
	{122, "Local keyboard (on/off)"},
	{123, "All notes off"},
	{124, "Omni mode off"},
	{125, "Omni mode on"},
	{126, "Mono operation"},
	{127, "Poly operation"},
};

QString MIDI::getControlName(uint num)
{
	return fromStringMap(controlNames, num);
}
