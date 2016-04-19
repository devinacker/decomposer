/*
 * MIDI input device implementation for WinMM
 */

#include "MIDIinput.h"
#include <Windows.h>

#ifdef UNICODE
#define QSTR(...) QString::fromWCharArray(__VA_ARGS__)
#else
#define QSTR(...) QString::fromLocal8Bit(__VA_ARGS__)
#endif // UNICODE

#define TEST(rc, ...) \
	do if (MMSYSERR_NOERROR != rc) \
	{ \
		emitError(this, rc); \
		return __VA_ARGS__; \
	} while (0)

QList<MIDIInput*> MIDIInput::devices;

struct InputInfo
{
	HMIDIIN handle;
	MIDIINCAPS caps;
	MIDIHDR header;
};

// ------------------------------------------------------------------------------------------------
static void emitError(MIDIInput *self, MMRESULT error)
{
	TCHAR errorText[MAXERRORLENGTH];

	if (MMSYSERR_NOERROR == midiInGetErrorText(error, errorText, MAXERRORLENGTH))
	{
		emit self->error(QSTR(errorText));
	}
}

// ------------------------------------------------------------------------------------------------
static void CALLBACK midiCallback(HMIDIIN handle, UINT msg, DWORD_PTR instance,
						   DWORD_PTR dw1, DWORD_PTR dw2)
{
	Q_UNUSED(handle);

	if (!instance)
	{
		Q_ASSERT(!"MIDI input callback called with null instance");
		// TODO: close handle here?
		return;
	}

	auto self = reinterpret_cast<MIDIInput*>(instance);

	if (msg == MIM_DATA)
	{
		quint8 event = dw1 >> 0;
		quint8 data1 = (dw1 >> 8)  & 0x7F;
		quint8 data2 = (dw1 >> 16) & 0x7F;

		emit self->midiEvent(event, data1, data2, dw2);
	}
	else if (msg == MIM_LONGDATA)
	{
		// handle sysex
		auto header = reinterpret_cast<MIDIHDR*>(dw1);
		auto buffer = reinterpret_cast<QByteArray*>(header->dwUser);

		buffer->append(header->lpData, header->dwBytesRecorded);

		if (header->dwFlags & MHDR_DONE)
		{
			emit self->sysExRecorded(*buffer, dw2);
			buffer->clear();
		}
	}
	else if (msg == MIM_OPEN)
	{
		emit self->opened();
	}
	else if (msg == MIM_CLOSE)
	{
		emit self->closed();
	}
}

// ------------------------------------------------------------------------------------------------
void MIDIInput::enumerate()
{
	// only enumerate once
	if (!MIDIInput::devices.isEmpty())
	{
		Q_ASSERT(!"attempted to enumerate MIDI input devices more than once");
		return;
	}

	UINT numDevs = midiInGetNumDevs();
	for (unsigned i = 0; i < numDevs; i++)
	{
		auto device = new MIDIInput(i);
		if (device->isValid())
		{
			MIDIInput::devices.append(device);
		}
		else
		{
			device->deleteLater();
		}
	}
}

// ------------------------------------------------------------------------------------------------
MIDIInput::MIDIInput(uint id)
	: MIDIDevice(id)
	, m_info(new InputInfo())
{
	MMRESULT result = midiInGetDevCaps(m_deviceID, &m_info->caps, sizeof(MIDIINCAPS));
	TEST(result);

	m_valid = true;
}

// ------------------------------------------------------------------------------------------------
MIDIInput::~MIDIInput()
{
	this->close();
	delete m_info;
}

// ------------------------------------------------------------------------------------------------
QString MIDIInput::name() const
{
	if (m_valid)
	{
		return QSTR(m_info->caps.szPname);
	}

	return QString();
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::open()
{
	if (!m_valid)
	{
		emit error(tr("tried to open an invalid device"));
		return false;
	}
	Q_ASSERT(!m_info->handle);

	MMRESULT result = midiInOpen(&m_info->handle, m_deviceID,
								 (DWORD_PTR)midiCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	TEST(result, false);

	// initialize sysex buffer
	m_info->header.lpData = new CHAR[SYSEX_IN_BUF_SIZE];
	m_info->header.dwBufferLength = SYSEX_IN_BUF_SIZE;
	m_info->header.dwFlags = 0;
	m_info->header.dwUser = (DWORD_PTR)&m_buffer;
	result = midiInPrepareHeader(m_info->handle, &m_info->header, sizeof(MIDIHDR));
	if (MMSYSERR_NOERROR != result)
	{
		delete[] m_info->header.lpData;
		m_info->header.lpData = nullptr;
		m_info->header.dwBufferLength = 0;

		emitError(this, result);
		return false;
	}

	// device was opened successfully
	midiInStart(m_info->handle);
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::close()
{
	if (!m_valid) return false;

	// do nothing if device was closed already
	if (!m_info->handle) return true;

	this->reset();

	// deinitialize sysex buffer
	MMRESULT result = midiInUnprepareHeader(m_info->handle, &m_info->header, sizeof(MIDIHDR));
	TEST(result, false);

	delete[] m_info->header.lpData;
	m_info->header.lpData = nullptr;
	m_info->header.dwBufferLength = 0;

	result = midiInClose(m_info->handle);
	TEST(result, false);

	// device was closed successfully
	m_info->handle = 0;
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::reset()
{
	MMRESULT result = midiInReset(m_info->handle);
	TEST(result, false);

	midiInStart(m_info->handle);
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::recordSysEx()
{
	if (!m_valid || !m_info->handle) return false;

	// use sysex buffer
	MMRESULT result = midiInAddBuffer(m_info->handle, &m_info->header, sizeof(MIDIHDR));
	if (MMSYSERR_NOERROR != result &&
		MIDIERR_STILLPLAYING != result)
	{
		emitError(this, result);
		return false;
	}

	return true;
}
