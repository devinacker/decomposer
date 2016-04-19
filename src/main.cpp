#include "mainwindow.h"
#include <QApplication>

#include "devices/MIDIinput.h"
#include "devices/MIDIoutput.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MIDIInput::enumerate();
	MIDIOutput::enumerate();

	MainWindow w;
	w.show();

	return a.exec();
}
