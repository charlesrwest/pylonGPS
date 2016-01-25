#include "casterBasestationWidget.hpp"

using namespace pylongps;

/**
This function initializes the so that it can be shown and fills in the details with the given basestation's information.
@param inputBasestationID: The casterID/basestationID combination that uniquely identifies a basestation
@param inputBasestation: The basestation to populate the fields with
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
casterBasestationWidget::casterBasestationWidget(std::pair<int64_t, int64_t> inputBasestationID, const base_station_stream_information &inputBasestation, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

updateBasestation(inputBasestation);
basestationID = inputBasestationID;

connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(addBasestationPushButton, SIGNAL(clicked(bool)), this, SLOT(emitBasestationAddedAsSelectedDataSourceSignal()));

shortDescriptionFrame->installEventFilter(this);
}

/**
This function updates all of the labels with the given basestation's information.
@param inputBasestation: The basestation to use for the updated information 
*/
void casterBasestationWidget::updateBasestation(const base_station_stream_information &inputBasestation)
{
basestation = inputBasestation;
syncLabels();
}

/**
This function updates the labels to match the basestation information that has been stored internally. 
*/
void casterBasestationWidget::syncLabels()
{
std::string basestationNameLabel = basestation.informal_name().size() == 0 ? "Unknown" : basestation.informal_name();

informalBasestationNameLabel->setText(QString(basestationNameLabel.c_str()));
latitudeLabel->setText(QString(("Lat: " + std::to_string(basestation.latitude())).c_str()));
longitudeLabel->setText(QString(("Lon: " + std::to_string(basestation.longitude())).c_str()));
formatLabel->setText(QString(("Format: "+ corrections_message_format_Name(basestation.message_format())).c_str()));
classLabel->setText(QString(("Class: "+ base_station_class_Name(basestation.station_class())).c_str()));
int tenthsOfARate = (int) basestation.expected_update_rate()*10;
expectedUpdateRateLabel->setText(QString(("Rate: " + std::to_string(tenthsOfARate/10)+"."+std::to_string(tenthsOfARate%10)+" per second").c_str()));
int64_t numberOfDecidays = (int64_t) (10*basestation.uptime()/1000000)/(60*60*24);

uptimeLabel->setText(QString(("Uptime: " + std::to_string(numberOfDecidays/10) + "."+std::to_string(numberOfDecidays%10)+" days").c_str()));
}

/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool casterBasestationWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
{
if(inputObject == (QObject*) shortDescriptionFrame)
{
if(inputEvent->type() == QEvent::Enter)
{
emit MouseEnteredSummarySection();
}
return true;
}
else
{
return QFrame::eventFilter(inputObject, inputEvent);
}

}

/**
This function hides the details associated with the basestation.
*/
void casterBasestationWidget::makeDetailsDisappear()
{
//detailsFrame->hide();
shortDescriptionFrame->setMaximumHeight(27);
int height = shortDescriptionFrame->height();
detailsFrame->setMaximumHeight(0);
//setMaximumWidth(width());
setMaximumHeight(height+3);
adjustSize();
}

/**
This function cases the object to emit the basestationAddedAsSelectedDataSource with the basestation member's ID.
*/
void casterBasestationWidget::emitBasestationAddedAsSelectedDataSourceSignal()
{
emit basestationAddedAsSelectedDataSource(basestationID);
}

/**
This function shows the details associated with the basestation.
*/
void casterBasestationWidget::makeDetailsAppear()
{
//detailsFrame->setVisible(true);
setMaximumHeight(81);
detailsFrame->setMaximumHeight(51);
adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool casterBasestationWidget::event(QEvent *inputEvent)
{
if(inputEvent->type() == QEvent::Leave)
{
emit makeDetailsDisappear();
}

return QFrame::event(inputEvent);
}

/**
This function updates the distance to the map center label
@param inputDistanceInMeters: How far to display on the widget
*/
void casterBasestationWidget::setDistanceLabel(double inputDistanceInMeters)
{
distanceLabel->setText(QString((std::to_string(inputDistanceInMeters/1000.0) + " km").c_str()));
}
