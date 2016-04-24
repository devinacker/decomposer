/*
 * MIDI output device implementation for ALSA (unfinished)
 */

#include "MIDIoutput.h"
#include "alsa.h"

// stream buffer size has to be less than 64kb (but not exactly 64kb)
#define STREAM_BUF_SIZE ((1 << 16) - sizeof(DWORD))

#define TEST(rc, ...) \
	do if (0 > rc) \
	{ \
		emit this->error(snd_strerror(rc)); \
		return __VA_ARGS__; \
	} while (0)

QList<MIDIOutput*> MIDIOutput::devices;

struct OutputInfo
{
	int client, port;

	snd_seq_port_info_t *portInfo = nullptr;
	snd_seq_port_subscribe_t *subsInfo = nullptr;

	snd_midi_event_t *encoder = nullptr;

	// TODO
	uint currHeader;
	bool streamPlaying;
};

// ------------------------------------------------------------------------------------------------
void MIDIOutput::enumerate()
{
	// only enumerate once
	if (!MIDIOutput::devices.isEmpty())
	{
		Q_ASSERT(!"attempted to enumerate MIDI input devices more than once");
		return;
	}

	QList<uint> ports = ALSA::enumerate(SND_SEQ_PORT_CAP_WRITE);
	for (uint &port : ports)
	{
		auto device = new MIDIOutput(port);
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
	int rc;
	m_info->client = id >> 8;
	m_info->port   = id & 0xFF;

	// allocate port info
	rc = snd_seq_port_info_malloc(&m_info->portInfo);
	TEST(rc);
	rc = snd_seq_get_any_port_info(ALSA::seq_handle, m_info->client, m_info->port, m_info->portInfo);
	TEST(rc);

	// allocate/init subscription info
	rc = snd_seq_port_subscribe_malloc(&m_info->subsInfo);
	TEST(rc);

	snd_seq_addr_t src, dest;
	dest.client = m_info->client;
	dest.port   = m_info->port;
	src.client  = ALSA::seq_client;
	src.port    = ALSA::seq_outport;

	snd_seq_port_subscribe_set_sender(m_info->subsInfo, &src);
	snd_seq_port_subscribe_set_dest(m_info->subsInfo, &dest);

	// allocate/init MIDI event encoder (for short events only)
	rc = snd_midi_event_new(3, &m_info->encoder);
	TEST(rc);
	snd_midi_event_reset_encode(m_info->encoder);
	snd_midi_event_no_status(m_info->encoder, 1);

	m_valid = true;
}

// ------------------------------------------------------------------------------------------------
MIDIOutput::~MIDIOutput()
{
	this->close();

	if (m_info->portInfo)
		snd_seq_port_info_free(m_info->portInfo);

	if (m_info->subsInfo)
		snd_seq_port_subscribe_free(m_info->subsInfo);

	if (m_info->encoder)
		snd_midi_event_free(m_info->encoder);

	delete m_info;
}

// ------------------------------------------------------------------------------------------------
QString MIDIOutput::name() const
{
	if (m_valid)
	{
		return QString(snd_seq_port_info_get_name(m_info->portInfo));
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

	int rc;

	rc = snd_seq_subscribe_port(ALSA::seq_handle, m_info->subsInfo);
	TEST(rc, false);

	emit this->opened();

	// TODO
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::close()
{
	if (!m_valid) return false;

	int rc;

	// TODO

	rc = snd_seq_unsubscribe_port(ALSA::seq_handle, m_info->subsInfo);
	TEST(rc, false);

	emit this->closed();

	// device was closed
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::reset()
{
	int rc = snd_seq_drop_output(ALSA::seq_handle);
	TEST(rc, false);

	// TODO: maybe turn off all notes

	return true;
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::send(quint8 data0, quint8 data1, quint8 data2)
{
	int rc;

	uchar data[3];
	data[0] = data0; data[1] = data1; data[2] = data2;

	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_source(&ev, ALSA::seq_outport);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	snd_midi_event_encode(m_info->encoder, data, 3, &ev);

	rc = snd_seq_event_output_direct(ALSA::seq_handle, &ev);
	TEST(rc);
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::send(const QByteArray &data)
{
	int rc;

	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_source(&ev, m_info->port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	snd_midi_event_encode(m_info->encoder, (const uchar*)data.constData(), data.size(), &ev);

	rc = snd_seq_event_output_direct(ALSA::seq_handle, &ev);
	TEST(rc);
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamOpen()
{
	// TODO

	m_info->streamPlaying = false;

	// prepare double buffers
	// TODO

	return this->open();
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSend(uint time, quint8 data0, quint8 data1, quint8 data2)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSend(uint time, const QByteArray &data)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSetTempo(uint time, double bpm)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamDelay(uint time)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void MIDIOutput::streamSetMarker(uint time, uint value)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamFlush()
{
	// if current MIDI stream buffer is ready for more data, dump the QByteArray into it
	// and switch to the other stream buffer

	// TODO

	if (!m_info->streamPlaying)
	{
		m_buffer.clear();
		return true;
	}
	else if (false /* TODO */)
	{
		// TODO
		m_buffer.clear();

		// TODO

		m_info->currHeader ^= 1;
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamStart(double bpm, uint ppq)
{
	ulong time = this->streamTime();

	// TODO
	emit this->error("not implemented yet");
	return false;

	// if stream is just now being started, prompt host application to fill both stream buffers
	if (time == 0)
	{
		// set default tempo and timebase
		// TODO

		m_info->currHeader = 0;

		for (int i = 0; i < 2; i++)
		{
			// TODO
		}
	}

	m_info->streamPlaying = true;
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamPause()
{
	// TODO
	emit this->error("not implemented yet");

	return false;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::streamStop()
{
	// TODO
	emit this->error("not implemented yet");

	m_info->streamPlaying = false;
	return true;
}

// ------------------------------------------------------------------------------------------------
ulong MIDIOutput::streamTime() const
{
	// TODO

	return 0;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::isStreamOpen() const
{
	// TODO
	return false;
}

// ------------------------------------------------------------------------------------------------
bool MIDIOutput::isStreamPlaying() const
{
	return m_info->streamPlaying;
}
