#include "caster.hpp"

using namespace pylongps;

/**
This function initializes the class, creates the associated database, and starts the two threads associated with it.
@param inputContext: The ZMQ context that this object should use
@param inputCasterID: The 64 bit ID associated with this caster (make sure it does not collide with caster IDs the clients are likely to run into)
@param inputTransmitterRegistrationAndStreamingPortNumber: The port number to register the ZMQ router socket used for receiving PylonGPS transmitter registrations/streams
@param inputAuthenticatedTransmitterRegistrationAndStreamingPortNumber: The port number to register the ZMQ router socket used for receiving authenticated PylonGPS transmitter registrations/streams
@param inputClientRequestPortNumber: The port to open to receive client requests (including requests from proxies for the list of sources)
@param inputClientStreamPublishingPortNumber: The port to open for the interface to publish stream data to clients
@param inputProxyStreamPublishingPortNumber: The port to open for the interface to publish stream data to proxies (potentially higher priority)
@param inputStreamStatusNotificationPortNumber: The port for the interface that is used to to publish stream status changes
@param inputCasterPublicKey: The public key for this caster to use for encryption/authentation
@param inputCasterSecretKey: The secret key for this caster to use for encryption/authentation
@param inputCasterSQLITEConnectionString: The connection string used to connect to or create the SQLITE database used for stream source entry management and query resolution.  If an empty string is given (by default), it will connect/create an in memory database with a random 64 bit hex number string (example: "file:jfuggekdai?mode=memory&cache=shared")

@throws: This function can throw exceptions
*/
caster::caster(zmq::context_t *inputContext, uint64_t inputCasterID, uint32_t inputTransmitterRegistrationAndStreamingPortNumber, uint32_t inputAuthenticatedTransmitterRegistrationAndStreamingPortNumber, uint32_t inputClientRequestPortNumber, uint32_t inputClientStreamPublishingPortNumber, uint32_t inputProxyStreamPublishingPortNumber, uint32_t inputStreamStatusNotificationPortNumber, const std::string &inputCasterPublicKey, const std::string &inputCasterSecretKey, const std::string &inputCasterSQLITEConnectionString)
{
if(inputContext == nullptr)
{
throw SOMException("Invalid ZMQ context\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Check that keys are the right size
if((inputCasterPublicKey.size() != 32 && inputCasterPublicKey.size() != 40) || (inputCasterSecretKey.size() != 32 && inputCasterSecretKey.size() != 40))
{
throw SOMException("Invalid ZMQ key(s)\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Save given parameters
context = inputContext;
casterID = inputCasterID;
transmitterRegistrationAndStreamingPortNumber = inputTransmitterRegistrationAndStreamingPortNumber;
authenticatedTransmitterRegistrationAndStreamingPortNumber = inputAuthenticatedTransmitterRegistrationAndStreamingPortNumber;
clientRequestPortNumber = inputClientRequestPortNumber;
clientStreamPublishingPortNumber = inputClientStreamPublishingPortNumber;
proxyStreamPublishingPortNumber = inputProxyStreamPublishingPortNumber;
streamStatusNotificationPortNumber = inputStreamStatusNotificationPortNumber;
casterPublicKey = inputCasterPublicKey;
casterSecretKey = inputCasterSecretKey;

//Set database connection string 
if(inputCasterSQLITEConnectionString == "")
{
//Generate random 64 bit unsigned int
//std::random_device randomnessSource;
//std::uniform_int_distribution<uint64_t> distribution;
//uint64_t connectionInteger = distribution(randomnessSource);

//databaseConnectionString = "file:" + std::to_string(connectionInteger) + "?mode=memory&cache=shared";  //Apparently Ubuntu has Poco 1.3 in its repositories, which doesn't support the more recent versions of SQLite which permit URI connection strings, so the default must be an actual file name or else mutiple connections cannot be made, which prevents more than one instance of this object being active at a time, so an alternate string and architecture must be used
databaseConnectionString = ":memory:";
}
else
{
databaseConnectionString = inputCasterSQLITEConnectionString;
}

//Attempt to connect to database and maintain connection as long as object exists so the database does not go out of scope before the object does
SOM_TRY
Poco::Data::SQLite::Connector::registerConnector();
SOM_CATCH("Error registering database connector")

SOM_TRY
databaseSession.reset(new Poco::Data::Session("SQLite", databaseConnectionString));
SOM_CATCH("Error creating/connecting to database\n")

//Drop the stream entry tables if they already exist.
SOM_TRY
(*databaseSession) << "BEGIN TRANSACTION; DROP TABLE IF EXISTS base_station_streams; DROP TABLE IF EXISTS base_station_signing_keys; END TRANSACTION;", Poco::Data::Keywords::now;
SOM_CATCH("Error dropping database tables during initialization")

//Recreate the tables without any entries
SOM_TRY
(*databaseSession) << "BEGIN TRANSACTION; CREATE TABLE base_station_streams (base_station_id integer primary key, latitude real, longitude real, expected_update_rate real, message_format integer, informal_name text, source_public_key text, start_time integer); CREATE TABLE base_station_signing_keys (signing_key text, base_station_stream_id integer, FOREIGN KEY(base_station_stream_id) REFERENCES base_station_streams(base_station_id) ON DELETE CASCADE); CREATE INDEX base_station_signing_keys_index ON base_station_signing_keys(base_station_stream_id);  END TRANSACTION;" << Poco::Data::Keywords::now;
SOM_CATCH("Error creating tables for caster\n")

//Initialize prepared statements
//SOM_TRY
//insertSimpleBaseStationDataIntoDatabasePreparedStatement.reset(new Poco::Data::Statement(*databaseSession));
//SOM_CATCH("Error initializing prepared statement")

//SOM_TRY
//(*insertSimpleBaseStationDataIntoDatabasePreparedStatement) << "INSERT INTO base_station_streams VALUES (?, ?, ?, ?, ?, ?, ?, ?)", use(std::get<0>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<1>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<2>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<3>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<4>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<5>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<6>(insertSimpleBaseStationDataIntoDatabaseTuple)), use(std::get<7>(insertSimpleBaseStationDataIntoDatabaseTuple)));
//SOM_CATCH("Error preparing statement\n")


//Initialize and bind shutdown socket
SOM_TRY
shutdownPublishingSocket.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing shutdownPublishingSocket\n")

int extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(shutdownPublishingConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*shutdownPublishingSocket, "shutdownPublishingSocketAddress");
SOM_CATCH("Error binding shutdownPublishingSocket\n")

//Initialize, connect and subscribe sockets which listen for shutdown signal
std::vector<std::unique_ptr<zmq::socket_t> *> signalListeningSockets = {&clientRequestHandlingThreadShutdownListeningSocket, &streamRegistrationAndPublishingThreadShutdownListeningSocket, &authenticationIDCheckingThreadShutdownListeningSocket, &statisticsGatheringThreadShutdownListeningSocket};

for(int i=0; i<signalListeningSockets.size(); i++)
{
SOM_TRY //Make socket
(*signalListeningSockets[i]).reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error creating socket to listen for shutdown signal\n")

SOM_TRY //Connect to hear signal
(*signalListeningSockets[i])->connect(shutdownPublishingConnectionString.c_str());
SOM_CATCH("Error connecting socket to listen for shutdown signal\n")

SOM_TRY //Set filter to allow any published messages to be received
(*signalListeningSockets[i])->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for socket listening for shutdown signal\n")
}

//Initialize and bind database access socket
SOM_TRY
databaseAccessSocket.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing shutdownPublishingSocket\n")

SOM_TRY //Bind to an dynamically generated address
std::tie(databaseAccessConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*databaseAccessSocket, "databaseAccessAddress");
SOM_CATCH("Error binding databaseAccessSocket\n")

//Attempt to register socket for authentication ID verification via ZAP protocol (could already be registered by another object)
SOM_TRY
ZAPAuthenticationSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing ZAPAuthenticationSocket\n")

SOM_TRY
try
{
ZAPAuthenticationSocket->bind("inproc://zeromq.zap.01");
}
catch(const zmq::error_t &inputError)
{
if(inputError.num() == EADDRINUSE)
{
ZAPAuthenticationSocket.reset(nullptr); //There is already a ZAP handler, so we don't need one 
}
else
{
throw; //It was some other error, so pass the exception along
}
}
SOM_CATCH("Error binding ZAP inproc socket\n")

//Initialize and bind transmitterRegistrationAndStreaming socket
SOM_TRY
transmitterRegistrationAndStreamingInterface.reset(new zmq::socket_t(*(context), ZMQ_ROUTER));
SOM_CATCH("Error intializing transmitterRegistrationAndStreamingInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(transmitterRegistrationAndStreamingPortNumber);
transmitterRegistrationAndStreamingInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding transmitterRegistrationAndStreamingInterface\n")

//Initialize and bind authenticatedTransmitterRegistrationAndStreamingPortNumber socket
SOM_TRY
authenticatedTransmitterRegistrationAndStreamingInterface.reset(new zmq::socket_t(*(context), ZMQ_ROUTER));
SOM_CATCH("Error intializing authenticatedTransmitterRegistrationAndStreamingInterface\n")

SOM_TRY //Set socket mode to CURVE server for authentication/encryption
int serverRole = 1;
authenticatedTransmitterRegistrationAndStreamingInterface->setsockopt(ZMQ_CURVE_SERVER, &serverRole, sizeof(serverRole));
SOM_CATCH("Error setting authenticatedTransmitterRegistrationAndStreamingInterface role\n")

SOM_TRY //Set secret key for socket
authenticatedTransmitterRegistrationAndStreamingInterface->setsockopt(ZMQ_CURVE_SECRETKEY, casterSecretKey.c_str(), casterSecretKey.size());
SOM_CATCH("Error setting secret key for caster\n")

SOM_TRY //Bind port for socket
std::string bindingAddress = "tcp://*:" + std::to_string(authenticatedTransmitterRegistrationAndStreamingPortNumber);
transmitterRegistrationAndStreamingInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding transmitterRegistrationAndStreamingInterface\n")

//Initialize and bind clientRequestInterface socket
SOM_TRY
clientRequestInterface.reset(new zmq::socket_t(*(context), ZMQ_ROUTER));
SOM_CATCH("Error intializing clientRequestInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(clientRequestPortNumber);
clientRequestInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding clientRequestInterface\n")

//Initialize and bind clientStreamPublishingInterface socket
SOM_TRY
clientStreamPublishingInterface.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing clientStreamPublishingInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(clientStreamPublishingPortNumber);
clientStreamPublishingInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding clientStreamPublishingInterface\n")

//Initialize and bind proxyStreamPublishingInterface socket
SOM_TRY
proxyStreamPublishingInterface.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing proxyStreamPublishingInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(proxyStreamPublishingPortNumber);
proxyStreamPublishingInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding proxyStreamPublishingInterface\n")

//Initialize and bind streamStatusNotificationInterface socket
SOM_TRY
streamStatusNotificationInterface.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing streamStatusNotificationInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(streamStatusNotificationPortNumber);
streamStatusNotificationInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding streamStatusNotificationInterface\n")

//Start threads
SOM_TRY
clientRequestHandlingThread.reset(new std::thread(&caster::clientRequestHandlingThreadFunction, this)); 
SOM_CATCH("Error initializing thread\n")

SOM_TRY
streamRegistrationAndPublishingThread.reset(new std::thread(&caster::streamRegistrationAndPublishingThreadFunction, this)); 
SOM_CATCH("Error initializing thread\n")

if(ZAPAuthenticationSocket.get() != nullptr)
{
SOM_TRY
authenticationIDCheckingThread.reset(new std::thread(&caster::authenticationIDCheckingThreadFunction, this)); 
SOM_CATCH("Error initializing thread\n")
}

SOM_TRY
statisticsGatheringThread.reset(new std::thread(&caster::statisticsGatheringThreadFunction, this)); 
SOM_CATCH("Error initializing thread\n")

}

/**
This function signals for the threads to shut down and then waits for them to do so.
*/
caster::~caster()
{
//Ensure that all exits points still unregister connector
SOMScopeGuard connectorGuard([](){ Poco::Data::SQLite::Connector::unregisterConnector();});

//Publish shutdown signal and wait for threads
try
{ //Send empty message to signal shutdown
SOM_TRY
shutdownPublishingSocket->send(nullptr, 0);
SOM_CATCH("Error sending shutdown signal for caster threads\n")
}
catch(const std::exception &inputException)
{
fprintf(stderr, "%s", inputException.what());
}

//Wait for threads to finish
if(authenticationIDCheckingThread.get() != nullptr)
{
authenticationIDCheckingThread->join(); 
}

clientRequestHandlingThread->join();
streamRegistrationAndPublishingThread->join();
statisticsGatheringThread->join();
}

/**
This function is called in the clientRequestHandlingThread to handle client requests and manage access to the SQLite database.
*/
void caster::clientRequestHandlingThreadFunction()
{
try
{
//Responsible for databaseAccessSocket, clientRequestInterface and clientRequestHandlingThreadShutdownListeningSocket


//Create priority queue for event queue and poll items, then start polling/event cycle
std::priority_queue<event> threadEventQueue;
std::unique_ptr<zmq::pollitem_t[]> pollItems;
int numberOfPollItems = 3;

SOM_TRY
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

//Populate the poll object list
pollItems[0] = {(void *) (*databaseAccessSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*clientRequestInterface), 0, ZMQ_POLLIN, 0};
pollItems[2] = {(void *) (*clientRequestHandlingThreadShutdownListeningSocket), 0, ZMQ_POLLIN, 0};


//Determine if an event has timed out (and deal with it if so) and then calculate the time until the next event timeout
Poco::Timestamp nextEventTime;
int64_t timeUntilNextEventInMilliseconds = 0;
while(true)
{
nextEventTime = handleEvents(threadEventQueue);

if(nextEventTime < 0)
{
timeUntilNextEventInMilliseconds = -1; //No events, so block until a message is received
}
else
{
timeUntilNextEventInMilliseconds = (nextEventTime - Poco::Timestamp())/1000 + 1; //Time in milliseconds till the next event, rounding up
}

//Poll until the next event timeout and resolve any messages that are received
SOM_TRY
if(zmq::poll(pollItems.get(), numberOfPollItems, timeUntilNextEventInMilliseconds) == 0)
{
continue; //Poll returned without indicating any messages have been received, so check events and go back to polling
}
SOM_CATCH("Error polling\n")

//Handle received messages

//Check if it is time to shutdown
if(pollItems[2].revents & ZMQ_POLLIN)
{
return; //Shutdown message received, so return
}

//Check if a database request has been received
if(pollItems[0].revents & ZMQ_POLLIN)
{//A database request has been received, so process it
//TODO: Handle database request
}

//Check if a client request has been received
if(pollItems[1].revents & ZMQ_POLLIN)
{//A client request has been received, so process it
//TODO: Handle client request
}

}

}
catch(const std::exception &inputException)
{ //If an exception is thrown, swallow it, send error message and terminate
fprintf(stderr, "clientRequestHandlingThreadFunction: %s\n", inputException.what());
return;
}

}

/**
This function is called in the streamRegistrationAndPublishingThread to handle stream registration and publishing updates.
*/
void caster::streamRegistrationAndPublishingThreadFunction()
{

}

/**
This function is called in the authenticationIDCheckingThread to verify if the ZMQ connection ID of authenticated connections matches the public key the connection is using.
*/
void caster::authenticationIDCheckingThreadFunction()
{

}

/**
This function monitors published updates, collects the associated statistics and periodically reports them to the database.
*/
void caster::statisticsGatheringThreadFunction()
{

}

/**
This function processes any events that are scheduled to have occurred by now and returns when the next event is scheduled to occur.  Which thread is calling this function is determined by the type of events in the event queue.
@param inputEventQueue: The event queue to process events from
@return: The time point associated with the soonest event timeout (negative if there are no outstanding events)

@throws: This function can throw exceptions
*/
Poco::Timestamp caster::handleEvents(std::priority_queue<pylongps::event> &inputEventQueue)
{

while(true)
{//Process an event if its time is less than the current timestamp

if(inputEventQueue.size() == 0)
{//No events left, so return negative
return Poco::Timestamp(-1);
}

if(inputEventQueue.top().time > Poco::Timestamp() )
{ //Next event isn't happening yet
return inputEventQueue.top().time;
}

//There is an event to process 
//TODO: Process events 

}

}

/**
This function checks if the databaseAccessSocket has received a database_request message and (if so) processes the message and sends a database_reply in response.
@throws: This function can throw exceptions
*/
void caster::processDatabaseRequest()
{

//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(databaseAccessSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving server registration/deregistration message")

//Create lambda to make it easy to send request failed replies
auto sendReplyLambda = [&] (bool inputRequestFailed, enum database_request_failure_reason inputReason = DATABASE_REQUEST_DESERIALIZATION_FAILED)
{
database_reply reply;
std::string serializedReply;

if(inputRequestFailed)
{ //Only set the reason if the request failed
reply.set_reason(inputReason);
}
reply.SerializeToString(&serializedReply);

SOM_TRY
databaseAccessSocket->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Attempt to deserialize
database_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendReplyLambda(true, DATABASE_REQUEST_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}

//Perform database operation
//TODO: Finish code to perform database operations


//Send reply

}

/**
This function checks if the clientRequestInterface has received a client_query_request message and (if so) processes the message and sends a client_query_reply in response.
@throws: This function can throw exceptions
*/
void caster::processClientQueryRequest()
{
}
