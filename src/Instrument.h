#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <cmath>
#include <QString>

#include "devices/MIDIdefs.h"
#include "devices/MIDIoutput.h"

struct InstrumentMacro
{
	enum Type {
		MacroCC,
		MacroNRPN,
		MacroSysEx
	} type;

	quint16 num = 0;
	quint16 current = 0;
	quint16 init = 0;
	QString se;

	friend struct Instrument;

private:
	QByteArray formatSysEx(uint8_t channel, uint8_t value, uint8_t note = 0)
	{
		// TODO
		QString result = se;
		result	// %c = channel (one digit)
				.replace("%c", QString::asprintf("%X", channel))
				// %C = channel (two digits)
				.replace("%C", QString::asprintf("%02X", channel))
				// %n = note (two digits)
				.replace("%n", QString::asprintf("%02X", note))
				// %v = value (two digits)
				.replace("%v", QString::asprintf("%02X", value));

		// TODO: convert hex string to bytearray

		return QByteArray();
	}
};

struct Instrument
{
private:
	// map instruments to MIDI channels
	static Instrument* channelInstruments[16];

	void checkInit(int& time, MIDIOutput *out)
	{
		if (shouldReset || channelInstruments[channel] != this)
		{
			this->init(time, out);
			if (time > 0) time = 0;
		}
	}

public:

	bool shouldReset = true;

	QString name = QObject::tr("New instrument");
	quint8 channel = 0;

	quint8 velocity = 127;

	quint8 program = 0;
	quint8 bank = 0;
	quint8 bankLSB = 0;

	double transpose = 0.0;
	double bendRange = 2.0;
	double pitchCenter = 0.0;
	quint16 currentPitch = 0x2000;

	QList<InstrumentMacro> macros;

	void send(int time, MIDIOutput *out, quint8 data0, quint8 data1 = 0, quint8 data2 = 0);
	void send(MIDIOutput *out, quint8 data0, quint8 data1 = 0, quint8 data2 = 0)
	{
		send(-1, out, data0, data1, data2);
	}

	void send(int time, MIDIOutput *out, const QByteArray &data);
	void send(MIDIOutput *out, const QByteArray &data)
	{
		send(-1, out, data);
	}

	void sendRPN(int time, MIDIOutput *out, quint16 param, quint16 value);
	void sendRPN(MIDIOutput *out, quint16 param, quint16 value)
	{
		sendRPN(-1, out, param, value);
	}

	void sendNRPN(int time, MIDIOutput *out, quint16 param, quint16 value);
	void sendNRPN(MIDIOutput *out, quint16 param, quint16 value)
	{
		sendNRPN(-1, out, param, value);
	}

	void init(int time, MIDIOutput *out);
	void init(MIDIOutput *out)
	{
		init(-1, out);
	}

	void noteOn(int time, MIDIOutput *out, quint8 note, quint8 velocity = (quint8)-1u);
	void noteOn(MIDIOutput *out, quint8 note, quint8 velocity = (quint8)-1u)
	{
		noteOn(-1, out, note, velocity);
	}

	void noteOff(int time, MIDIOutput *out, quint8 note, quint8 velocity = (quint8)-1u);
	void noteOff(MIDIOutput *out, quint8 note, quint8 velocity = (quint8)-1u)
	{
		noteOff(-1, out, note, velocity);
	}

	void pitch(int time, MIDIOutput *out, quint16 value);
	void pitch(MIDIOutput *out, quint16 value)
	{
		pitch(-1, out, value);
	}

	void macro(int time, MIDIOutput *out, uint num, quint8 note, quint16 param);
	void macro(MIDIOutput *out, uint num, quint8 note, quint16 param)
	{
		macro(-1, out, num, note, param);
	}
};

#endif // INSTRUMENT_H
