#ifndef INSTRUMENTPANEL_H
#define INSTRUMENTPANEL_H

#include <QWidget>
#include "Instrument.h"

namespace Ui {
class InstrumentPanel;
}

class MIDIInput;
class MIDIOutput;

class InstrumentPanel : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentPanel(QWidget *parent = 0);
	~InstrumentPanel();

public slots:
	void setInputDevice(MIDIInput*);
	void setOutputDevice(MIDIOutput*);

private slots:
	void receiveMIDI(quint8 event, quint8 data1, quint8 data2);

private:
	Ui::InstrumentPanel *ui;

	void updateForm();

	// move this into the actual song eventually (once it exists)
	Instrument m_instruments[64];
	Instrument *m_pCurrInst;

	// pointers to devices selected on device panel
	MIDIInput *m_pCurrInput;
	MIDIOutput *m_pCurrOutput;
};

#endif // INSTRUMENTPANEL_H
