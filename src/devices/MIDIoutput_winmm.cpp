/*
 * MIDI output device implementation for WinMM
 */

#include "MIDIoutput.h"
#include <Windows.h>

// stream buffer size has to be less than 64kb (but not exactly 64kb)
#define STREAM_BUF_SIZE ((1 << 16) - sizeof(DWORD))

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

QList<MIDIOutput*> MIDIOutput::devices;

struct OutputInfo
{
	HMIDIOUT handle;
	MIDIOUTCAPS caps;

	/* Stream-related info */
	HMIDISTRM stream;
	MIDIHDR header[2];
	uint currHeader;
	bool streamPlaying;
};

// ------------------------------------------------------------------------------------------------
static void emitError(MIDIOutput *self, MMRESULT error)
{
	TCHAR errorText[MAXERRORLENGTH];

	if (MMSYSERR_NOERROR == midiOutGetErrorText(error, errorText, MAXERRORLENGTH))
	{
		emit self->error(QSTR(errorText));
	}
}

// ------------------------------------------------------------------------------------------------
static void CALLBACK midiCallback(HMIDIOUT handle, UINT msg, DWORD_PTR instance,
						   DWORD_PTR dw1, DWORD_PTR dw2)
{
	Q_UNUSED(dw2);

	if (!instance)
	{
		Q_ASSERT(!"MIDI input callback called with null instance");
		// TODO: close handle here?
		return;
	}

	auto self = reinterpret_cast<MIDIOutput*>(instance);

	if (msg == MOM_DONE)
	{
		auto header = reinterpret_cast<MIDIHDR*>(dw1);

		if (header->dwUser)
		{
			// TODO: is this call safe here?
			midiOutUnprepareHeader(handle, header, sizeof(MIDIHDR));

			delete[] header->lpData;
			delete header;
		}
		else
		{
			// notify the host application to populate the next buffer
			emit self->streamReady();
		}
	}
	else if (msg == MOM_POSITIONCB)
	{
		auto header = reinterpret_cast<MIDIHDR*>(dw1);
		auto event  = reinterpret_cast<MIDIEVENT*>(header->lpData + header->dwOffset);

		uint marker = MEVT_EVENTPARM(event->dwEvent);
		emit self->streamMarker(marker);
	}
	else if (msg == MOM_OPEN)
	{
		emit self->opened();
	}
	else if (msg == MOM_CLOSE)
	{
		emit self->closed();
	}
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::enumerate()
{
	// only enumerate once
	if (!MIDIOutput::devices.isEmpty())
	{
		Q_ASSERT(!"attempted to enumerate MIDI input devices more than once");
		return;
	}

	UINT numDevs = midiOutGetNumDevs();
	for (unsigned i = 0; i < numDevs; i++)
	{
		auto device = new MIDIOutput(i);
		if (device->isValid())
		{
			MIDIOutput::devices.append(device);
		}
		else
		{
			device->deleteLater();
		}
	}
}

// ------------------------------------------------------------------------------------------------
MIDIOutput::MIDIOutput(uint id)
	: MIDIDevice(id)
	, m_info(new OutputInfo())
{
	MMRESULT result = midiOutGetDevCaps(m_deviceID, &m_info->caps, sizeof(MIDIOUTCAPS));
	TEST(result);

	m_valid = true;
}

// ------------------------------------------------------------------------------------------------
MIDIOutput::~MIDIOutput()
{
	this->close();
	delete m_info;
}

// ------------------------------------------------------------------------------------------------
QString MIDIOutput::name() const
{
	if (m_valid)
	{
		return QSTR(m_info->caps.szPname);
	}

	return QString();
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::open()
{
	if (!m_valid)
	{
		emit error(tr("tried to open an invalid device"));
		return false;
	}

	// close the device if it's already open (as a normal or streamed output)
	if (!this->close())
		return false;

	MMRESULT result = midiOutOpen(&m_info->handle, m_deviceID,
								 (DWORD_PTR)midiCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	TEST(result, false);

	// device was opened successfully
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::close()
{
	if (!m_valid) return false;

	// closing a stream output?
	if (m_info->stream)
	{
		this->streamStop();
		this->reset();

		// unprepare double buffers
		for (int i = 0; i < 2; i++)
		{
			m_info->header[i].dwBufferLength = 0;
			delete[] m_info->header[i].lpData;
			m_info->header[i].lpData = nullptr;

			MMRESULT result = midiOutUnprepareHeader((HMIDIOUT)m_info->stream, &m_info->header[i], sizeof(MIDIHDR));
			TEST(result, false);
		}

		MMRESULT result = midiStreamClose(m_info->stream);
		TEST(result, false);

		m_info->streamPlaying = false;
		m_info->stream = 0;
	}
	// closing a non-stream output
	else if (m_info->handle)
	{
		this->reset();

		MMRESULT result = midiOutClose(m_info->handle);
		TEST(result, false);

		m_info->handle = 0;
	}

	// device was closed
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::reset()
{
	HMIDIOUT handle = m_info->stream ? (HMIDIOUT)m_info->stream : m_info->handle;

	MMRESULT result = midiOutReset(handle);
	TEST(result, false);

	return true;
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::send(quint8 data0, quint8 data1, quint8 data2)
{
	HMIDIOUT handle = m_info->stream ? (HMIDIOUT)m_info->stream : m_info->handle;

	MMRESULT result = midiOutShortMsg(handle, data0 | (data1 << 8) | (data2 << 16));
	TEST(result);
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::send(const QByteArray &data)
{
	HMIDIOUT handle = m_info->stream ? (HMIDIOUT)m_info->stream : m_info->handle;

	// Create a new buffer and header which will be disposed by the callback function
	MIDIHDR *header = new MIDIHDR;
	header->dwFlags = 0;
	header->dwBufferLength = data.size();
	header->lpData = new CHAR[header->dwBufferLength];
	memcpy(header->lpData, data.constData(), header->dwBufferLength);

	// tell callback to delete this when done
	header->dwUser = 1;

	bool ok = true;

	MMRESULT result = midiOutPrepareHeader(handle, header, sizeof(MIDIHDR));
	if (MMSYSERR_NOERROR != result)
	{
		emitError(this, result);
		ok = false;
	}

	result = midiOutLongMsg(handle, header, sizeof(MIDIHDR));
	if (MMSYSERR_NOERROR != result)
	{
		emitError(this, result);
		ok = false;
	}

	if (!ok)
	{
		midiOutUnprepareHeader(handle, header, sizeof(MIDIHDR));
		delete[] header->lpData;
		delete header;
	}
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamOpen()
{
	// close the device if it's already open (as a normal or streamed output)
	if (!this->close())
		return false;

	uint id = m_deviceID;

	MMRESULT result = midiStreamOpen(&m_info->stream, &id, 1,
									 (DWORD_PTR)midiCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	TEST(result, false);

	m_info->streamPlaying = false;

	// prepare double buffers
	for (int i = 0; i < 2; i++)
	{
		m_info->header[i].dwBufferLength = STREAM_BUF_SIZE;
		m_info->header[i].lpData = new CHAR[STREAM_BUF_SIZE];
		m_info->header[i].dwFlags = 0;
		// don't delete when received by the callback
		m_info->header[i].dwUser = 0;

		result = midiOutPrepareHeader((HMIDIOUT)m_info->stream, &m_info->header[i], sizeof(MIDIHDR));
		TEST(result, false);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
static void addMIDIEvent(QByteArray &data, MIDIEVENT &event)
{
	data.append((const char*)&event.dwDeltaTime, sizeof(DWORD));
	data.append((const char*)&event.dwStreamID , sizeof(DWORD));
	data.append((const char*)&event.dwEvent    , sizeof(DWORD));
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSend(uint time, quint8 data0, quint8 data1, quint8 data2)
{
	MIDIEVENT event;

	event.dwDeltaTime = time;
	event.dwStreamID = 0;
	event.dwEvent = data0 | (data1 << 8) | (data2 << 16) | (MEVT_SHORTMSG << 24);

	addMIDIEvent(m_buffer, event);
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSend(uint time, const QByteArray &data)
{
	// ignore messages that are too long
	if (data.size() >= (1 << 16)) return;

	MIDIEVENT event;

	event.dwDeltaTime = time;
	event.dwStreamID = 0;
	event.dwEvent = data.size() | (MEVT_LONGMSG << 24);

	addMIDIEvent(m_buffer, event);

	m_buffer.append(data);
	// make sure this is padded to DWORD size
	// TODO: test
	for (int i = data.size(); i % sizeof(DWORD); i++)
	{
		m_buffer.append('\0');
	}
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSetTempo(uint time, double bpm)
{
	MIDIEVENT event;

	event.dwDeltaTime = time;
	event.dwStreamID = 0;
	uint tempo = (60 * 1000000) / bpm;
	// ignore tempos that are too low
	if (tempo >= (1 << 24)) return;

	event.dwEvent = tempo | (MEVT_TEMPO << 24);

	addMIDIEvent(m_buffer, event);
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamDelay(uint time)
{
	MIDIEVENT event;

	event.dwDeltaTime = time;
	event.dwStreamID = 0;
	event.dwEvent = MEVT_NOP << 24;

	addMIDIEvent(m_buffer, event);
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSetMarker(uint time, uint value)
{
	// ignore values that are too high
	if (value >= (1 << 24)) return;

	MIDIEVENT event;

	event.dwDeltaTime = time;
	event.dwStreamID = 0;
	event.dwEvent = value | (MEVT_NOP << 24) | MEVT_F_CALLBACK;

	addMIDIEvent(m_buffer, event);
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamFlush()
{
	// if current MIDI stream buffer is ready for more data, dump the QByteArray into it
	// and switch to the other stream buffer

	auto &header = m_info->header[m_info->currHeader];

	if (!m_info->streamPlaying)
	{
		m_buffer.clear();
		return true;
	}
	else if (!(header.dwFlags & MHDR_INQUEUE) && header.lpData)
	{
		header.dwBytesRecorded = qMin(STREAM_BUF_SIZE, (unsigned)m_buffer.size());
		memcpy(header.lpData, m_buffer.constData(), header.dwBytesRecorded);
		m_buffer.clear();

		MMRESULT result = midiStreamOut(m_info->stream, &header, sizeof(MIDIHDR));
		TEST(result, false);

		m_info->currHeader ^= 1;
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamStart(double bpm, uint ppq)
{
	ulong time = this->streamTime();

	MMRESULT result = midiStreamRestart(m_info->stream);
	TEST(result, false);

	// if stream is just now being started, prompt host application to fill both stream buffers
	if (time == 0)
	{
		// set default tempo and timebase
		MIDIPROPTEMPO tempo;
		tempo.cbStruct = sizeof(MIDIPROPTEMPO);
		tempo.dwTempo = (60 * 1000000) / bpm;
		result = midiStreamProperty(m_info->stream, (LPBYTE)&tempo, MIDIPROP_SET | MIDIPROP_TEMPO);
//		TEST(result, false);

		MIDIPROPTIMEDIV timediv;
		timediv.cbStruct = sizeof(MIDIPROPTIMEDIV);
		timediv.dwTimeDiv = ppq;
		result = midiStreamProperty(m_info->stream, (LPBYTE)&timediv, MIDIPROP_SET | MIDIPROP_TIMEDIV);
//		TEST(result, false);

		m_info->currHeader = 0;

		for (int i = 0; i < 2; i++)
		{
			m_info->header[i].dwBytesRecorded = 0;
			midiStreamOut(m_info->stream, &m_info->header[i], sizeof(MIDIHDR));
		}
	}

	m_info->streamPlaying = true;
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamPause()
{
	MMRESULT result = midiStreamPause(m_info->stream);
	TEST(result, false);

	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamStop()
{
	MMRESULT result = midiStreamStop(m_info->stream);
	TEST(result, false);

	m_info->streamPlaying = false;
	return true;
}

// ------------------------------------------------------------------------------------------------
ulong MIDIOutput::streamTime() const
{
	MMTIME mmt;
	mmt.wType = TIME_TICKS;
	mmt.u.ticks = 0;

	midiStreamPosition(m_info->stream, &mmt, sizeof(MMTIME));

	if (mmt.wType == TIME_TICKS)
	{
		return mmt.u.ticks;
	}

	// should always be in ticks for MIDI streams...
	return 0;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::isStreamOpen() const
{
	return m_info->stream != 0;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::isStreamPlaying() const
{
	return m_info->streamPlaying;
}
