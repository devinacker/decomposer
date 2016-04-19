/*
 * Non-platform specific MIDI device functions
 */

#include "MIDIoutput.h"
#include "MIDIdefs.h"

// ------------------------------------------------------------------------------------------------
void MIDIOutput::sendRPN(quint8 channel, quint16 param, quint16 value)
{
	this->send(EVENT_CONTROL(channel), CC_RPN_MSB,        MIDI_MSB(param));
	this->send(EVENT_CONTROL(channel), CC_RPN_LSB,        MIDI_LSB(param));

	this->send(EVENT_CONTROL(channel), CC_DATA_ENTRY_MSB, MIDI_MSB(value));
	this->send(EVENT_CONTROL(channel), CC_DATA_ENTRY_LSB, MIDI_LSB(value));

	this->send(EVENT_CONTROL(channel), CC_RPN_MSB,        MIDI_MSB(RPN_RESET));
//	this->send(EVENT_CONTROL(channel), CC_RPN_LSB,        MIDI_LSB(RPN_RESET));
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::sendNRPN(quint8 channel, quint16 param, quint16 value)
{
	this->send(EVENT_CONTROL(channel), CC_NRPN_MSB,       MIDI_MSB(param));
	this->send(EVENT_CONTROL(channel), CC_NRPN_LSB,       MIDI_LSB(param));

	this->send(EVENT_CONTROL(channel), CC_DATA_ENTRY_MSB, MIDI_MSB(value));
	this->send(EVENT_CONTROL(channel), CC_DATA_ENTRY_LSB, MIDI_LSB(value));

	this->send(EVENT_CONTROL(channel), CC_NRPN_MSB,       MIDI_MSB(RPN_RESET));
//	this->send(EVENT_CONTROL(channel), CC_NRPN_LSB,       MIDI_LSB(RPN_RESET));
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSendRPN(uint time, quint8 channel, quint16 param, quint16 value)
{
	this->streamSend(time, EVENT_CONTROL(channel), CC_RPN_MSB,        MIDI_MSB(param));
	this->streamSend(0,    EVENT_CONTROL(channel), CC_RPN_LSB,        MIDI_LSB(param));

	this->streamSend(0,    EVENT_CONTROL(channel), CC_DATA_ENTRY_MSB, MIDI_MSB(value));
	this->streamSend(0,    EVENT_CONTROL(channel), CC_DATA_ENTRY_LSB, MIDI_LSB(value));

	this->streamSend(0,    EVENT_CONTROL(channel), CC_RPN_MSB,        MIDI_MSB(RPN_RESET));
//	this->streamSend(0,    EVENT_CONTROL(channel), CC_RPN_LSB,        MIDI_LSB(RPN_RESET));
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSendNRPN(uint time, quint8 channel, quint16 param, quint16 value)
{
	this->streamSend(time, EVENT_CONTROL(channel), CC_NRPN_MSB,       MIDI_MSB(param));
	this->streamSend(0,    EVENT_CONTROL(channel), CC_NRPN_LSB,       MIDI_LSB(param));

	this->streamSend(0,    EVENT_CONTROL(channel), CC_DATA_ENTRY_MSB, MIDI_MSB(value));
	this->streamSend(0,    EVENT_CONTROL(channel), CC_DATA_ENTRY_LSB, MIDI_LSB(value));

	this->streamSend(0,    EVENT_CONTROL(channel), CC_NRPN_MSB,       MIDI_MSB(RPN_RESET));
//	this->streamSend(0,    EVENT_CONTROL(channel), CC_NRPN_LSB,       MIDI_LSB(RPN_RESET));
}
