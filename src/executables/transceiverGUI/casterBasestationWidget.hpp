#pragma once

#include<QWidget>
#include "ui_casterBaseStationWidgetFramework.h"
#include<map>
#include "base_station_stream_information.pb.h"
#include<utility>

namespace pylongps
{

/**
This class represents the widget which displays the details for a basestation and allows it to be selected for forwarding.
*/ 
class casterBasestationWidget: public QFrame, public Ui::basestationGUIWidget
{
Q_OBJECT

public:
/**
This function initializes the so that it can be shown and fills in the details with the given basestation's information.
@param inputBasestationID: The casterID/basestationID combination that uniquely identifies a basestation
@param inputBasestation: The basestation to populate the fields with
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
casterBasestationWidget(std::pair<int64_t, int64_t> inputBasestationID, const base_station_stream_information &inputBasestation, QWidget *inputParent = 0, Qt::WindowFlags inputFlags = 0);

/**
This function updates all of the labels with the given basestation's information.
@param inputBasestation: The basestation to use for the updated information 
*/
void updateBasestation(const base_station_stream_information &inputBasestation);

/**
This function updates the distance to the map center label
@param inputDistanceInMeters: How far to display on the widget
*/
void setDistanceLabel(double inputDistanceInMeters);

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

signals:
void MouseEnteredSummarySection();
void MouseLeftFrame();
void basestationAddedAsSelectedDataSource(std::pair<int64_t, int64_t>); //Triggered when + button pressed, emits casterID/basestationID pair

}; 












}
