#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "InstrumentPanel.h"
#include "DevicePanel.h"

// ------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QList<QWidget*> widgets;

	auto *devicePanel = new DevicePanel();
	ui->tabWidget->addTab(devicePanel, tr("Devices"));

	auto *instrumentPanel = new InstrumentPanel();
	widgets.append(instrumentPanel);
	ui->tabWidget->addTab(instrumentPanel, tr("Instruments"));

	for (QWidget *& widget: widgets)
	{
		connect(devicePanel, SIGNAL(inputChanged(MIDIInput*)), widget, SLOT(setInputDevice(MIDIInput*)));
		connect(devicePanel, SIGNAL(outputChanged(MIDIOutput*)), widget, SLOT(setOutputDevice(MIDIOutput*)));
	}
}

// ------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
