#pragma once

#include<QWidget>
#include "ui_fileDataSenderWidgetFramework.h"
#include<map>
#include "base_station_stream_information.pb.h"
#include<utility>

namespace pylongps
{

/**
This class represents the widget which displays the details for a file data sender.
*/ 
class fileDataSenderWidget: public QFrame, public Ui::fileDataSenderGUIWidget
{
Q_OBJECT

public:
/**
This function initializes the so that it can be shown and fills in the details with the given data sender's.
@param inputFileDataSenderIDString: The ID string associated with the related data sender
@param inputFilePath: The path to the file this sender is sending data to
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
fileDataSenderWidget(const std::string &inputFileDataSenderIDString, const std::string &inputFilePath, QWidget *inputParent = 0, Qt::WindowFlags inputFlags = 0);


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool eventFilter(QObject *inputObject, QEvent *inputEvent);

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
virtual bool event(QEvent *inputEvent);

std::string dataSenderIDString;
std::string dataSenderSourceFilePath;

public slots:
/**
This function hides the details associated with the data sender.
*/
void makeDetailsDisappear();

/**
This function shows the details associated with the data sender.
*/
void makeDetailsAppear();

/**
This function cases the object to emit the dataReceiverMinusButtonPressed with the associated data sender ID.
*/
void emitDataSenderMinusButtonPressed();

signals:
void MouseEnteredSummarySection();
void MouseLeftFrame();
void dataSenderMinusButtonPressed(std::string);
void sizeChanged();
void widgetMoved();
}; 












}
