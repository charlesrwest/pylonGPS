#pragma once

#include<QWidget>
#include "ui_fileDataReceiverWidgetFramework.h"
#include<map>
#include "base_station_stream_information.pb.h"
#include<utility>

namespace pylongps
{

/**
This class represents the widget which displays the details for a file data receiver and allows it to create related senders.
*/ 
class fileDataReceiverWidget: public QFrame, public Ui::fileDataReceiverGUIWidget
{
Q_OBJECT

public:
/**
This function initializes the so that it can be shown and fills in the details with the given data receivers's.
@param inputFileDataReceiverConnectionString: The connection string associated with the related data receiver
@param inputFilePath: The path to the file this receiver is getting data from
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
fileDataReceiverWidget(const std::string &inputFileDataReceiverConnectionString, const std::string &inputFilePath, QWidget *inputParent = 0, Qt::WindowFlags inputFlags = 0);


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

std::string dataReceiverConnectionString;
std::string dataReceiversSourceFilePath;

public slots:
/**
This function hides the details associated with the basestation.
*/
void makeDetailsDisappear();

/**
This function shows the details associated with the basestation.
*/
void makeDetailsAppear();

/**
This function cases the object to emit the dataReceiverPlusButtonPressed with the basestation member's ID.
*/
void emitDataReceiverPlusButtonPressed();

/**
This function cases the object to emit the basestationAddedAsSelectedDataSource with the basestation member's ID.
*/
void emitDataReceiverMinusButtonPressed();

signals:
void MouseEnteredSummarySection();
void MouseLeftFrame();
void dataReceiverPlusButtonPressed(std::string); //Triggered when + button pressed, emits related connection string
void dataReceiverMinusButtonPressed(std::string); //Triggered when + button pressed, emits related connection string
void sizeChanged();
void widgetMoved();
}; 












} 
