#include "fileDataSender.hpp"

using namespace pylongps;

/**
This function initializes the fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputFilePointer: A file stream pointer to retrieve data from

@throws: This function can throw exceptions
*/
fileDataSender::fileDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, FILE *inputFilePointer) : filePointer(nullptr, &fclose), context(inputContext)
{
SOM_TRY
subConstructor(inputSourceConnectionString, inputFilePointer);
SOM_CATCH("Error with subconstructor\n")
}

/**
This function initializes the fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputZMQConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputFilePath: The path to the file to send data to

@throws: This function can throw exceptions
*/
fileDataSender::fileDataSender(const std::string &inputZMQConnectionString, zmq::context_t &inputContext, const std::string &inputFilePath) : filePointer(nullptr, &fclose), context(inputContext)
{
FILE *file = fopen(inputFilePath.c_str(), "rb");
if(file == nullptr)
{
throw SOMException("Unable to open file path\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

SOM_TRY
subConstructor(inputZMQConnectionString, file);
SOM_CATCH("Error with subconstructor\n")
}

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string fileDataSender::notificationAddress()
{
return notificationConnectionString;
}

/**
This function forwards any received messages to the given file descriptor
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool fileDataSender::readAndWriteData(reactor<fileDataSender> &inputReactor, zmq::socket_t &inputSocket)
{
try
{
zmq::message_t messageBuffer;

SOM_TRY
inputSocket.recv(&messageBuffer);
SOM_CATCH("Error, unable to receive message\n")

if(fwrite(messageBuffer.data(), messageBuffer.size(), 1, filePointer.get()) != 1)
{//File write error
throw SOMException("File write error\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}
fflush(filePointer.get());

}
catch(const std::exception &inputException)
{
//Send notification and then kill the reactor by throwing an exception
data_receiver_status_notification notification;
notification.set_unrecoverable_error_has_occurred(true);

SOM_TRY
sendProtobufMessage(*notificationPublishingSocket, notification);
SOM_CATCH("Error sending error notification\n")

throw inputException;
}

return false;
}

/**
Helper function for the common elements between constructors that delegation doesn't appear to fit well.  Should only be called as part of a constructor
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePointer: A file stream pointer to retrieve data from

@throw: This function can throw exceptions
*/
void fileDataSender::subConstructor(const std::string &inputSourceConnectionString, FILE *inputFilePointer)
{
if(inputFilePointer == nullptr)
{
throw SOMException("Received null file descriptor\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

informationSourceConnectionString = inputSourceConnectionString;

SOM_TRY
filePointer.reset(inputFilePointer);
SOM_CATCH("Error, unable to store file pointer\n")

//Construct reactor
SOM_TRY
senderReactor.reset(new reactor<fileDataSender>(&context, this));
SOM_CATCH("Error initializing reactor\n")

//Create socket for subscribing
SOM_TRY
subscriberSocket.reset(new zmq::socket_t(context, ZMQ_SUB));
SOM_CATCH("Error making socket\n")

//Set to receive all messages
SOM_TRY //Set filter to allow any published messages to be received
subscriberSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for subscriberSocket\n")

//Connect to information source
SOM_TRY
subscriberSocket->connect(informationSourceConnectionString.c_str());
SOM_CATCH("Error connecting socket\n")

//Create socket for notification publishing
SOM_TRY
notificationPublishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
int extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "tcpDataSenderNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the subscriber socket to the reactor
SOM_TRY
senderReactor->addInterface(subscriberSocket, &fileDataSender::readAndWriteData);
SOM_CATCH("Error, unable to add interface\n")

//Start the reactor thread
SOM_TRY
senderReactor->start();
SOM_CATCH("Error, unable to start interface\n")
}
