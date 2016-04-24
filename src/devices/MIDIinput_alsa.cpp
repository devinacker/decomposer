/*
 * MIDI input device implementation for ALSA (unfinished)
 */

#include "MIDIinput.h"
#include "alsa.h"

#define TEST(rc, ...) \
	do if (0 > rc) \
	{ \
		emit this->error(snd_strerror(rc)); \
		return __VA_ARGS__; \
	} while (0)

QList<MIDIInput*> MIDIInput::devices;

struct InputInfo
{
	int client, port;

	snd_seq_port_info_t *portInfo = nullptr;
	snd_seq_port_subscribe_t *subsInfo = nullptr;

	InputThread *thread = nullptr;
};

// ------------------------------------------------------------------------------------------------
void MIDIInput::enumerate()
{
	// only enumerate once
	if (!MIDIInput::devices.isEmpty())
	{
		Q_ASSERT(!"attempted to enumerate MIDI input devices more than once");
		return;
	}

	QList<uint> ports = ALSA::enumerate(SND_SEQ_PORT_CAP_SUBS_READ);
	for (uint &port : ports)
	{
		auto device = new MIDIInput(port);
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
	src.client  = m_info->client;
	src.port    = m_info->port;
	dest.client = ALSA::seq_client;
	dest.port   = ALSA::seq_inport;

	snd_seq_port_subscribe_set_sender(m_info->subsInfo, &src);
	snd_seq_port_subscribe_set_dest(m_info->subsInfo, &dest);

	m_info->thread = new InputThread(this);
	connect(m_info->thread, SIGNAL(midiEvent(quint8,quint8,quint8,uint)),
			this, SIGNAL(midiEvent(quint8,quint8,quint8,uint)));

	m_valid = true;
}

// ------------------------------------------------------------------------------------------------
MIDIInput::~MIDIInput()
{
	this->close();

	if (m_info->portInfo)
		snd_seq_port_info_free(m_info->portInfo);

	if (m_info->subsInfo)
		snd_seq_port_subscribe_free(m_info->subsInfo);

	delete m_info;
}

// ------------------------------------------------------------------------------------------------
QString MIDIInput::name() const
{
	if (m_valid)
	{
		return QString(snd_seq_port_info_get_name(m_info->portInfo));
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

	int rc;

	rc = snd_seq_subscribe_port(ALSA::seq_handle, m_info->subsInfo);
	TEST(rc, false);

	emit this->opened();

	m_info->thread->start();

	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::close()
{
	if (!m_valid) return false;

	int rc;

	// do nothing if device was closed already
	// TODO

	this->reset();

	rc = snd_seq_unsubscribe_port(ALSA::seq_handle, m_info->subsInfo);
	TEST(rc, false);

	if (m_info->thread->isRunning())
	{
		m_info->thread->requestInterruption();
		m_info->thread->wait();
		emit this->closed();
	}

	// device was closed successfully
	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::reset()
{
	int rc = snd_seq_drop_input(ALSA::seq_handle);
	TEST(rc, false);

	return true;
}

// ------------------------------------------------------------------------------------------------
bool MIDIInput::recordSysEx()
{
	if (!m_valid) return false;

	emit this->error("not implemented yet");
	// TODO

	return false;
}
