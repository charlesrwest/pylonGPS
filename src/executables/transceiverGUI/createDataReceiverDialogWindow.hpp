#pragma once

#include<QDialog>
#include "ui_createDataReceiverDialogWindowFramework.h"
#include<string>
#include<QFileDialog>
#include "utilityFunctions.hpp"

namespace pylongps
{ 

/**
This class represents the dialog window that is used to create data receivers that are receiving from somewhere other than a caster.
*/
class createDataReceiverDialogWindow : public QDialog, public Ui::createDataReceiverDialogWindowFramework
{
Q_OBJECT

public:
/**
This function intitializes the object to emit the proper signals, load keys and restrict the possible inputs in the lineedits.
@param inputParentWidget: The parent widget for this dialog
@param inputFlags: Flags for this dialog
*/
createDataReceiverDialogWindow(QWidget *inputParentWidget = nullptr, Qt::WindowFlags inputFlags = 0);

std::string dataReaderFilePath;

public slots:
/**
This function opens a QDialog to select a file to receive data from.
*/
void selectDataSourceFile();

/**
This function emits a createFileDataReader signal with dataReaderFilePath as the value.
*/
void emitCreateFileDataReaderSignal();

/**
This function emits the createTCPDataReader signal with the IP address/port in the TCP tab's widgets.
*/
void emitCreateTCPDataReaderSignal();

/**
This function emits the createZMQDataReader signal with the IP address/port in the ZMQ tab's widgets.
*/
void emitZMQDataReader();

signals:
void createFileDataReader(std::string inputFilePath);
void createTCPDataReader(std::string inputIPAddress, int64_t inputPortNumber);
void createZMQDataReader(std::string inputIPAddress, int64_t inputPortNumber);
};











}
