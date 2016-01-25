#include "createDataReceiverDialogWindow.hpp"

using namespace pylongps;

/**
This function intitializes the object to emit the proper signals, load keys and restrict the possible inputs in the lineedits.
@param inputParentWidget: The parent widget for this dialog
@param inputFlags: Flags for this dialog
*/
createDataReceiverDialogWindow::createDataReceiverDialogWindow(QWidget *inputParentWidget, Qt::WindowFlags inputFlags) : QDialog(inputParentWidget, inputFlags)
{
setupUi(this);

//Validate IP addresses
QRegExp IPAddressRegex("^0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])\\.0*([1-9]?\\d|1\\d\\d|2[0-4]\\d|25[0-5])$");
TCPReceiverIPAddressLineEdit->setValidator(new QRegExpValidator(IPAddressRegex, this));
TCPReceiverPortLineEdit->setValidator( new QIntValidator(0, 65535, this) );

ZMQDataReceiverIPAddressLineEdit->setValidator(new QRegExpValidator(IPAddressRegex, this));
ZMQDataReceiverPortLineEdit->setValidator( new QIntValidator(0, 65535, this) );

filePathTextLabel->setText("");

connect(selectFileDataSourcePushButton, SIGNAL(clicked(bool)), this, SLOT(selectDataSourceFile()));
connect(createFileDataReceiverPushButton, SIGNAL(clicked(bool)), this, SLOT(emitCreateFileDataReaderSignal()));

connect(createTCPDataReceiver, SIGNAL(clicked(bool)), this, SLOT(emitCreateTCPDataReaderSignal()));
connect(createZMQDataReceiverLineEdit, SIGNAL(clicked(bool)), this, SLOT(emitZMQDataReader()));

//Make window finish when data receiver added
connect(this, SIGNAL(createFileDataReader(std::string)), this, SLOT(close()));
connect(this, SIGNAL(createTCPDataReader(std::string, int64_t)), this, SLOT(close()));
connect(this, SIGNAL(createZMQDataReader(std::string, int64_t)), this, SLOT(close()));
}

/**
This function opens a QDialog to select a file to receive data from.
*/
void createDataReceiverDialogWindow::selectDataSourceFile()
{
//Get path
QString qPath;
qPath = QFileDialog::getOpenFileName(this, "Open File Data Source", QString(dataReaderFilePath.c_str()), "File Data Source (*)");
dataReaderFilePath = qPath.toStdString();
filePathTextLabel->setText(dataReaderFilePath.c_str());
}

/**
This function emits a createFileDataReader signal with dataReaderFilePath as the value.
*/
void createDataReceiverDialogWindow::emitCreateFileDataReaderSignal()
{
if(dataReaderFilePath.size() != 0)
{
emit createFileDataReader(dataReaderFilePath);
}
}

/**
This function emits the createTCPDataReader signal with the IP address/port in the TCP tab's widgets.
*/
void createDataReceiverDialogWindow::emitCreateTCPDataReaderSignal()
{
int64_t portNumber = 0;

if(convertStringToInteger(TCPReceiverPortLineEdit->text().toStdString(), portNumber) != true)
{
return;
}

emit createTCPDataReader(TCPReceiverIPAddressLineEdit->text().toStdString(), portNumber);
}

/**
This function emits the createZMQDataReader signal with the IP address/port in the ZMQ tab's widgets.
*/
void createDataReceiverDialogWindow::emitZMQDataReader()
{
int64_t portNumber = 0;

if(convertStringToInteger(ZMQDataReceiverPortLineEdit->text().toStdString(), portNumber) != true)
{
return;
}

emit createZMQDataReader(ZMQDataReceiverIPAddressLineEdit->text().toStdString(), portNumber);
}


