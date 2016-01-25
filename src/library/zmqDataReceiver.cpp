#include "zmqDataReceiver.hpp"

using namespace pylongps;

/**
This function initializes the zmqDataReceiver to retrieve data from the given ZMQ PUB socket.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
zmqDataReceiver::zmqDataReceiver(const std::string &inputIPAddressAndPort, zmq::context_t &inputContext) : zmqDataReceiver(inputIPAddressAndPort, 0, 0, inputContext, false)
{ //Delegating to more complex constructor

}

/**
This function initializes the zmqDataReceiver to retrieve data from the given given PylonGPS caster PUB socket.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputCasterID: The ID of the caster to listen to (host format)
@param inputStreamID: The stream ID associated with the stream to listen to (host format)
@param inputContext: A reference to the ZMQ context to use
@param inputSubscribingToCaster: True if this object is subscribing to a caster and needs to strip the casterID/streamID from the stream before forwarding it 

@throws: This function can throw exceptions
*/
zmqDataReceiver::zmqDataReceiver(const std::string &inputIPAddressAndPort, int64_t inputCasterID, int64_t inputStreamID, zmq::context_t &inputContext, bool inputSubscribingToCaster)  : context(inputContext)
{
stripHeader = inputSubscribingToCaster;
casterID = inputCasterID;
streamID = inputStreamID;

//Construct reactor
SOM_TRY
receiverReactor.reset(new reactor<zmqDataReceiver>(&context, this));
SOM_CATCH("Error initializing reactor\n")

//Create socket to read from the ZMQ port
std::unique_ptr<zmq::socket_t> ZMQReadingSocket;

SOM_TRY
ZMQReadingSocket.reset(new zmq::socket_t(context, ZMQ_SUB));
SOM_CATCH("Error making socket\n")

//Set filter to allow all entries
if(!stripHeader)
{//Not receiving from a PylonGPS caster, so don't filter by header
SOM_TRY
ZMQReadingSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for ZMQReadingSocket\n")
}
else
{ //Presumably casterID, streamID already set by higher level constructor
Poco::Int64 filter[2];
filter[0] = Poco::ByteOrder::toNetwork(casterID);
filter[1] = Poco::ByteOrder::toNetwork(streamID);

SOM_TRY  //Set filter for casterID,streamID in network format
ZMQReadingSocket->setsockopt(ZMQ_SUBSCRIBE, (void *) filter, sizeof(Poco::Int64)*2);
SOM_CATCH("Error setting subscription for ZMQReadingSocket\n")
}

//Connect to the ZMQ publisher to get data from
SOM_TRY
std::string connectionString = "tcp://" + inputIPAddressAndPort;
ZMQReadingSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting to ZMQ PUB socket\n")

//Create socket for publishing
SOM_TRY
publishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
int extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(publisherConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*publishingSocket, "zmqDataReceiverSocketAddress");
SOM_CATCH("Error binding publishingSocket\n")


//Create socket for notification publishing
SOM_TRY
notificationPublishingSocket.reset(new zmq::socket_t(context, ZMQ_PUB));
SOM_CATCH("Error making socket\n")

//Bind socket for publishing and store address
SOM_TRY //Bind to an dynamically generated address
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "zmqDataReceiverNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the socket to the reactor
SOM_TRY
receiverReactor->addInterface(ZMQReadingSocket, &zmqDataReceiver::readAndPublishData, publisherConnectionString);
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
std::string zmqDataReceiver::address()
{
return publisherConnectionString;
}

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string zmqDataReceiver::notificationAddress()
{
return notificationConnectionString;
}

/**
This function reads from the ZMQ PUB port and forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputFileDescriptor: The file descriptor to read from
@return: false if the reactor doesn't need to restart its poll cycle

@throw: This function can throw exceptions
*/
bool zmqDataReceiver::readAndPublishData(reactor<zmqDataReceiver> &inputReactor, zmq::socket_t &inputSocket)
{
try
{
zmq::message_t messageBuffer;

SOM_TRY
if(!inputSocket.recv(&messageBuffer))
{
return false; //False alarm 
}
SOM_CATCH("Error, unable to receive message\n")

if(!stripHeader)
{
SOM_TRY
publishingSocket->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error publishing data\n")
}
else
{
if(messageBuffer.size() < sizeof(Poco::Int64)*2)
{
return false; //Message is smaller then header, so ignore invalid message
}

SOM_TRY
publishingSocket->send(((char *) messageBuffer.data())+sizeof(Poco::Int64)*2, messageBuffer.size()-sizeof(Poco::Int64)*2);
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
