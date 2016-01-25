#pragma once

#include<QWidget>
#include "ui_casterBaseStationDataReceiverWidgetFramework.h"
#include<map>
#include "base_station_stream_information.pb.h"
#include<utility>

namespace pylongps
{

/**
This class represents the widget which displays the details for a basestation and allows it to be selected for forwarding.
*/ 
class casterBasestationDataReceiverWidget : public QFrame, public Ui::basestationDataReceiverGUIWidget
{
Q_OBJECT

public:
/**
This function initializes the so that it can be shown and fills in the details with the given basestation's information.
@param inputBasestationID: The casterID/basestationID combination that uniquely identifies a basestation
@param inputBasestation: The basestation to populate the fields with
@param inputDataReceiverConnectionString: The string to use to get data from the associated data receiver
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
casterBasestationDataReceiverWidget(std::pair<int64_t, int64_t> inputBasestationID, const base_station_stream_information &inputBasestation, const std::string &inputDataReceiverConnectionString, QWidget *inputParent = nullptr, Qt::WindowFlags inputFlags = 0);

/**
This function updates all of the labels with the given basestation's information.
@param inputBasestation: The basestation to use for the updated information 
*/
void updateBasestation(const base_station_stream_information &inputBasestation);

/**
This function updates the labels to match the basestation information that has been stored internally. 
*/
void syncLabels();

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

std::pair<int64_t, int64_t> basestationID;
base_station_stream_information basestation;
std::string dataReceiverConnectionString;

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
This function cases the object to emit the basestationAddedAsSelectedDataSource with the basestation member's ID.
*/
void emitBasestationAddedAsSelectedDataSourceSignal();

/**
This function cases the object to emit the basestationAddedAsSelectedDataSource with the data connection string.
*/
void emitBasestationAddedAsSelectedDataSourceDataReceiverSignal();

/**
This function emits the dataReceiverWidgetClosed with this widgets casterID/basestationID combination.
*/
void emitDataReceiverWidgetClosed();

signals:
void MouseEnteredSummarySection();
void MouseLeftFrame();
void basestationAddedAsSelectedDataSource(std::pair<int64_t, int64_t>); //Triggered when + button pressed, emits casterID/basestationID pair
void basestationAddedAsSelectedDataSource(std::string); //Triggered when + button pressed, emits data receiver string
void dataReceiverWidgetClosed(std::pair<int64_t, int64_t>); //Emits the casterID/basestationID associated with this widget
void sizeChanged();
void widgetMoved();
}; 












}
