#pragma once

#include<QDialog>
#include "ui_createDataSenderDialogWindowFramework.h"
#include<string>
#include<QFileDialog>
#include <QDoubleValidator>
#include "utilityFunctions.hpp"
#include "common_enums.pb.h"
#include "credentials.pb.h"

namespace pylongps
{ 

/**
This class represents the dialog window that is used to create data senders that are sending received data to somewhere else.
*/
class dataSenderDialogWindow : public QDialog, public Ui::createDataSenderDialogWindowFramework
{
Q_OBJECT

public:
/**
This function intitializes the object to emit the proper signals, load keys and restrict the possible inputs in the lineedits.
@param inputDataReceiverConnectionString: The data receiver that these senders will be associated with's address
@param inputParentWidget: The parent widget for this dialog
@param inputFlags: Flags for this dialog
*/
dataSenderDialogWindow(const std::string &inputDataReceiverConnectionString, QWidget *inputParentWidget = nullptr, Qt::WindowFlags inputFlags = 0);

std::string dataReceiverConnectionString;
std::string casterIPAddress;
int64_t casterRegistrationPortNumber;
std::string casterSecretKey;
credentials basestationCredentials;
bool credentialsLoaded = false;

std::string fileDataSenderPath;
std::string casterSenderSecretKeyPath;
std::string casterCredentialsPath;


public slots:
/**
This function opens a file selection dialog and allows a secret key to be loaded from a file for use with an authenticated basestation.
*/
void loadSecretKeyViaDialog();

/**
This function opens a file selection dialog and allows a credentials object to be loaded from a file for use with an authenticated basestation.
*/
void loadCredentialsViaDialog();

/**
This function opens a QDialog to select a file to send data to.
*/
void selectDataSourceFile();

/**
This function emits the createBasestationDataSender signal with the data in the dialog's widgets.
*/
void emitCreateBasestationDataSender();

/**
This function emits a createFileDataSender signal with fileDataSenderPath as the value.
*/
void emitCreateFileDataSenderSignal();

/**
This function emits the createTCPDataReader signal with the IP address/port in the TCP tab's widgets.
*/
void emitCreateTCPDataSenderSignal();

/**
This function emits the createZMQDataSender signal with the IP address/port in the ZMQ tab's widgets.
*/
void emitCreateZMQDataSender();

signals:
void createBasestationDataSender(std::string inputDataReceiverConnectionString, std::string inputInformalName, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, double inputExpectedUpdateRate, std::string inputBasestationSecretKey, credentials inputCredentials, bool credentialsLoaded);
void createFileDataSender(std::string inputDataReceiverConnectionString, std::string inputFilePath);
void createTCPDataSender(std::string inputDataReceiverConnectionString, int64_t inputPortNumber);
void createZMQDataSender(std::string inputDataReceiverConnectionString, int64_t inputPortNumber);
};











} 
