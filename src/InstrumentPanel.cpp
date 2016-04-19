#include "InstrumentPanel.h"
#include "ui_InstrumentPanel.h"

#include "Instrument.h"

#include "devices/MIDIinput.h"
#include "devices/MIDIoutput.h"

#include <QMessageBox>

InstrumentPanel::InstrumentPanel(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::InstrumentPanel)
	, m_pCurrInput(nullptr)
	, m_pCurrOutput(nullptr)
{
	ui->setupUi(this);

	for (int i = 0; i < 64; i++)
	{
		m_instruments[i].channel = (i % 16);
	}
	m_pCurrInst = &m_instruments[0];
	updateForm();

	// set up ui controls
	auto valueChangedInt = static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged);
	auto valueChangedDouble = static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);

	connect(ui->editInstNum, valueChangedInt, [=](int val)
	{
		m_pCurrInst = &m_instruments[val-1];
		updateForm();
	});

	connect(ui->editInstName, &QLineEdit::editingFinished, [=]()
	{
		m_pCurrInst->name = ui->editInstName->text();
	});

	connect(ui->btnRecordParams, &QAbstractButton::clicked, [=]()
	{
		QMessageBox::information(this, windowTitle(), "this ain't done yet");
	});

	connect(ui->btnResetParams, &QAbstractButton::clicked, [=]()
	{
		if (QMessageBox::Yes == QMessageBox::question(
					this, windowTitle(), tr("Really reset this instrument?"),
					QMessageBox::Yes | QMessageBox::No))
		{
			int chn = m_pCurrInst->channel;

			*m_pCurrInst = Instrument();
			m_pCurrInst->channel = chn;

			updateForm();
		}
	});

	connect(ui->editBank, valueChangedInt, [=](int val)
	{
		m_pCurrInst->bank = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editBankLSB, valueChangedInt, [=](int val)
	{
		m_pCurrInst->bankLSB = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editBendRange, valueChangedDouble, [=](double val)
	{
		m_pCurrInst->bendRange = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editOutputChn, valueChangedInt, [=](int val)
	{
		m_pCurrInst->channel = val - 1;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editProgramNum, valueChangedInt, [=](int val)
	{
		m_pCurrInst->program = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editTranspose, valueChangedDouble, [=](double val)
	{
		m_pCurrInst->transpose = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editVelocity, valueChangedInt, [=](int val)
	{
		m_pCurrInst->velocity = val;
		m_pCurrInst->shouldReset = true;
	});

	connect(ui->editPitchCenter, valueChangedDouble, [=](double val)
	{
		m_pCurrInst->pitchCenter = val;
		m_pCurrInst->shouldReset = true;
	});
}

// ------------------------------------------------------------------------------------------------
InstrumentPanel::~InstrumentPanel()
{
	delete ui;
}

// ------------------------------------------------------------------------------------------------
void InstrumentPanel::updateForm()
{
	ui->editInstName->setText(m_pCurrInst->name);
	ui->editOutputChn->setValue(m_pCurrInst->channel + 1);
	ui->editVelocity->setValue(m_pCurrInst->velocity);
	ui->editProgramNum->setValue(m_pCurrInst->program);
	ui->editBank->setValue(m_pCurrInst->bank);
	ui->editBankLSB->setValue(m_pCurrInst->bankLSB);
	ui->editTranspose->setValue(m_pCurrInst->transpose);
	ui->editBendRange->setValue(m_pCurrInst->bendRange);

}

// ------------------------------------------------------------------------------------------------
void InstrumentPanel::setInputDevice(MIDIInput *input)
{
	if (m_pCurrInput)
	{
		disconnect(m_pCurrInput, 0, this, 0);
		disconnect(this, 0, m_pCurrInput, 0);
	}

	m_pCurrInput = input;

	if (input)
	{
		connect(m_pCurrInput, SIGNAL(midiEvent(quint8, quint8, quint8, uint)),
				this, SLOT(receiveMIDI(quint8, quint8, quint8)));
	}
}

// ------------------------------------------------------------------------------------------------
void InstrumentPanel::setOutputDevice(MIDIOutput *output)
{
	if (m_pCurrOutput)
	{
		disconnect(m_pCurrOutput, 0, this, 0);
		disconnect(this, 0, m_pCurrOutput, 0);
	}

	m_pCurrOutput = output;
}

// ------------------------------------------------------------------------------------------------
void InstrumentPanel::receiveMIDI(quint8 event, quint8 data1, quint8 data2)
{
	if (!isVisible()) return;

	switch (event & 0xF0)
	{
	case EVENT_NOTEOFF(0):
		if (m_pCurrOutput)
			m_pCurrInst->noteOff(m_pCurrOutput, data1);
		break;

	case EVENT_NOTEON(0):
		if (m_pCurrOutput)
			m_pCurrInst->noteOn(m_pCurrOutput, data1);
		break;

	case EVENT_PITCH(0):
		if (m_pCurrOutput)
			m_pCurrInst->pitch(m_pCurrOutput, MIDI_WORD(data2, data1));
		break;

	default:
		if (m_pCurrOutput)
			m_pCurrInst->send(m_pCurrOutput, event, data1, data2);
		break;

		// TODO: capture CC/PC messages here when recording
	}
}
