/*
 * Platform interface for MIDI input devices.
 *
 * Call MIDIOutput::enumerate() on program start to create instances for all available devices.
 * (The QApplication instance must exist, as it is used as the parent of the device instances.)
 * MIDIOutput::getDevices() returns a list of pointers to all existing input device instances.
 * Device instances are destroyed when the application is closed.
 *
 * \todo: information about output signals
 *
 */

#ifndef MIDIOUTPUT_H
#define MIDIOUTPUT_H

#include "MIDIdevice.h"
#include <QList>

class MIDIOutput : public MIDIDevice
{
	Q_OBJECT

public:
	MIDIOutput(uint id);
	~MIDIOutput();

	static void enumerate();
	static QList<MIDIOutput*> getDevices()
	{
		return MIDIOutput::devices;
	}

	QString name() const;

	/* \returns the stream's elapsed time in ticks (or 0 if not playing or unable to determine)
	 */
	ulong streamTime() const;

	/* \returns whether or not the device is open in stream mode
	 */
	bool isStreamOpen() const;

	/* \returns whether or not the stream is playing
	 */
	bool isStreamPlaying() const;

public slots:
	/* Open an output device for normal (non-streamed) output.
	 * If the device is already opened for streamed output, it is closed and re-opened first.
	 */
	bool open();
	bool close();
	bool reset();

	/* Sends a MIDI message to the output device.
	 * \param data0 The MIDI status byte.
	 * \param data1 The first MIDI data byte (optional).
	 * \param data2 The second MIDI data byte (optional).
	 */
	void send(quint8 data0, quint8 data1 = 0, quint8 data2 = 0);
	/* Sends a long MIDI message (e.g. SysEx) to the output device.
	 * \param data The MIDI message buffer.
	 */
	void send(const QByteArray &data);

	/* Send a RPN or NRPN to a specific channel. These are helper methods to send().
	 * \param channel The MIDI channel number (0-15).
	 * \param param The RPN or NRPN parameter number. See MIDIdefs.h for valid RPN numbers.
	 * \param value The parameter value to set.
	 */
	void sendRPN(quint8 channel, quint16 param, quint16 value);
	void sendNRPN(quint8 channel, quint16 param, quint16 value);

	/* Open the device in stream mode. If the device is already opened for normal output,
	 * it is closed and re-opened first. The stream can still be closed using close().
	 *
	 * In stream mode, the streamReady() signal is emitted to tell the host to call streamSend()
	 * and similar functions in order to buffer an amount of timestamped MIDI events.
	 * The amount (total length in ticks) is for the host to determine, but should be the same
	 * length each time (such as one quarter note's worth of MIDI events).
	 * streamDelay() can be used to pad out the buffer to the desired total number of ticks.
	 *
	 * The stream wrapper uses a double-buffered setup. After filling the buffer, the application
	 * must call streamFlush() to swap buffers and send the buffer that was just filled to the
	 * output device.
	 *
	 * \returns whether or not the stream was opened successfully
	 */
	bool streamOpen();

	/* Send a normal (short) MIDI message to the stream. This is analogous to send(), but with
	 * a timestamp that specifies the time (in ticks) that this message occurs relative to the
	 * previous message (like in a standard MIDI file).
	 * \param time the timestamp of the message (in ticks).
	 * \param data0 The MIDI status byte.
	 * \param data1 The first MIDI data byte (optional).
	 * \param data2 The second MIDI data byte (optional).
	 */
	void streamSend(uint time, quint8 data0, quint8 data1 = 0, quint8 data2 = 0);
	/* Send a long MIDI message to the stream. This is analogous to send(), but with
	 * a timestamp that specifies the time (in ticks) that this message occurs relative to the
	 * previous message (like in a standard MIDI file.
	 * \param time the timestamp of the message (in ticks).
	 * \param data The MIDI message buffer.
	 */
	void streamSend(uint time, const QByteArray &data);

	/* Send a RPN or NRPN to a specific channel. These are helper methods to streamSend().
	 * \param time the timestamp of the message (in ticks).
	 * \param channel The MIDI channel number (0-15).
	 * \param param The RPN or NRPN parameter number. See MIDIdefs.h for valid RPN numbers.
	 * \param value The parameter value to set.
	 */
	void streamSendRPN(uint time, quint8 channel, quint16 param, quint16 value);
	void streamSendNRPN(uint time, quint8 channel, quint16 param, quint16 value);

	/* Send a tempo change event to the stream. Out-of-range tempo values are ignored
	 * (the range of valid values is system-dependent).
	 * \param time the timestamp of the message (in ticks).
	 * \param bpm the new tempo in beats per minute
	 */
	void streamSetTempo(uint time, double bpm);

	/* Send a null event with a specified timestamp. Use this to pad out the event buffer
	 * to the desired length.
	 * \param time the timestamp of the message (in ticks).
	 */
	void streamDelay(uint time);

	/* Send a marker event with a specified timestamp. When this event is played by the system,
	 * the streamMarker signal is emitted with the value that was passed in.
	 * \param time the timestamp of the message (in ticks).
	 * \param value the marker value which will be emitted with streamMarker().
	 */
	void streamSetMarker(uint time, uint value);

	/* Flush the current buffer to the device and swap buffers.
	 * This must be called by the application after the buffer has been filled.
	 * Calling this while the stream is not playing will flush the buffer but will not
	 * send any actual stream data to the device (i.e. the buffer is discarded).
	 * \returns whether or not the buffer was flushed successfully.
	 */
	bool streamFlush();

	/* Start playback of the stream, or continue a paused stream.
	 * Once this is called, streamReady() will be emitted periodically to request buffer data
	 * from the application.
	 *
	 * \param bpm initial beats per minute
	 * \param ppq initial ticks per beat
	 *
	 * \returns whether or not the stream was started successfully
	 */
	bool streamStart(double bpm = 120.0, uint ppq = 96);
	/* Pause the stream. Playback can be resumed by calling streamStart() again.
	 * \returns whether or not the stream was paused successfully
	 */
	bool streamPause();
	/* Stop the stream. Playback can be restarted by calling streamStart().
	 * \returns whether or not the stream was stopped successfully
	 */
	bool streamStop();

signals:
	void streamReady();
	void streamMarker(uint);

private:
	struct OutputInfo *m_info;
	QByteArray m_buffer;

	static QList<MIDIOutput*> devices;
};

#endif // MIDIOUTPUT_H
