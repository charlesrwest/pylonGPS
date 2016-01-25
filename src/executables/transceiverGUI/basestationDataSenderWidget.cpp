#include "basestationDataSenderWidget.hpp"

using namespace pylongps;

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
basestationDataSenderWidget::basestationDataSenderWidget(const std::string &inputDataSenderIDString, const std::string &inputInformalBasestationName, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, double inputExpectedMessageRate, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

dataSenderIDString = inputDataSenderIDString;
informalBasestationName = inputInformalBasestationName;
latitude = inputLatitude;
longitude = inputLongitude;
messageFormat = inputMessageFormat;
expectedMessageRate = inputExpectedMessageRate;

informalBasestationNameLabel->setText(QString(inputInformalBasestationName.c_str()));
latitudeLabel->setText(QString(("Lat: " + std::to_string(latitude)).c_str()));
longitudeLabel->setText(QString(("Lon: " + std::to_string(longitude)).c_str()));
formatLabel->setText(QString(corrections_message_format_Name(messageFormat).c_str()));
expectedUpdateRateLabel->setText(QString(("Rate: " + std::to_string(expectedMessageRate) +" (1/s)").c_str()));


connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(minusPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataSenderMinusButtonPressed()));

shortDescriptionFrame->installEventFilter(this);
}


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool basestationDataSenderWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
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
This function hides the details associated with the data sender.
*/
void basestationDataSenderWidget::makeDetailsDisappear()
{
shortDescriptionFrame->setMaximumHeight(27);
int height = shortDescriptionFrame->height();
detailsFrame->setMaximumHeight(0);
setMaximumHeight(height+3);
adjustSize();
}


/**
This function cases the object to emit the dataReceiverMinusButtonPressed with the associated data sender ID.
*/
void basestationDataSenderWidget::emitDataSenderMinusButtonPressed()
{
emit dataSenderMinusButtonPressed(dataSenderIDString);
}

/**
This function shows the details associated with the data sender.
*/
void basestationDataSenderWidget::makeDetailsAppear()
{
setMaximumHeight(81);
detailsFrame->setMaximumHeight(51);
adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool basestationDataSenderWidget::event(QEvent *inputEvent)
{
if(inputEvent->type() == QEvent::Leave)
{
emit makeDetailsDisappear();
}

if(inputEvent->type() == QEvent::Resize)
{
bool returnValue = QFrame::event(inputEvent);
emit sizeChanged();
return returnValue;
}

if(inputEvent->type() == QEvent::Move)
{
emit widgetMoved();
}

return QFrame::event(inputEvent);
}  
