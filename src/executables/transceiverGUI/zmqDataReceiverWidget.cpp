#include "zmqDataReceiverWidget.hpp"

using namespace pylongps;

/**
This function initializes the so that it can be shown and fills in the details with the given data receivers's.
@param inputDataReceiverConnectionString: The connection string associated with the related data receiver
@param inputIPAddress: A string representing the IP address of the ZMQ PUB socket the receiver is getting data from
@param inputPortNumber: The port number of the server
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
zmqDataReceiverWidget::zmqDataReceiverWidget(const std::string &inputDataReceiverConnectionString, const std::string &inputIPAddress, int64_t inputPortNumber, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

dataReceiverConnectionString = inputDataReceiverConnectionString;
IPAddress = inputIPAddress;
portNumber = inputPortNumber;

IPAddressLabel->setText(QString(("ZMQ: " + IPAddress).c_str()));
portLabel->setText(QString(("Port: " + std::to_string(portNumber)).c_str()));

connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(addDataSenderPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataReceiverPlusButtonPressed()));
connect(removeDataReceiverPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataReceiverMinusButtonPressed()));

shortDescriptionFrame->installEventFilter(this);
}


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool zmqDataReceiverWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
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
void zmqDataReceiverWidget::makeDetailsDisappear()
{
shortDescriptionFrame->setMaximumHeight(27);
int height = shortDescriptionFrame->height();
detailsFrame->setMaximumHeight(0);
setMaximumHeight(height+3);
adjustSize();
}

/**
This function cases the object to emit the dataReceiverPlusButtonPressed with the basestation member's ID.
*/
void zmqDataReceiverWidget::emitDataReceiverPlusButtonPressed()
{
emit dataReceiverPlusButtonPressed(dataReceiverConnectionString);
}


/**
This function cases the object to emit the basestationAddedAsSelectedDataSource with the basestation member's ID.
*/
void zmqDataReceiverWidget::emitDataReceiverMinusButtonPressed()
{
emit dataReceiverMinusButtonPressed(dataReceiverConnectionString);
}

/**
This function shows the details associated with the basestation.
*/
void zmqDataReceiverWidget::makeDetailsAppear()
{
setMaximumHeight(49);
detailsFrame->setMaximumHeight(28);
adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool zmqDataReceiverWidget::event(QEvent *inputEvent)
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
  
