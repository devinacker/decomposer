#include "Instrument.h"

Instrument* Instrument::channelInstruments[16] = {0};

void Instrument::send(int time, MIDIOutput *out, quint8 data0, quint8 data1, quint8 data2)
{
	if (!out || channel > 15) return;

	// always play MIDI events on this instrument's channel
	if (data0 & 0x80)
		data0 = EVENT_REROUTE(data0, channel);

	checkInit(time, out);

	if (time >= 0)
		out->streamSend(time, data0, data1, data2);
	else
		out->send(data0, data1, data2);
}

// ------------------------------------------------------------------------------------------------
void Instrument::send(int time, MIDIOutput *out, const QByteArray &data)
{
	if (!out || channel > 15) return;

	checkInit(time, out);

	if (time >= 0)
		out->streamSend(time, data);
	else
		out->send(data);
}

// ------------------------------------------------------------------------------------------------
void Instrument::sendRPN(int time, MIDIOutput *out, quint16 param, quint16 value)
{
	if (!out || channel > 15) return;

	checkInit(time, out);

	if (time >= 0)
		out->streamSendRPN(time, channel, param, value);
	else
		out->sendRPN(channel, param, value);
}

// ------------------------------------------------------------------------------------------------
void Instrument::sendNRPN(int time, MIDIOutput *out, quint16 param, quint16 value)
{
	if (!out || channel > 15) return;

	checkInit(time, out);

	if (time >= 0)
		out->streamSendNRPN(time, channel, param, value);
	else
		out->sendNRPN(channel, param, value);
}

// ------------------------------------------------------------------------------------------------
void Instrument::init(int time, MIDIOutput *out)
{
	if (!out || channel > 15) return;

	channelInstruments[channel] = this;
	shouldReset = false;

	// controller reset
	this->send(time, out, EVENT_CONTROL(channel), CC_ALL_CONTROLLERS_OFF);

	if (time > 0) time = 0;

	// send program and bank
	this->send(time, out, EVENT_CONTROL(channel), CC_BANK_MSB, bank);
	this->send(time, out, EVENT_CONTROL(channel), CC_BANK_LSB, bankLSB);
	this->send(time, out, EVENT_PROGRAM(channel), program);

	// send pitch bend range
	this->sendRPN(time, out, RPN_PITCH_BEND_RANGE, bendRange * (1 << 7));

	// center pitch wheel
	this->pitch(time, out, 0x2000);

	// send transpose/finetune
	double intPart;
	double decPart = modf(transpose, &intPart);

	this->sendRPN(time, out, RPN_MASTER_TUNE, (64 + (int)(intPart)) << 7);
	this->sendRPN(time, out, RPN_MASTER_FINETUNE, (64 + (int)(64 * decPart)) << 7);

	// init macros
	for (int i = 0; i < macros.size(); i++)
	{
		const InstrumentMacro &m = macros.at(i);
		this->macro(time, out, i, 0, m.init);
	}
}

// ------------------------------------------------------------------------------------------------
void Instrument::noteOn(int time, MIDIOutput *out, quint8 note, quint8 velocity)
{
	if (velocity > 127)
		velocity = this->velocity;

	this->send(time, out, EVENT_NOTEON(channel), note, velocity);
}

// ------------------------------------------------------------------------------------------------
void Instrument::noteOff(int time, MIDIOutput *out, quint8 note, quint8 velocity)
{
	if (velocity > 127)
		velocity = this->velocity;

	this->send(time, out, EVENT_NOTEOFF(channel), note, velocity);
}

// ------------------------------------------------------------------------------------------------
void Instrument::pitch(int time, MIDIOutput *out, quint16 value)
{
	currentPitch = value;
	qint16 center = 81.92 * pitchCenter;

	if (value + center > 0x3FFF)
		value = 0x3FFF;
	else if (center < 0 && -center > value)
		value = 0;
	else
		value += center;

	this->send(time, out, EVENT_PITCH(channel), MIDI_LSB(value), MIDI_MSB(value));
}

// ------------------------------------------------------------------------------------------------
void Instrument::macro(int time, MIDIOutput *out, uint num, quint8 note, quint16 param)
{
	if (num >= (uint)macros.size()) return;

	InstrumentMacro &m = macros[num];

	m.current = param;

	switch (m.type)
	{
	case InstrumentMacro::MacroCC:
		this->send(time, out, EVENT_CONTROL(channel), m.num, m.current);
		break;

	case InstrumentMacro::MacroNRPN:
		this->sendNRPN(time, out, m.num, m.current);
		break;

	case InstrumentMacro::MacroSysEx:
	{
		QByteArray data = m.formatSysEx(channel, param, note);
		this->send(time, out, data);
	}
		break;
	}
}
