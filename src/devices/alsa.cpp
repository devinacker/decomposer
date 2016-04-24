#include "alsa.h"
#include "MIDIdefs.h"

#include <QElapsedTimer>

using namespace ALSA;

snd_seq_t* ALSA::seq_handle = nullptr;
int ALSA::seq_client = -1;
int ALSA::seq_inport = -1;
int ALSA::seq_outport = -1;

static void deinit()
{
	snd_seq_delete_simple_port(seq_handle, seq_inport);
	seq_inport = -1;

	snd_seq_delete_simple_port(seq_handle, seq_outport);
	seq_outport = -1;

	snd_seq_close(seq_handle);
	seq_client = -1;
	seq_handle = nullptr;
}

int ALSA::init()
{
	// already initialized?
	if (seq_client >= 0)
		return 0;

	int rc;

	rc = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (rc) return rc;

	rc = snd_seq_set_client_name(seq_handle, "Decomposer MIDI Interface");
	if (rc) return rc;

	seq_inport = snd_seq_create_simple_port(seq_handle, "Decomposer (in)",
										  SND_SEQ_PORT_CAP_WRITE,
										  SND_SEQ_PORT_TYPE_APPLICATION);
	if (seq_inport < 0) return seq_inport;

	seq_outport = snd_seq_create_simple_port(seq_handle, "Decomposer (out)",
										  SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
										  SND_SEQ_PORT_TYPE_APPLICATION);
	if (seq_outport < 0) return seq_outport;

	seq_client = snd_seq_client_id(seq_handle);
	if (seq_client < 0) return seq_client;

	atexit(deinit);
	return 0;
}

QList<uint> ALSA::enumerate(int caps)
{
	QList<uint> ports;

	snd_seq_client_info_t *clientInfo;
	snd_seq_port_info_t *portInfo;

	snd_seq_client_info_alloca(&clientInfo);
	snd_seq_port_info_alloca(&portInfo);

	init();

	// iterate over all sequencer clients
	snd_seq_client_info_set_client(clientInfo, -1);
	while (0 == snd_seq_query_next_client(seq_handle, clientInfo))
	{
		int client = snd_seq_client_info_get_client(clientInfo);
		// ignore system ports and our own ports
		if (client == SND_SEQ_CLIENT_SYSTEM || client == seq_client)
			continue;

		// iterate over all of this client's ports
		snd_seq_port_info_set_port(portInfo, -1);
		while (0 == snd_seq_query_next_port(seq_handle, portInfo))
		{
			int port = snd_seq_port_info_get_port(portInfo);

			// see if this port's capabilities match what we want
			if (caps & snd_seq_port_info_get_capability(portInfo))
			{
				// pack the client and port number into a single int
				uint id = (client << 8) | port;
				ports.append(id);
			}
		}
	}

	return ports;
}

InputThread::InputThread(QObject *parent)
	: QThread(parent)
{
	this->handle = seq_handle;
	this->npfd = snd_seq_poll_descriptors_count(this->handle, POLLIN);
	this->pfd = (pollfd*)malloc(this->npfd * sizeof(pollfd));
	snd_seq_poll_descriptors(this->handle, this->pfd, this->npfd, POLLIN);
}

InputThread::~InputThread()
{
	free(this->pfd);
}

void InputThread::run()
{
	snd_midi_event_t *decoder;
	// TODO: change size for sysex
	snd_midi_event_new(16, &decoder);

	// don't use running status
	snd_midi_event_no_status(decoder, 1);

	snd_seq_event_t *ev;

	// short MIDI message buffer
	quint8 data[3];
	// input start time
	QElapsedTimer timer;
	timer.start();

	while (!this->isInterruptionRequested())
	{
		do
		{
			if (poll(this->pfd, this->npfd, 100) <= 0)
				break;
			snd_seq_event_input(this->handle, &ev);

			uint time = timer.elapsed();

			switch (ev->type)
			{
			case SND_SEQ_EVENT_SYSEX:
				// TODO
				break;

			default:
				if (0 < snd_midi_event_decode(decoder, data, 3, ev))
				{
					// TODO: timestamp
					emit this->midiEvent(data[0], data[1], data[2], time);
				}
				break;
			}

			snd_seq_free_event(ev);

		} while (snd_seq_event_input_pending(this->handle, 0) > 0);

		QThread::msleep(1);
	}

	snd_midi_event_free(decoder);
}
