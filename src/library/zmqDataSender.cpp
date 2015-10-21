#include "zmqDataSender.hpp"

using namespace pylongps;

/**
This function initializes the zmqDataSender to publish data using a ZMQ PUB socket on the given port.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputPortNumberToPublishOn: The TCP port number to bind/use for publishing

@throws: This function can throw exceptions
*/
zmqDataSender::zmqDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, int inputPortNumberToPublishOn) : context(inputContext)
{
if(inputPortNumberToPublishOn < 0)
{
throw SOMException("Received negative port number\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

informationSourceConnectionString = inputSourceConnectionString;

//Construct reactor
SOM_TRY
senderReactor.reset(new reactor<zmqDataSender>(&context, this));
SOM_CATCH("Error initializing reactor\n")



//Create socket for publishing
SOM_TRY
publishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

std::string connectionString = "tcp://*:" +std::to_string(inputPortNumberToPublishOn);

SOM_TRY //Bind
publishingSocket->bind(connectionString.c_str());
SOM_CATCH("Error connecting socket\n")

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
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "zmqDataSenderNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the subscriber socket to the reactor
SOM_TRY
senderReactor->addInterface(subscriberSocket, &zmqDataSender::readAndWriteData);
SOM_CATCH("Error, unable to add interface\n")

//Start the reactor thread
SOM_TRY
senderReactor->start();
SOM_CATCH("Error, unable to start interface\n")
}



/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string zmqDataSender::notificationAddress()
{
return notificationConnectionString;
}

/**
This function forwards any received messages to the publisher socket
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool zmqDataSender::readAndWriteData(reactor<zmqDataSender> &inputReactor, zmq::socket_t &inputSocket)
{
try
{
zmq::message_t messageBuffer;

SOM_TRY
inputSocket.recv(&messageBuffer);
SOM_CATCH("Error, unable to receive message\n")

SOM_TRY
publishingSocket->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error forwarding to publisher\n")

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
