#include "fileDataSenderWidget.hpp"

using namespace pylongps;

/**
This function initializes the so that it can be shown and fills in the details with the given data sender's.
@param inputFileDataSenderIDString: The ID string associated with the related data sender
@param inputFilePath: The path to the file this sender is sending data to
@param inputParent: The widget which takes responsibility for this object's deletion/resize (if any)
@param inputFlags: The flags to pass to the QFrame base class
*/
fileDataSenderWidget::fileDataSenderWidget(const std::string &inputFileDataSenderIDString, const std::string &inputFilePath, QWidget *inputParent, Qt::WindowFlags inputFlags) : QFrame(inputParent, inputFlags)
{
setupUi(this);
makeDetailsDisappear();
setFocusPolicy(Qt::StrongFocus);

dataSenderIDString = inputFileDataSenderIDString;
dataSenderSourceFilePath = inputFilePath;
filePathLabel->setText(QString(dataSenderSourceFilePath.c_str()));

connect(this, SIGNAL(MouseEnteredSummarySection()), this, SLOT(makeDetailsAppear()));
connect(minusPushButton, SIGNAL(clicked(bool)), this, SLOT(emitDataSenderMinusButtonPressed()));

shortDescriptionFrame->installEventFilter(this);
}


/**
This function intercepts events that happen to the frame (including mouse hover) and emits the appropriate signals.
@param inputObject: The object that prompted the event signal
@param inputEvent: The event signal
*/
bool fileDataSenderWidget::eventFilter(QObject *inputObject, QEvent *inputEvent)
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
void fileDataSenderWidget::makeDetailsDisappear()
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
void fileDataSenderWidget::emitDataSenderMinusButtonPressed()
{
emit dataSenderMinusButtonPressed(dataSenderIDString);
}

/**
This function shows the details associated with the data sender.
*/
void fileDataSenderWidget::makeDetailsAppear()
{
//setMaximumHeight(81);
//detailsFrame->setMaximumHeight(51);
//adjustSize();
}

/**
This function processes events for this frame.
@param inputEvent: The event signal
*/
bool fileDataSenderWidget::event(QEvent *inputEvent)
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

