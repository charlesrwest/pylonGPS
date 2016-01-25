#include "fileDataReceiverWidget.hpp"

using namespace pylongps;

/**
This function initializes the so that it can be shown and fills in the details with the given data receivers's.
@param inputFileDataReceiverConnectionString: The connection string associated with the related data receiver
@param inputFilePath: The path to the file this receiver is getting data from
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
fileDataReceiverWidget::fileDataReceiverWidget(const std::string &inputFileDataReceiverConnectionString, const std::string &inputFilePath, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

dataReceiverConnectionString = inputFileDataReceiverConnectionString;
dataReceiversSourceFilePath = inputFilePath;
filePathLabel->setText(QString(dataReceiversSourceFilePath.c_str()));

connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(addDataSenderPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataReceiverPlusButtonPressed()));
connect(removeBasestationPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataReceiverMinusButtonPressed()));

shortDescriptionFrame->installEventFilter(this);
}


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool fileDataReceiverWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
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
void fileDataReceiverWidget::makeDetailsDisappear()
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
void fileDataReceiverWidget::emitDataReceiverPlusButtonPressed()
{
emit dataReceiverPlusButtonPressed(dataReceiverConnectionString);
}


/**
This function cases the object to emit the basestationAddedAsSelectedDataSource with the basestation member's ID.
*/
void fileDataReceiverWidget::emitDataReceiverMinusButtonPressed()
{
emit dataReceiverMinusButtonPressed(dataReceiverConnectionString);
}

/**
This function shows the details associated with the basestation.
*/
void fileDataReceiverWidget::makeDetailsAppear()
{
//setMaximumHeight(81);
//detailsFrame->setMaximumHeight(51);
//adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool fileDataReceiverWidget::event(QEvent *inputEvent)
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

