#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <QObject>
#include <QString>
#include <QCoreApplication>

class MIDIDevice : public QObject
{
	Q_OBJECT

public:
	MIDIDevice(uint id)
		: QObject(qApp)
		, m_deviceID(id)
		, m_valid(false)
	{
	}
	virtual ~MIDIDevice()
	{
	}

	virtual uint id() const { return m_deviceID; }
	virtual bool isValid() const { return m_valid; }

	virtual QString name() const = 0;

public slots:
	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual bool reset() = 0;

signals:
	/* Emitted when a device error occurs.
	 * The specific error text is platform-dependent.
	 */
	void error(QString err);

	/* Emitted when the device has been opened.
	 */
	void opened();

	/* Emitted when the device has been closed.
	 */
	void closed();

protected:
	uint m_deviceID;
	bool m_valid;

};

#endif // MIDIDEVICE_H

