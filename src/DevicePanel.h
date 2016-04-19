#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <QWidget>

namespace Ui {
class DevicePanel;
}

class MIDIInput;
class MIDIOutput;

class DevicePanel : public QWidget
{
	Q_OBJECT

public:
	explicit DevicePanel(QWidget *parent = 0);
	~DevicePanel();

private slots:
	void setInputDevice(int);
	void setOutputDevice(int);

	void recordSysEx();

	void receiveMIDI(quint8 event, quint8 data1, quint8 data2, uint time);
	void receiveSysEx(QByteArray data, uint time);
	void receiveError(QString);

	void updateStream();

signals:
	void resetClicked();

	void inputChanged(MIDIInput*);
	void outputChanged(MIDIOutput*);

private:
	Ui::DevicePanel *ui;

	MIDIInput *m_pCurrInput;
	MIDIOutput *m_pCurrOutput;

	QString m_sysexPath;

	void log(const QString &str);
};

#endif // DEVICEPANEL_H
