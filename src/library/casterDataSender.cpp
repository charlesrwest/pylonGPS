#include "casterDataSender.hpp"

using namespace pylongps;

/**
This function initializes the casterDataSender to establish a connection and register an unauthenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 

@throws: This function can throw exceptions
*/
casterDataSender::casterDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate) : casterDataSender(inputSourceConnectionString, inputContext, std::string(),  credentials(), inputCasterRegistrationIPAddressAndPort, inputLatitude, inputLongitude, inputMessageFormat, inputInformalName, inputExpectedUpdateRate, false)
{ //Delegate to more complex constructor
}

/**
This function initializes the casterDataSender to establish a connection and register an authenticated basestation with it.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputSecretSigningKey: The libsodium signing secret key (crypto_sign_SECRETKEYBYTES length)
@param inputCredentials: The credentials message that has been generated for use with this signing key
@param inputCasterRegistrationIPAddressAndPort: The IP address/port to use for registration with the caster in the form "IPAddress:Port"
@param inputLatitude: The approximate latitude of the basestation
@param inputLongitude: The approximate longitude of the basestation
@param inputMessageFormat: The message format used with the updates
@param inputInformalName: The name that should be displayed when the basestation appears in a list
@param inputExpectedUpdateRate: Expected updates per second 
@param inputIsAuthenticatedConnection: Flag to allow function to be used to create unauthenticated connections so that it can be used a delegate constructor

@throws: This function can throw exceptions
*/
casterDataSender::casterDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, const std::string &inputSecretSigningKey, const credentials &inputCredentials, const std::string &inputCasterRegistrationIPAddressAndPort, double inputLatitude, double inputLongitude, corrections_message_format inputMessageFormat, const std::string &inputInformalName, double inputExpectedUpdateRate, bool inputIsAuthenticatedConnection) : context(inputContext)
{
if(inputIsAuthenticatedConnection)
{
if(inputSecretSigningKey.size() != crypto_sign_SECRETKEYBYTES)
{
throw SOMException("The signing key is an incorrect length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

secretKey = inputSecretSigningKey;
basestationCredentialsMessage = inputCredentials;
}

informationSourceConnectionString = inputSourceConnectionString;

//Create and connect socket to use for communicate with the caster
SOM_TRY
sendingSocket.reset(new zmq::socket_t(context, ZMQ_DEALER));
SOM_CATCH("Error making socket\n")

SOM_TRY //Set maximum wait for registration response
sendingSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &CASTER_DATA_SENDER_MAX_WAIT_TIME, sizeof(CASTER_DATA_SENDER_MAX_WAIT_TIME));
SOM_CATCH("Error setting socket timeout\n")

SOM_TRY //Connect to caster
std::string connectionString = "tcp://"+inputCasterRegistrationIPAddressAndPort;
sendingSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting to caster\n")

//Create registration request message
transmitter_registration_request registrationRequest;
transmitter_registration_reply registrationReply;
auto basestationInfo = registrationRequest.mutable_stream_info();
basestationInfo->set_latitude(inputLatitude);
basestationInfo->set_longitude(inputLongitude);
basestationInfo->set_expected_update_rate(inputExpectedUpdateRate);
basestationInfo->set_message_format(inputMessageFormat);
basestationInfo->set_informal_name(inputInformalName);

if(secretKey.size() != 0)
{ //This is an authenticated basestation stream
(*registrationRequest.mutable_transmitter_credentials()) = basestationCredentialsMessage;
}

//Send request to caster and get response
bool replyReceived = false;
bool replyDeserialized = false;

SOM_TRY
std::tie(replyReceived, replyDeserialized) = remoteProcedureCall(*sendingSocket, registrationRequest, registrationReply);
SOM_CATCH("Error, unable to complete RPC\n")

if(!replyReceived || !replyDeserialized)
{
throw SOMException("Registration failed\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}

if(!registrationReply.request_succeeded())
{
throw SOMException("Registration failed\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
} //We have registered as a basestation and the socket can now be used for distributing updates

//Construct reactor
SOM_TRY
senderReactor.reset(new reactor<casterDataSender>(&context, this));
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
std::tie(notificationConnectionString, extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*notificationPublishingSocket, "zmqDataSenderNotificationSocketAddress");
SOM_CATCH("Error binding notificationPublishingSocket\n")

//Give ownership of the subscriber socket to the reactor
SOM_TRY
senderReactor->addInterface(subscriberSocket, &casterDataSender::readAndWriteData);
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
std::string casterDataSender::notificationAddress()
{
return notificationConnectionString;
}

/**
This function forwards any received messages to the "sending" socket
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool casterDataSender::readAndWriteData(reactor<casterDataSender> &inputReactor, zmq::socket_t &inputSocket)
{
try
{
zmq::message_t messageBuffer;


SOM_TRY
inputSocket.recv(&messageBuffer);
SOM_CATCH("Error, unable to receive message\n")

if(secretKey.size() == 0)
{ //Unauthenticated connection
SOM_TRY
sendingSocket->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error forwarding to publisher\n")
}
else
{ //Authenticated connection, so add signature
std::string messageString((char *) messageBuffer.data(), messageBuffer.size());
std::string messageStringWithSignature = calculateAndPreappendSignature(messageString, secretKey);

SOM_TRY
sendingSocket->send(messageStringWithSignature.c_str(), messageStringWithSignature.size());
SOM_CATCH("Error forwarding to publisher\n")
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
