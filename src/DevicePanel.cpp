#include "DevicePanel.h"
#include "ui_DevicePanel.h"

#include "devices/MIDIdefs.h"
#include "devices/MIDIinput.h"
#include "devices/MIDIoutput.h"

#include <QTime>
#include <QFile>
#include <QFileDialog>

DevicePanel::DevicePanel(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DevicePanel)
	, m_pCurrInput(nullptr)
	, m_pCurrOutput(nullptr)
{
	ui->setupUi(this);


	// fill combos with all detected midi devices
	for (auto device : MIDIInput::getDevices())
	{
		ui->cmbInputDevices->addItem(device->name(), device->id());
	}
	for (auto device : MIDIOutput::getDevices())
	{
		ui->cmbOutputDevices->addItem(device->name(), device->id());
	}

	ui->cmbInputDevices->setCurrentIndex(-1);
	ui->cmbOutputDevices->setCurrentIndex(-1);

	connect(ui->cmbInputDevices, SIGNAL(currentIndexChanged(int)), this, SLOT(setInputDevice(int)));
	connect(ui->cmbOutputDevices, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputDevice(int)));
	connect(ui->btnRecordSysEx, SIGNAL(clicked(bool)), this, SLOT(recordSysEx()));

	connect(ui->btnResetOutput, &QPushButton::clicked, [=]()
	{
		if (m_pCurrInput)
			m_pCurrInput->reset();
		if (m_pCurrOutput)
			m_pCurrOutput->reset();
	});

	connect(ui->btnStartStream, &QPushButton::clicked, [=]()
	{
		if (m_pCurrOutput)
		{
			connect(m_pCurrOutput, SIGNAL(streamReady()), this, SLOT(updateStream()), Qt::UniqueConnection);
			m_pCurrOutput->streamStart();
		}
	});

	connect(ui->btnPauseStream, &QPushButton::clicked, [=]()
	{
		if (m_pCurrOutput)
		{
			m_pCurrOutput->streamPause();
		}
	});

	connect(ui->btnStopStream, &QPushButton::clicked, [=]()
	{
		if (m_pCurrOutput)
		{
			disconnect(m_pCurrOutput, SIGNAL(streamReady()), this, SLOT(updateStream()));
			m_pCurrOutput->streamStop();
		}
	});
}

// ------------------------------------------------------------------------------------------------
DevicePanel::~DevicePanel()
{
	delete ui;
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::updateStream()
{
	// test stream data

	// kick
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  36, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 36);

	// hihats
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  42, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 42);
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  44, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 44);
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  42, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 42);

	// snare
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  40, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 40);

	// hihats
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  42, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 42);
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  44, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 44);
	m_pCurrOutput->streamSend(0,  EVENT_NOTEON(9),  42, 127);
	m_pCurrOutput->streamSend(24, EVENT_NOTEOFF(9), 42);

//	qDebug("flushing stream, buffer size is %u", m_pCurrOutput->m_buffer.size());
	m_pCurrOutput->streamFlush();
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::log(const QString &str)
{
	ui->listInputData->addItem(str);
	if (ui->listInputData->count() > 100)
		delete ui->listInputData->takeItem(0);

	ui->listInputData->scrollToBottom();
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::setInputDevice(int index)
{
	if (m_pCurrInput)
	{
		log(tr("closing %1").arg(m_pCurrInput->name()));
		m_pCurrInput->close();

		disconnect(m_pCurrInput, 0, this, 0);
		disconnect(this, 0, m_pCurrInput, 0);
	}

	const auto& devices = MIDIInput::getDevices();

	if (index >= 0 && index < devices.size())
	{
		m_pCurrInput = devices[index];

		connect(m_pCurrInput, SIGNAL(error(QString)), this, SLOT(receiveError(QString)));
		connect(m_pCurrInput, SIGNAL(midiEvent(quint8, quint8, quint8, uint)),
				this, SLOT(receiveMIDI(quint8, quint8, quint8, uint)));
		connect(m_pCurrInput, SIGNAL(sysExRecorded(QByteArray, uint)),
				this, SLOT(receiveSysEx(QByteArray, uint)));

		log(tr("opening %1").arg(m_pCurrInput->name()));
		if (!m_pCurrInput->open())
		{
			log(tr("failed to open device"));
		}
	}
	else
	{
		m_pCurrInput = nullptr;
	}

	emit inputChanged(m_pCurrInput);
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::setOutputDevice(int index)
{
	if (m_pCurrOutput)
	{
		log(tr("closing %1").arg(m_pCurrOutput->name()));
		m_pCurrOutput->close();

		disconnect(m_pCurrOutput, 0, this, 0);
		disconnect(this, 0, m_pCurrOutput, 0);
	}

	const auto& devices = MIDIOutput::getDevices();

	if (index >= 0 && index < devices.size())
	{
		m_pCurrOutput = devices[index];

		connect(m_pCurrOutput, SIGNAL(error(QString)), this, SLOT(receiveError(QString)));

		log(tr("opening %1").arg(m_pCurrOutput->name()));
		if (!m_pCurrOutput->streamOpen())
		{
			log(tr("failed to open device"));
		}
	}
	else
	{
		m_pCurrOutput = nullptr;
	}

	emit outputChanged(m_pCurrOutput);
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::receiveMIDI(quint8 event, quint8 data1, quint8 data2, uint time)
{
	if (!isVisible()) return;

	if (m_pCurrOutput)
	{
		m_pCurrOutput->send(event, data1, data2);
	}

	QTime eventTime = QTime::fromMSecsSinceStartOfDay(time);

	QString str = tr("%1 channel %2 ")
			.arg(eventTime.toString("hh:mm:ss.zzz"))
			.arg((event & 0xF) + 1);

	uint16_t pitch = data1 | (data2 << 7);

	switch (event >> 4) // TODO: MIDI event and RPN enums
	{
	case 0x8:
		str += tr("note off: %1 at velo %2").arg(data1).arg(data2);
		break;

	case 0x9:
		str += tr("note on: %1 at velo %2").arg(data1).arg(data2);
		break;

	case 0xA:
		str += tr("aftertouch: note %1 pressure %2").arg(data1).arg(data2);
		break;

	case 0xB:
		str += tr("control %1 (%3) change to value %2")
				.arg(data1).arg(data2).arg(MIDI::getControlName(data1));
		break;

	case 0xC:
		str += tr("program change: %1").arg(data1);
		break;

	case 0xD:
		str += tr("channel aftertouch: pressure %1").arg(data1);
		break;

	case 0xE:
		str += tr("pitch wheel: %1").arg(pitch);
		break;

	default: // TODO: display system messages
		return;
	}

	log(str);
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::recordSysEx()
{
	if (!m_pCurrInput) return;

	QString sysexPath =
			QFileDialog::getSaveFileName(this,
										 tr("Save SysEx dump"),
										 QString("%1.syx").arg(m_pCurrInput->name()),
										 tr("SysEx dumps (*.syx)"));

	if (!sysexPath.isEmpty())
	{
		m_sysexPath = sysexPath;
		m_pCurrInput->recordSysEx();

		log(tr("SysEx recording started (reset device to cancel)"));
	}
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::receiveSysEx(QByteArray data, uint time)
{
	QTime eventTime = QTime::fromMSecsSinceStartOfDay(time);

	QString str = tr("%1 received SysEx (%2 bytes)")
			.arg(eventTime.toString("hh:mm:ss.zzz"))
			.arg(data.size());
	log(str);

	if (data.isEmpty()) return;

	QFile file(m_sysexPath);
	if (!file.open(QIODevice::WriteOnly))
	{
		log(tr("unable to save %1").arg(m_sysexPath));
		return;
	}

	file.write(data);
	file.close();

	log(tr("SysEx saved to %1").arg(m_sysexPath));
}

// ------------------------------------------------------------------------------------------------
void DevicePanel::receiveError(QString error)
{
	log(tr("error: %1").arg(error));
}
