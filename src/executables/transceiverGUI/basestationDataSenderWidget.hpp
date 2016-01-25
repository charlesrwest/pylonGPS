#pragma once

#include<QWidget>
#include "ui_casterBaseStationDataSenderWidgetFramework.h"
#include<map>
#include "base_station_stream_information.pb.h"
#include<utility>

namespace pylongps
{

/**
This class represents the widget which displays the details for a data sender.
*/ 
class basestationDataSenderWidget: public QFrame, public Ui::basestationDataSenderGUIWidget
{
Q_OBJECT

public:
/**
This function initializes the so that it can be shown and fills in the details with the given data sender's.
@param inputDataSenderIDString: The ID string associated with the related data sender
@param inputInformalBasestationName: The string which people will see when they look at this basestation in a GUI
@param inputLatitude: The latitude to display
@param inputLongitude: The longitude to display
@param inputMessageFormat: The format of the messages this object is forwarding
@param inputExpectedMessageRate: Rate of message sending (1/s)
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
basestationDataSenderWidget(const std::string &inputDataSenderIDString, const std::string &inputInformalBasestationName, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, double inputExpectedMessageRate, QWidget *inputParent = 0, Qt::WindowFlags inputFlags = 0);


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
std::string informalBasestationName;
double latitude;
double longitude;
corrections_message_format messageFormat;
double expectedMessageRate;

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
