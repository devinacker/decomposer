#ifndef ALSA_H
#define ALSA_H

#include <alsa/asoundlib.h>
#include <QList>
#include <QThread>

namespace ALSA
{
	extern snd_seq_t *seq_handle;
	extern int seq_client;
	extern int seq_inport, seq_outport;

	int init();

	QList<uint> enumerate(int caps);
}

class InputThread : public QThread
{
	Q_OBJECT

public:
	InputThread(QObject *parent = nullptr);
	~InputThread();
	void run();

signals:
	void midiEvent(quint8 event, quint8 data1, quint8 data2, uint time);

private:
	snd_seq_t *handle;
	int npfd;
	struct pollfd *pfd;
};

#endif // ALSA_H
