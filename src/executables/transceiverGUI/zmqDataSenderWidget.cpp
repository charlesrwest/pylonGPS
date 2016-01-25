#include "zmqDataSenderWidget.hpp"

using namespace pylongps;

/**
This function initializes the so that it can be shown and fills in the details with the given data sender's.
@param inputDataSenderIDString: The ID string associated with the related data sender
@param inputPortNumber: The port number this sender will try to bind
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
zmqDataSenderWidget::zmqDataSenderWidget(const std::string &inputDataSenderIDString, int64_t inputPortNumber, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

dataSenderIDString = inputDataSenderIDString;
portNumber = inputPortNumber;
portLabel->setText(QString(("Port: " + std::to_string(portNumber)).c_str()));

connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(minusPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataSenderMinusButtonPressed()));

shortDescriptionFrame->installEventFilter(this);
}


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool zmqDataSenderWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
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
void zmqDataSenderWidget::makeDetailsDisappear()
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
void zmqDataSenderWidget::emitDataSenderMinusButtonPressed()
{
emit dataSenderMinusButtonPressed(dataSenderIDString);
}

/**
This function shows the details associated with the data sender.
*/
void zmqDataSenderWidget::makeDetailsAppear()
{
setMaximumHeight(49);
detailsFrame->setMaximumHeight(20);
adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool zmqDataSenderWidget::event(QEvent *inputEvent)
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
