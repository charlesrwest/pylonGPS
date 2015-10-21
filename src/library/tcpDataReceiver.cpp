#include "tcpDataReceiver.hpp"

using namespace pylongps;

/**
This function initializes the tcpDataReceiver to retrieve data from the given TCP address/port.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
tcpDataReceiver::tcpDataReceiver(const std::string &inputIPAddressAndPort, zmq::context_t &inputContext) : context(inputContext)
{
//Construct reactor
SOM_TRY
receiverReactor.reset(new reactor<tcpDataReceiver>(&context, this));
SOM_CATCH("Error initializing reactor\n")

//Create socket to read from the TCP port
std::unique_ptr<zmq::socket_t> TCPReadingSocket;

SOM_TRY
TCPReadingSocket.reset(new zmq::socket_t(context, ZMQ_STREAM));
SOM_CATCH("Error making socket\n")

//Connect to the tcp server to get data from
SOM_TRY
std::string connectionString = "tcp://" + inputIPAddressAndPort;
TCPReadingSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting to TCP server\n")

//Create socket for publishing
SOM_TRY
publishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
int extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(publisherConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*publishingSocket, "tcpDataReceiverSocketAddress");
SOM_CATCH("Error binding publishingSocket\n")

//Create socket for notification publishing
SOM_TRY
notificationPublishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
SOM_TRY //Bind to an dynamically generated address
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "tcpDataReceiverNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the socket to the reactor
SOM_TRY
receiverReactor->addInterface(TCPReadingSocket, &tcpDataReceiver::readAndPublishData, publisherConnectionString);
SOM_CATCH("Error, unable to add interface\n")

//Start the reactor thread
SOM_TRY
receiverReactor->start();
SOM_CATCH("Error, unable to add interface\n")
}


/**
This function returns a string containing the ZMQ connection string required to connect this object's publisher (which forwards data from the associated file).
@return: The connection string to use to connect to this data source
*/
std::string tcpDataReceiver::address()
{
return publisherConnectionString;
}

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string tcpDataReceiver::notificationAddress()
{
return notificationConnectionString;
}

/**
This function reads from the TCP port and forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from
@return: false if the reactor doesn't need to restart its poll cycle

@throw: This function can throw exceptions
*/
bool tcpDataReceiver::readAndPublishData(reactor<tcpDataReceiver> &inputReactor, zmq::socket_t &inputSocket)
{
try
{
zmq::message_t messageBuffer;

SOM_TRY
inputSocket.recv(&messageBuffer);
SOM_CATCH("Error, unable to receive message\n")

if(!messageBuffer.more())
{
SOM_TRY
publishingSocket->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error publishing data\n")
}

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
