This is the beginning of what will be **Decomposer**, my tracker-style MIDI sequencer. It is designed to drive a single MIDI output device (and possibly VSTi and other synth plugin standards eventually).

Currently this mostly consists of my own MIDI input/output/streaming interface, which exposes MIDI input and output devices as QObjects and provides a system for buffering timestamped MIDI events for the actual sequencer (and will eventually be extended to allow writing to a standard MIDI file). Currently only a Windows implementation exists, but I plan to implement it for ALSA and CoreMIDI eventually. Look in the *src/devices* subdirectory if you want to see how this works.

On the GUI side, existing elements are the devices page (which lets you select an input and output device and logs most incoming MIDI traffic) and the instruments page, for editing up to 64 MIDI "instruments". An instrument in this context refers to a channel/bank/program combo, plus several relevant parameters like transpose/pitch-bend range RPNs, as well as (soon) macros.

Macros (not yet implemented) are a set of MIDI CC, NRPN, and parameterized SysEx messages that the user can configure for each instrument, set an initial value for, and then change within the actual sequence in the same manner as traditional tracker effect commands. Device macros will probably be implemented as well for setting global effect parameters, etc.

The sequencer will be started next, and will have individual pattern orders for each track (think FamiTracker, GoatTracker, etc.)

This is a Qt 5 and C++11 project. As usual, it's released under the MIT license, but aside from the MIDI interface there's nothing here worth borrowing or stealing yet.