 #include "dataSenderDialogWindow.hpp"

using namespace pylongps;

/**
This function intitializes the object to emit the proper signals, load keys and restrict the possible inputs in the lineedits.
@param inputDataReceiverConnectionString: The data receiver that these senders will be associated with's address
@param inputParentWidget: The parent widget for this dialog
@param inputFlags: Flags for this dialog
*/
dataSenderDialogWindow::dataSenderDialogWindow(const std::string &inputDataReceiverConnectionString, QWidget *inputParentWidget, Qt::WindowFlags inputFlags) : QDialog(inputParentWidget, inputFlags)
{
qRegisterMetaType<corrections_message_format>("corrections_message_format");
qRegisterMetaType<credentials>("credentials");
setupUi(this);

dataReceiverConnectionString = inputDataReceiverConnectionString;

filePathTextLabel->setText("");

//Validate port numbers
TCPSenderPortLineEdit->setValidator( new QIntValidator(0, 65535, this) );
ZMQDataSenderPortLineEdit->setValidator( new QIntValidator(0, 65535, this) );
latitudeLineEdit->setValidator( new QDoubleValidator(-90.0, 90.0, 5, this) );
latitudeLineEdit->setValidator( new QDoubleValidator(-90.0, 90.0, 5, this) );
longitudeLineEdit->setValidator( new QDoubleValidator(-180.0, 180.0, 5, this) );
updateFrequencyLineEdit->setValidator( new QDoubleValidator(0.0, 1000.0, 5, this) );

auto formatDescriptor = corrections_message_format_descriptor();
for(int i=0; i<formatDescriptor->value_count(); i++)
{
messageFormatComboBox->addItem(QString(formatDescriptor->value(i)->name().c_str()));
} //Populate combobox

connect(loadSecretKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadSecretKeyViaDialog()));
connect(loadCredentialsPushButton, SIGNAL(clicked(bool)), this, SLOT(loadCredentialsViaDialog()));
connect(createBasestationPushButton, SIGNAL(clicked(bool)), this, SLOT(emitCreateBasestationDataSender()));

connect(selectOutputFilePushButton, SIGNAL(clicked(bool)), this, SLOT(selectDataSourceFile())); 
connect(createFileDataSenderPushButton, SIGNAL(clicked(bool)), this, SLOT(emitCreateFileDataSenderSignal()));

 
connect(createTCPDataSenderPushButton, SIGNAL(clicked(bool)), this, SLOT(emitCreateTCPDataSenderSignal()));

connect(createZMQDataSenderPushButton, SIGNAL(clicked(bool)), this, SLOT(emitCreateZMQDataSender()));


connect(this, SIGNAL(createBasestationDataSender(std::string, std::string, double, double, corrections_message_format, double, std::string, credentials, bool)), this, SLOT(close()));
connect(this, SIGNAL(createFileDataSender(std::string, std::string)), this, SLOT(close()));
connect(this, SIGNAL(createTCPDataSender(std::string, int64_t)), this, SLOT(close()));
connect(this, SIGNAL(createZMQDataSender(std::string, int64_t)), this, SLOT(close()));
}

/**
This function opens a file selection dialog and allows a secret key to be loaded from a file for use with an authenticated basestation.
*/
void dataSenderDialogWindow::loadSecretKeyViaDialog()
{
QString qPath;
qPath = QFileDialog::getOpenFileName(this, "Open Secret Key File", QString(casterSenderSecretKeyPath.c_str()), "Secret Keys (*.pylonSecretKey)");
casterSenderSecretKeyPath = qPath.toStdString();

//Try loading the key
std::string loadedKey;
loadedKey = loadSecretKeyFromFile(casterSenderSecretKeyPath);
if(loadedKey.size() == 0)
{
return;
}

//Key loaded OK
casterSecretKey = loadedKey;
}

/**
This function opens a file selection dialog and allows a credentials object to be loaded from a file for use with an authenticated basestation.
*/
void dataSenderDialogWindow::loadCredentialsViaDialog()
{
QString qPath;
qPath = QFileDialog::getOpenFileName(this, "Open Credentials File", QString(casterCredentialsPath.c_str()), "Pylon Credentials File (*.pylonCredentials)");
casterCredentialsPath = qPath.toStdString();

//Try loading the credentials object
if(loadProtobufObjectFromFile(casterSenderSecretKeyPath, basestationCredentials) == false)
{
return;
}

//Credentials loaded OK
credentialsLoaded = true;
}

/**
This function opens a QDialog to select a file to send data to.
*/
void dataSenderDialogWindow::selectDataSourceFile()
{
//Get path
QString qPath;
qPath = QFileDialog::getSaveFileName(this, QString("Open File to Send to"), QString(fileDataSenderPath.c_str()), QString("File Data Sink (*)"));
fileDataSenderPath = qPath.toStdString();
filePathTextLabel->setText(fileDataSenderPath.c_str());
}

/**
This function emits the createBasestationDataSender signal with the data in the dialog's widgets.
*/
void dataSenderDialogWindow::emitCreateBasestationDataSender()
{
std::string informalName = informalNameLineEdit->text().toStdString();
double latitude = 0.0;
double longitude = 0.0;
double expectedUpdateRate = 0.0;
if(!convertStringToDouble(latitudeLineEdit->text().toStdString(), latitude) )
{
return;
}

if(!convertStringToDouble(longitudeLineEdit->text().toStdString(), longitude) )
{
return;
}

if(!convertStringToDouble(updateFrequencyLineEdit->text().toStdString(), expectedUpdateRate) )
{
return;
}

auto formatDescriptor = corrections_message_format_descriptor();
if(messageFormatComboBox->currentIndex() >= formatDescriptor->value_count())
{
return;
}

corrections_message_format messageFormat = (corrections_message_format) formatDescriptor->value(messageFormatComboBox->currentIndex())->number();

emit createBasestationDataSender(dataReceiverConnectionString, informalNameLineEdit->text().toStdString(), latitude, longitude, messageFormat, expectedUpdateRate, casterSecretKey, basestationCredentials, credentialsLoaded);
}

/**
This function emits a createFileDataSender signal with fileDataSenderPath as the value.
*/
void dataSenderDialogWindow::emitCreateFileDataSenderSignal()
{
emit createFileDataSender(dataReceiverConnectionString, fileDataSenderPath);
}

/**
This function emits the createTCPDataReader signal with the IP address/port in the TCP tab's widgets.
*/
void dataSenderDialogWindow::emitCreateTCPDataSenderSignal()
{
int64_t portNumber;
if(!convertStringToInteger(TCPSenderPortLineEdit->text().toStdString(), portNumber) )
{
return;
}

emit createTCPDataSender(dataReceiverConnectionString, portNumber);
}

/**
This function emits the createZMQDataSender signal with the IP address/port in the ZMQ tab's widgets.
*/
void dataSenderDialogWindow::emitCreateZMQDataSender()
{
int64_t portNumber;
if(!convertStringToInteger(ZMQDataSenderPortLineEdit->text().toStdString(), portNumber) )
{
return;
}

emit createZMQDataSender(dataReceiverConnectionString, portNumber);
}

