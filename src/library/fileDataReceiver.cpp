#include "fileDataReceiver.hpp"

using namespace pylongps;

/**
This function initializes the fileDataReceiver to retrieve data from the given file descriptor.  The object takes ownership of the file and closes the pointer on destruction.
@param inputFilePointer: A file stream pointer to retrieve data from
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
fileDataReceiver::fileDataReceiver(FILE *inputFilePointer, zmq::context_t &inputContext) : context(inputContext)
{
SOM_TRY
subConstructor(inputFilePointer);
SOM_CATCH("Error with subconstructor\n")
}

/**
This function initializes the fileDataReceiver to retrieve data from the file that the given path points to.
@param inputFilePath: The path to the file to retrieve data from
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
fileDataReceiver::fileDataReceiver(const std::string &inputFilePath, zmq::context_t &inputContext)  : context(inputContext)
{
FILE *file = fopen(inputFilePath.c_str(), "rb");
if(file == nullptr)
{
throw SOMException("Unable to open file path\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

SOM_TRY
subConstructor(file);
SOM_CATCH("Error with subconstructor\n")
}

/**
This function returns a string containing the ZMQ connection string required to connect this object's publisher (which forwards data from the associated file).
@return: The connection string to use to connect to this data source
*/
std::string fileDataReceiver::address()
{
return publisherConnectionString;
}

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string fileDataReceiver::notificationAddress()
{
return notificationConnectionString;
}

/**
This function performs a nonblocking read of the given file descriptor and (if there is any data) forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputFileDescriptor: The file descriptor to read from

@throw: This function can throw exceptions
*/
bool fileDataReceiver::readAndPublishData(reactor<fileDataReceiver> &inputReactor, FILE *inputFileDescriptor)
{
auto fileNumber = fileno(inputFileDescriptor);

int32_t numberOfBytesRead = 0;

numberOfBytesRead = read(fileNumber, (void *) dataBuffer, FILE_DATA_RECEIVER_DATA_BUFFER_SIZE);

if(numberOfBytesRead < 0)
{//Read error
//Send notification and then kill the reactor by throwing an exception
data_receiver_status_notification notification;
notification.set_unrecoverable_error_has_occurred(true);

SOM_TRY
sendProtobufMessage(*notificationPublishingSocket, notification);
SOM_CATCH("Error sending error notification\n")

throw SOMException("File descripter returned error\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}

if(numberOfBytesRead == 0)
{//False alert
return false;
}

SOM_TRY
publishingSocket->send(dataBuffer, numberOfBytesRead);
SOM_CATCH("Error publishing data\n")

return false;
}

/**
Helper function for the common elements between constructors that delegation doesn't appear to fit well.  Should only be called as part of a constructor
@param inputFilePointer: A file stream pointer to retrieve data from

@throw: This function can throw exceptions
*/
void fileDataReceiver::subConstructor(FILE *inputFilePointer)
{
if(inputFilePointer == nullptr)
{
throw SOMException("File pointer is nullptr\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Construct reactor
SOM_TRY
receiverReactor.reset(new reactor<fileDataReceiver>(&context, this));
SOM_CATCH("Error initializing reactor\n")

//Create socket for publishing
SOM_TRY
publishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
int extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(publisherConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*publishingSocket, "fileDataReceiverSocketAddress");
SOM_CATCH("Error binding publishingSocket\n")

//Create socket for notification publishing
SOM_TRY
notificationPublishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
SOM_TRY //Bind to an dynamically generated address
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "fileDataReceiverNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the file to the reactor
SOM_TRY
receiverReactor->addInterface(inputFilePointer, &fileDataReceiver::readAndPublishData, publisherConnectionString);
SOM_CATCH("Error, unable to add interface\n")

//Start the reactor thread
SOM_TRY
receiverReactor->start();
SOM_CATCH("Error, unable to add interface\n")
}
