/*
 * Platform interface for MIDI input devices.
 *
 * Call MIDIInput::enumerate() on program start to create instances for all available devices.
 * (The QApplication instance must exist, as it is used as the parent of the device instances.)
 * MIDIInput::getDevices() returns a list of pointers to all existing input device instances.
 * Device instances are destroyed when the application is closed.
 *
 * \todo: information about input signals
 *
 */

#ifndef MIDIINPUT_H
#define MIDIINPUT_H

#include "MIDIdevice.h"
#include <QList>

#define SYSEX_IN_BUF_SIZE 1024

class MIDIInput : public MIDIDevice
{
	Q_OBJECT

public:
	MIDIInput(uint id);
	~MIDIInput();

	static void enumerate();
	static QList<MIDIInput*> getDevices()
	{
		return MIDIInput::devices;
	}

	QString name() const;

public slots:
	bool open();
	bool close();
	bool reset();

	/* Begin listening for a single SysEx message from the input device.
	 * After the message is received, sysExRecorded() is emitted and recording stops.
	 * Calling this while recording is already enabled does nothing.
	 * \returns true if recording started successfully (or was started already), false otherwise
	 */
	bool recordSysEx();

signals:
	/* Emitted when any MIDI event occurs.
	 * \param time The time (in ms) when the event occurred after opening the device.
	 * TODO: additional signals for specific common events (maybe)
	 */
	void midiEvent(quint8 event, quint8 data1, quint8 data2, uint time);

	/* Emitted when a SysEx message is recorded.
	 * Call recordSysEx() first to enable SysEx recording. Only one message is recorded per call.
	 * \param data The complete SysEx message. May be empty if recording was cancelled or interrupted.
	 * \param time The time (in ms) when the message was received after opening the device.
	 */
	void sysExRecorded(QByteArray data, uint time);

private:
	struct InputInfo *m_info;
	QByteArray m_buffer;

	static QList<MIDIInput*> devices;
};

#endif // MIDIINPUT_H
