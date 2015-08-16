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
@param inputKeyRegistrationAndRemovalPortNumber: The port for the interface that is used to add/remove 
@param inputCasterPublicKey: The public key for this caster to use for encryption/authentation
@param inputCasterSecretKey: The secret key for this caster to use for encryption/authentation
@param inputSigningKeysManagementKey: The signing key that has permission to add/remove allowed signing keys from the caster
@param inputOfficialSigningKeys: A list of the initial set of approved keys for official basestations
@param inputRegisteredCommunitySigningKeys: A list of the initial set of approved keys for registered community basestations
@param inputBlacklistedKeys: A list of signing keys not to trust 
@param inputCasterSQLITEConnectionString: The connection string used to connect to or create the SQLITE database used for stream source entry management and query resolution.  If an empty string is given (by default), it will connect/create an in memory database with a random 64 bit number string (example: "file:9735926149617295559?mode=memory&cache=shared")

@throws: This function can throw exceptions
*/
caster::caster(zmq::context_t *inputContext, int64_t inputCasterID, uint32_t inputTransmitterRegistrationAndStreamingPortNumber, uint32_t inputAuthenticatedTransmitterRegistrationAndStreamingPortNumber, uint32_t inputClientRequestPortNumber, uint32_t inputClientStreamPublishingPortNumber, uint32_t inputProxyStreamPublishingPortNumber, uint32_t inputStreamStatusNotificationPortNumber, uint32_t inputKeyRegistrationAndRemovalPortNumber, const std::string &inputCasterPublicKey, const std::string &inputCasterSecretKey, const std::string &inputSigningKeysManagementKey, const std::vector<std::string> &inputOfficialSigningKeys, const std::vector<std::string> &inputRegisteredCommunitySigningKeys, const std::vector<std::string> &inputBlacklistedKeys, const std::string &inputCasterSQLITEConnectionString) : databaseConnection(nullptr, &sqlite3_close_v2)
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

//Check key lengths and place the keys in the set
if(inputSigningKeysManagementKey.size() != crypto_sign_PUBLICKEYBYTES)
{
throw SOMException("Master signing key is incorrect length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
signingKeysManagementKey = inputSigningKeysManagementKey;

for(int i=0; i<inputOfficialSigningKeys.size(); i++)
{
if(inputOfficialSigningKeys[i].size() != crypto_sign_PUBLICKEYBYTES)
{
throw SOMException("One of the official signing keys is incorrect length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
officialSigningKeys.insert(inputOfficialSigningKeys[i]);
}

for(int i=0; i<inputRegisteredCommunitySigningKeys.size(); i++)
{
if(inputRegisteredCommunitySigningKeys[i].size() != crypto_sign_PUBLICKEYBYTES)
{
throw SOMException("One of the registered community signing keys is incorrect length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
registeredCommunitySigningKeys.insert(inputRegisteredCommunitySigningKeys[i]);
}

for(int i=0; i<inputBlacklistedKeys.size(); i++)
{
if(inputBlacklistedKeys[i].size() != crypto_sign_PUBLICKEYBYTES)
{
throw SOMException("One of the signing keys in the blacklist is a incorrect length\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
blacklistedSigningKeys.insert(inputBlacklistedKeys[i]);
}


//Set database connection string 
if(inputCasterSQLITEConnectionString == "")
{
//Generate random 64 bit unsigned int
std::random_device randomnessSource;
std::uniform_int_distribution<uint64_t> distribution;
uint64_t connectionInteger = distribution(randomnessSource);

databaseConnectionString = "file:" + std::to_string(connectionInteger) + "?mode=memory&cache=shared";
}
else
{
databaseConnectionString = inputCasterSQLITEConnectionString;
}

//Attempt to connect to the database
sqlite3 *databaseConnectionBuffer = nullptr;
if(sqlite3_open_v2(databaseConnectionString.c_str(), &databaseConnectionBuffer, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI,NULL) != SQLITE_OK)
{
throw SOMException("Unable to open database connection\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
databaseConnection.reset(databaseConnectionBuffer); //Transfer ownership so that connection will be closed when object goes out of scope

//Make sure foreign key relationshcrypto_sign_PUBLICKEYBYTESips will be honored
if(sqlite3_exec(databaseConnection.get(), "PRAGMA foreign_keys = on;", NULL, NULL, NULL) != SQLITE_OK)
{
throw SOMException("Error enabling foreign keys\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Load custom SQLite functions (sin, cos, acos) for great circle calculations
if(sqlite3_create_function(databaseConnection.get(), "sin",1, SQLITE_UTF8, NULL, &pylongps::SQLiteSinFunctionDegrees, NULL, NULL ) != SQLITE_OK)
{
throw SOMException("Error adding SQLite3 function\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
if(sqlite3_create_function(databaseConnection.get(), "cos",1, SQLITE_UTF8, NULL, &pylongps::SQLiteCosFunctionDegrees, NULL, NULL ) != SQLITE_OK)
{
throw SOMException("Error adding SQLite3 function\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
if(sqlite3_create_function(databaseConnection.get(), "acos",1, SQLITE_UTF8, NULL, &pylongps::SQLiteAcosFunctionDegrees, NULL, NULL ) != SQLITE_OK)
{
throw SOMException("Error adding SQLite3 function\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Setup converter so that queries can be processed
SOM_TRY
setupBaseStationToSQLInterface();
SOM_CATCH("Error setting up basestationToSQLInterface\n")


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
databaseAccessSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing databaseAccessSocket\n")

SOM_TRY //Bind to an dynamically generated address
std::tie(databaseAccessConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*databaseAccessSocket, "databaseAccessAddress");
SOM_CATCH("Error binding databaseAccessSocket\n")

//Initialize and connect the registration thread's database request socket
SOM_TRY
registrationDatabaseRequestSocket.reset(new zmq::socket_t(*(context), ZMQ_DEALER));
SOM_CATCH("Error intializing registrationDatabaseRequestSocket\n")

SOM_TRY
registrationDatabaseRequestSocket->connect(databaseAccessConnectionString.c_str());
SOM_CATCH("Error connecting registration database request socket")

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

//Initialize and bind keyRegistrationAndRemoval socket
SOM_TRY
keyRegistrationAndRemovalInterface.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing keyRegistrationAndRemovalInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(inputKeyRegistrationAndRemovalPortNumber);
keyRegistrationAndRemovalInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding keyRegistrationAndRemovalInterface\n")




//Initialize and bind clientRequestInterface socket
SOM_TRY
clientRequestInterface.reset(new zmq::socket_t(*(context), ZMQ_REP));
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
//SOMScopeGuard connectorGuard([](){ Poco::Data::SQLite::Connector::unregisterConnector();});

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
SOM_TRY
processDatabaseRequest();
SOM_CATCH("Error processing database request\n")
}

//Check if a client request has been received
if(pollItems[1].revents & ZMQ_POLLIN)
{//A client request has been received, so process it
SOM_TRY
processClientQueryRequest();
SOM_CATCH("Error processing request\n")
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
try
{
//Responsible for streamRegistrationAndPublishingThreadShutdownListeningSocket, authenticatedTransmitterRegistrationAndStreamingInterface, transmitterRegistrationAndStreamingInterface, registrationDatabaseRequestSocket, keyRegistrationAndRemovalInterface
//Publishes to clientStreamPublishingInterface, proxyStreamPublishingInterface, streamStatusNotificationInterface

//Create priority queue for event queue and poll items, then start polling/event cycle
std::priority_queue<event> threadEventQueue;
std::unique_ptr<zmq::pollitem_t[]> pollItems;
int numberOfPollItems = 5;

SOM_TRY
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

//Populate the poll object list
pollItems[0] = {(void *) (*streamRegistrationAndPublishingThreadShutdownListeningSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*transmitterRegistrationAndStreamingInterface), 0, ZMQ_POLLIN, 0};
pollItems[2] = {(void *) (*authenticatedTransmitterRegistrationAndStreamingInterface), 0, ZMQ_POLLIN, 0};
pollItems[3] = {(void *) (*registrationDatabaseRequestSocket), 0, ZMQ_POLLIN, 0};
pollItems[4] = {(void *) (*keyRegistrationAndRemovalInterface), 0, ZMQ_POLLIN, 0};


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
if(pollItems[0].revents & ZMQ_POLLIN)
{
return; //Shutdown message received, so return
}

//Check if an unauthenticated registration or stream update has been received
if(pollItems[1].revents & ZMQ_POLLIN)
{//A message has been received, so process it (unauthenticated)
SOM_TRY
processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage(threadEventQueue, false);
SOM_CATCH("Error processing registration request\n")
}

//Check if a key status change request has been received 
if(pollItems[4].revents & ZMQ_POLLIN)
{//A message has been received, so process it
SOM_TRY
processKeyManagementRequest(threadEventQueue);
SOM_CATCH("Error processing request\n")
}

//Check if an authenticated registration or stream update has been received
if(pollItems[2].revents & ZMQ_POLLIN)
{//A message has been received, so process it (authenticated)
SOM_TRY
processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage(threadEventQueue, true);
SOM_CATCH("Error processing registration request\n")
}

//Check if we've received a reply from the database server
if(pollItems[3].revents & ZMQ_POLLIN)
{//A message has been received, so process it
SOM_TRY
processTransmitterRegistrationAndStreamingDatabaseReply();
SOM_CATCH("Error processing request\n")
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
This function is called in the authenticationIDCheckingThread to verify if the ZMQ connection ID of authenticated connections matches the public key the connection is using.
*/
void caster::authenticationIDCheckingThreadFunction()
{

try
{
//Responsible for ZAPAuthenticationSocket, authenticationIDCheckingThreadShutdownListeningSocket

//Create priority queue for event queue and poll items, then start polling/event cycle
std::priority_queue<event> threadEventQueue;
std::unique_ptr<zmq::pollitem_t[]> pollItems;
int numberOfPollItems = 2;

SOM_TRY
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

//Populate the poll object list
pollItems[0] = {(void *) (*ZAPAuthenticationSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*authenticationIDCheckingThreadShutdownListeningSocket), 0, ZMQ_POLLIN, 0};

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
if(pollItems[1].revents & ZMQ_POLLIN)
{
return; //Shutdown message received, so return
}

//Check if a database request has been received
if(pollItems[0].revents & ZMQ_POLLIN)
{//A ZAP authentication message (http://rfc.zeromq.org/spec:27) has been received, so process it (ZMQ identity must contain the 32 byte public key in the first 32 bytes of the identity, so that the key credentials can be checked at the router socket). 
SOM_TRY
processZAPAuthenticationRequest();
SOM_CATCH("Error processing database request\n")
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

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();

if(inputEventQueue.top().time > currentTime )
{ //Next event isn't happening yet
return inputEventQueue.top().time;
}

//There is an event to process
event eventToProcess = inputEventQueue.top();
inputEventQueue.pop(); //Remove event from queue


//Process events
if(eventToProcess.HasExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field))
{ //possible_base_station_event_timeout
possible_base_station_event_timeout eventInstance = eventToProcess.GetExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field);

std::map<std::string, connectionStatus> *mapPointer = nullptr;
if(eventInstance.is_authenticated())
{
mapPointer = &authenticatedConnectionIDToConnectionStatus;
}
else
{
mapPointer = &unauthenticatedConnectionIDToConnectionStatus;
}


if((mapPointer->at(eventInstance.connection_id()).timeLastMessageWasReceived.epochMicroseconds() + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0) <= eventToProcess.time.epochMicroseconds())
{//It has been more than SECONDS_BEFORE_CONNECTION_TIMEOUT since a message was received, so drop connection
if(eventInstance.is_authenticated())
{
SOM_TRY //Remove from database
removeAuthenticatedConnection(eventInstance.connection_id());
SOM_CATCH("Error, unable to remove authenticated connection\n")
continue;
}
else
{
SOM_TRY
removeUnauthenticatedConnection(eventInstance.connection_id());
SOM_CATCH("Error, unable to remove unauthenticated connection\n")
}
}

}
 
if(eventToProcess.HasExtension(blacklist_key_timeout_event::blacklist_key_timeout_event_field))
{ //Blacklist entry timed out, so simply remove the key from the blacklist
blacklistedSigningKeys.erase(eventToProcess.GetExtension(blacklist_key_timeout_event::blacklist_key_timeout_event_field).blacklist_key());
continue;
}

if(eventToProcess.HasExtension(connection_key_timeout_event::connection_key_timeout_event_field))
{ //A connection key has timed out, so remove it
SOM_TRY
removeConnectionKey(eventToProcess.GetExtension(connection_key_timeout_event::connection_key_timeout_event_field).connection_key());
SOM_CATCH("Error removing connection key\n")
continue;
}

if(eventToProcess.HasExtension(signing_key_timeout_event::signing_key_timeout_event_field))
{ //A signing key has timed out, so remove it
SOM_TRY
removeSigningKey(eventToProcess.GetExtension(signing_key_timeout_event::signing_key_timeout_event_field).key());
SOM_CATCH("Error removing signing key\n")
continue;
}

}

}

/**
This function adds a authenticated connection by placing it in the associated maps/sets and the database.  The call is ignored if the connection key is not found in connectionKeyToSigningKeys, so addConnectionKey should have been called first for the connection key.
@param inputConnectionID: The ZMQ connection ID string of the connection to add
@param inputConnectionKey: The ZMQ CURVE key that is being used with this connection (connection key)
@param inputConnectionStatus: The current status of the connection
@param inputBaseStationStreamInfo: The connection's details to register with the database
@param inputEventQueue: The queue to register the timeout associated with this connection (close if idle for X seconds).

@throws: This function can throw exceptions
*/
void caster::addAuthenticatedConnection(const std::string &inputConnectionID,  const std::string &inputConnectionKey, const connectionStatus &inputConnectionStatus, const base_station_stream_information &inputBaseStationStreamInfo, std::priority_queue<event> &inputEventQueue)
{
if(connectionKeyToSigningKeys.count(inputConnectionID))
{
return; //Connection key entry not found, so the connection cannot be registered
}

//Add to maps/sets
authenticatedConnectionIDToConnectionKey.emplace(inputConnectionID, inputConnectionKey);
connectionKeyToAuthenticatedConnectionIDs.emplace(inputConnectionKey, inputConnectionID);
authenticatedConnectionIDToConnectionStatus[inputConnectionID] = inputConnectionStatus;

//Send registration request to database
std::string serializedDatabaseRequest;
database_request databaseRequest;
(*databaseRequest.mutable_base_station_to_register()) = inputBaseStationStreamInfo;
databaseRequest.set_registration_connection_id(inputConnectionID);

databaseRequest.SerializeToString(&serializedDatabaseRequest);

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
registrationDatabaseRequestSocket->send(serializedDatabaseRequest.c_str(), serializedDatabaseRequest.size());
SOM_CATCH("Error sending database request\n")

//Add timeout event to queue
Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();

possible_base_station_event_timeout timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_id(inputConnectionID);
timeoutEventSubMessage.set_is_authenticated(true);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}

/**
This function removes a authenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove

@throws: This function can throw exceptions
*/
void caster::removeAuthenticatedConnection(const std::string &inputConnectionID)
{
//Check if it has already been erased
if(authenticatedConnectionIDToConnectionKey.count(inputConnectionID) == 0)
{
return;
}

//Remove from maps/sets
std::string connectionKey(authenticatedConnectionIDToConnectionKey.at(inputConnectionID));
authenticatedConnectionIDToConnectionKey.erase(inputConnectionID);

removeKeyValuePairFromStringMultimap(connectionKeyToAuthenticatedConnectionIDs, connectionKey, inputConnectionID);

auto basestationID = authenticatedConnectionIDToConnectionStatus.at(inputConnectionID).baseStationID;
authenticatedConnectionIDToConnectionStatus.erase(inputConnectionID);

//Remove from database
std::string serializedDatabaseRequest;
database_request databaseRequest;
databaseRequest.set_delete_base_station_id(basestationID);

databaseRequest.SerializeToString(&serializedDatabaseRequest);

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
registrationDatabaseRequestSocket->send(serializedDatabaseRequest.c_str(), serializedDatabaseRequest.size());
SOM_CATCH("Error sending database request\n")
}

/**
This function adds a connection signing key, updates the associated maps and schedules it to timeout if it has a timeout time.  Only signing keys which are already present will be acknowledged.
@param inputConnectionKey: The connection key to add
@param inputExpirationTime: The timestamp of when this key expires (negative if it does not expire on its own)
@param inputSigningKeys: The keys which have signed this 
@param inputEventQueue: The event queue to add the timeout event to

@return: true if the key had valid signing keys and was added
*/
bool caster::addConnectionKey(const std::string &inputConnectionKey, int64_t inputExpirationTime, const std::vector<std::string> &inputSigningKeys, std::priority_queue<event> &inputEventQueue)
{
//Check if any of the signing keys are considered valid
std::set<std::string> listOfOfficialSigningKeys;
for(int i=0; i<inputSigningKeys.size(); i++)
{
if(officialSigningKeys.count(inputSigningKeys[i]) > 0)
{
listOfOfficialSigningKeys.insert(inputSigningKeys[i]);
}
}

std::set<std::string> listOfRegisteredCommunitySigningKeys;
if(listOfOfficialSigningKeys.size() == 0)
{
for(int i=0; i<inputSigningKeys.size(); i++)
{
if(registeredCommunitySigningKeys.count(inputSigningKeys[i]) > 0)
{
listOfRegisteredCommunitySigningKeys.insert(inputSigningKeys[i]);
}
}
}

if(listOfOfficialSigningKeys.size() && listOfRegisteredCommunitySigningKeys.size() == 0)
{
return false; //Doesn't have a valid signing key
}

std::set<std::string> *set = nullptr;
if(listOfOfficialSigningKeys.size() > 0)
{ //Official 
set = &listOfOfficialSigningKeys;
}
else
{ //Registered community
set = &listOfRegisteredCommunitySigningKeys;
}

//Add to maps
for(auto iter = set->begin(); iter!=set->end(); iter++)
{
signingKeyToConnectionKeys.emplace(*iter, inputConnectionKey);
connectionKeyToSigningKeys.emplace(inputConnectionKey, *iter);
}

//Add timeout event to queue
if(inputExpirationTime >= 0)
{
connection_key_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_key(inputConnectionKey);

event timeoutEvent(inputExpirationTime);
(*timeoutEvent.MutableExtension(connection_key_timeout_event::connection_key_timeout_event_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}

return true;
}

/**
This function removes the given connection key from the maps and removes all associated connections.
@param inputConnectionKey: The connection key to remove

@throws: This function can throw exceptions
*/
void caster::removeConnectionKey(const std::string &inputConnectionKey)
{
//Remove/delete all affiliated connections
auto equal_range = connectionKeyToAuthenticatedConnectionIDs.equal_range(inputConnectionKey);
for(auto iter = equal_range.first; iter != equal_range.second;)
{
auto buffer = iter++; //Iter's entry gets deleted, so we have to increment before that happens
SOM_TRY
removeAuthenticatedConnection(iter->second);
SOM_CATCH("Error removing authenticated connection\n")
iter = buffer; 
}

//Remove from maps
equal_range = connectionKeyToSigningKeys.equal_range(inputConnectionKey);
for(auto iter = equal_range.first; iter != equal_range.second;)
{
removeKeyValuePairFromStringMultimap(signingKeyToConnectionKeys, iter->second, inputConnectionKey);
}
connectionKeyToSigningKeys.erase(inputConnectionKey);
}

/**
This function adds a new signing key and schedules its timeout.
@param inputSigningKey: The signing key to add
@param inputIsOfficialSigningKey: True if the signing key is an "Official" one and false if it is "Registered Community"
@param inputExpirationTime: When the signing key becomes invalid
@param inputEventQueue: The event queue to add the timeout event to

@return: true if the key was added successfully or is already present
*/
bool caster::addSigningKey(const std::string &inputSigningKey, bool inputIsOfficialSigningKey, int64_t inputExpirationTime, std::priority_queue<event> &inputEventQueue)
{
if((officialSigningKeys.count(inputSigningKey) > 0 && inputIsOfficialSigningKey) || (registeredCommunitySigningKeys.count(inputSigningKey) > 0 && !inputIsOfficialSigningKey))
{
return true; //The key has already been added
}

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
if(inputExpirationTime < timeValue)
{
return false; //Key has already expired
}

if(inputIsOfficialSigningKey)
{
officialSigningKeys.insert(inputSigningKey);
}
else
{
registeredCommunitySigningKeys.insert(inputSigningKey);
}

//Add timeout event
signing_key_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_key(inputSigningKey);
timeoutEventSubMessage.set_is_official(inputIsOfficialSigningKey);

event timeoutEvent(inputExpirationTime);
(*timeoutEvent.MutableExtension(signing_key_timeout_event::signing_key_timeout_event_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}

/**
This function removes a signing key.  This can cause a cascade where a connection key which is reliant on it is removed, which can in turn cause many connections to be removed.
@param inputSigningKey: The signing key to remove

@throws: This function can throw exceptions
*/
void caster::removeSigningKey(const std::string &inputSigningKey)
{
//Remove any affected connection keys and delete all references to this key
auto equal_range = signingKeyToConnectionKeys.equal_range(inputSigningKey);
for(auto iter = equal_range.first; iter != equal_range.second;)
{
auto buffer = iter++; //Object refered to by iter will be removed
if(connectionKeyToSigningKeys.count(iter->second) < 2)
{//If this connection key is only signed by this signing key, remove it
SOM_TRY
removeConnectionKey(iter->second);
SOM_CATCH("Error removing signing key\n")
}
else
{
removeKeyValuePairFromStringMultimap(connectionKeyToSigningKeys, iter->second, inputSigningKey);
signingKeyToConnectionKeys.erase(iter);
}
iter = buffer;
} //Takes care of signingKeyToConnectionKeys, connectionKeyToSigningKeys

//Erase from both (won't do anything if not there)
officialSigningKeys.erase(inputSigningKey);
registeredCommunitySigningKeys.erase(inputSigningKey);
}

/**
Add a key to the blacklist, triggering its removal from "official" and "registered community" and the prevention of it from being reused until the certificate expires.
@param inputBlacklistKey: The key to place on the blacklist
@param inputExpirationTime: When the key expires
@param inputEventQueue: The queue to to ad the expiration event to

@throws: this function can throw exceptions
*/
void caster::addBlacklistKey(const std::string &inputBlacklistKey, int64_t inputExpirationTime, std::priority_queue<event> &inputEventQueue)
{
SOM_TRY //Remove the key from list of signing keys
removeSigningKey(inputBlacklistKey);
SOM_CATCH("Error removing blacklist key\n")

//Add to the list of blacklisted keys
blacklistedSigningKeys.insert(inputBlacklistKey);

//Add timeout event
blacklist_key_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_blacklist_key(inputBlacklistKey);

event timeoutEvent(inputExpirationTime);
(*timeoutEvent.MutableExtension(blacklist_key_timeout_event::blacklist_key_timeout_event_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}

/**
This function removes a unauthenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove

@throws: This function can throw exceptions
*/
void caster::removeUnauthenticatedConnection(const std::string &inputConnectionID)
{
//Check if it has already been erased
if(unauthenticatedConnectionIDToConnectionStatus.count(inputConnectionID) == 0)
{
return;
}

//Remove from maps/sets
auto basestationID = unauthenticatedConnectionIDToConnectionStatus.at(inputConnectionID).baseStationID;
unauthenticatedConnectionIDToConnectionStatus.erase(inputConnectionID);

//Remove from database
std::string serializedDatabaseRequest;
database_request databaseRequest;
databaseRequest.set_delete_base_station_id(basestationID);

databaseRequest.SerializeToString(&serializedDatabaseRequest);

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
registrationDatabaseRequestSocket->send(serializedDatabaseRequest.c_str(), serializedDatabaseRequest.size());
SOM_CATCH("Error sending database request\n")
}

/**
This function sets up the basestationToSQLInterface and generates the associated tables so that basestations can be stored and returned.  databaseConnection must be setup before this function is called.

@throws: This function can throw exceptions
*/
void caster::setupBaseStationToSQLInterface()
{
//Initialize object with database connection and table name
SOM_TRY
basestationToSQLInterface.reset(new protobufSQLConverter<base_station_stream_information>(databaseConnection.get(), "base_station_stream_information"));
SOM_CATCH("Error initializing basestationToSQLInterface\n")

//Register each of the fields with their associated database entries
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(base_station_stream_information, latitude, "latitude" ));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(base_station_stream_information, longitude, "longitude" ));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(base_station_stream_information, expected_update_rate, "expected_update_rate" ));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(base_station_stream_information, corrections_message_format, message_format, "message_format"));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(base_station_stream_information, informal_name, "informal_name"));

basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_ENUM_FIELD(base_station_stream_information, base_station_class, station_class, "station_class"));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(base_station_stream_information, base_station_id, "base_station_id"), true); //This field must be set in all all messages stored
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_STRING_FIELD(base_station_stream_information, source_public_key, "source_public_key"));
basestationToSQLInterface->addField(PYLON_GPS_GEN_REPEATED_STRING_FIELD(base_station_stream_information, signing_keys,  "signing_keys", "signing_key", "foreign_key"));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(base_station_stream_information, real_update_rate, "real_update_rate" ));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_DOUBLE_FIELD(base_station_stream_information, uptime, "uptime" ));
basestationToSQLInterface->addField(PYLON_GPS_GEN_OPTIONAL_INT64_FIELD(base_station_stream_information, start_time, "start_time"));

//Generate associated tables in database
SOM_TRY
basestationToSQLInterface->createTables();
SOM_CATCH("Error creating tables for basestationToSQLInterface")
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
auto sendReplyLambda = [&] (bool inputRequestFailed, enum database_request_failure_reason inputReason = DATABASE_REQUEST_DESERIALIZATION_FAILED, std::string inputConnectionString = std::string(""))
{
database_reply reply;
std::string serializedReply;

if(inputRequestFailed)
{ //Only set the reason if the request failed
reply.set_reason(inputReason);
}
if(inputConnectionString != "")
{
reply.set_registration_connection_id(inputConnectionString);
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

//Validate submessage
if(request.has_base_station_to_register())
{//Add basestation
std::string replyConnectionID;
if(request.has_registration_connection_id())
{
replyConnectionID = request.registration_connection_id();
}

if(!request.base_station_to_register().has_latitude() || !request.base_station_to_register().has_longitude() || !request.base_station_to_register().has_base_station_id() || !request.base_station_to_register().has_start_time() || !request.base_station_to_register().has_message_format())
{//Message did not have required fields, so send back message saying request failed
SOM_TRY
sendReplyLambda(true, DATABASE_REQUEST_FORMAT_INVALID, replyConnectionID);
return;
SOM_CATCH("Error sending reply")
}

//Perform database operation
SOM_TRY //Attempt to store basestation in database
basestationToSQLInterface->store((*request.mutable_base_station_to_register()));
SOM_CATCH("Error inserting basestation to database\n")
//Send reply
SOM_TRY
sendReplyLambda(false, DATABASE_REQUEST_FORMAT_INVALID, replyConnectionID); //Request succedded
SOM_CATCH("Error sending reply\n")
return;
}

if(request.has_delete_base_station_id() )
{//Perform delete operation
SOM_TRY
std::vector<::google::protobuf::int64> keysToDelete;
keysToDelete.push_back(request.delete_base_station_id());
basestationToSQLInterface->deleteObjects(keysToDelete);
SOM_CATCH("Error deleting from database\n")

SOM_TRY
sendReplyLambda(false); //Request succeeded
SOM_CATCH("Error sending reply\n")
return;
}

if(request.has_base_station_to_update_id() && request.has_real_update_rate())
{ //Perform update operation
SOM_TRY
basestationToSQLInterface->update(fieldValue((::google::protobuf::int64) request.base_station_to_update_id()), "real_update_rate", fieldValue(request.real_update_rate()));
SOM_CATCH("Error updating database\n")

SOM_TRY
sendReplyLambda(false); //Request succedded
SOM_CATCH("Error sending reply\n")
return;
}

SOM_TRY
sendReplyLambda(true, DATABASE_REQUEST_FORMAT_INVALID); //Request failed
SOM_CATCH("Error sending reply\n")
}

/**
This function checks if the clientRequestInterface has received a client_query_request message and (if so) processes the message and sends a client_query_reply in response.
@throws: This function can throw exceptions
*/
void caster::processClientQueryRequest()
{
//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(clientRequestInterface->recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving server registration/deregistration message")

//Create lambda to make it easy to send request failed replies
auto sendReplyLambda = [&] (bool inputRequestFailed, enum client_query_request_failure_reason inputReason = CLIENT_QUERY_REQUEST_DESERIALIZATION_FAILED, ::google::protobuf::int64 inputCasterID = 0,  std::vector<base_station_stream_information> inputBaseStations = std::vector<base_station_stream_information>(0))
{
client_query_reply reply;
std::string serializedReply;

if(inputRequestFailed)
{ //Only set the reason if the request failed
reply.set_failure_reason(inputReason);
}
else
{
reply.set_caster_id(inputCasterID);
for(int i=0; i<inputBaseStations.size(); i++)
{
auto newObjectPointer = reply.mutable_base_stations()->Add();
(*newObjectPointer) = (inputBaseStations[i]);
}
}
reply.SerializeToString(&serializedReply);

SOM_TRY
clientRequestInterface->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Attempt to deserialize
client_query_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendReplyLambda(true, CLIENT_QUERY_REQUEST_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}


//TODO: establish limits on query complexity

int boundParameterCount = 0;
std::string sqlQueryString;
SOM_TRY
sqlQueryString = generateClientQueryRequestSQLString(request, boundParameterCount);
SOM_CATCH("Error generating request sql string\n")

if(boundParameterCount > 999)
{ //Too many bound parameters to process
SOM_TRY
sendReplyLambda(true, CLIENT_QUERY_REQUEST_TOO_COMPLEX);
return;
SOM_CATCH("Error sending reply");
}


std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> clientQueryStatement(nullptr, &sqlite3_finalize);

SOM_TRY
basestationToSQLInterface->prepareStatement(clientQueryStatement, sqlQueryString);
SOM_CATCH("Error preparing query statement\n")

int bindingParameterCount = bindClientQueryRequestFields(clientQueryStatement, request);
if(bindingParameterCount != boundParameterCount)
{
throw SOMException("Invalid ZMQ context\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

int returnValue = sqlite3_step(clientQueryStatement.get());
//sqlite3_reset(clientQueryStatement.get()); //Reset so that statement can be used again
if(returnValue != SQLITE_DONE && returnValue != SQLITE_ROW)
{
throw SOMException("Error executing statement\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

//Check column number and type
if(sqlite3_column_count(clientQueryStatement.get()) !=1)
{
throw SOMException("Error, wrong number of result columns returned\n", SQLITE3_ERROR, __FILE__, __LINE__);
}



if(sqlite3_column_type(clientQueryStatement.get(), 0) != SQLITE_INTEGER && returnValue != SQLITE_DONE)
{
throw SOMException("Error, wrong type of result columns returned\n", SQLITE3_ERROR, __FILE__, __LINE__);
}


int stepReturnValue = returnValue;
std::vector<::google::protobuf::int64> resultPrimaryKeys;
while(true)
{
if(stepReturnValue == SQLITE_DONE)
{ //All results for this field have been retrieved
break; 
}

resultPrimaryKeys.push_back((::google::protobuf::int64) sqlite3_column_int(clientQueryStatement.get(), 0));

stepReturnValue = sqlite3_step(clientQueryStatement.get());

if(stepReturnValue != SQLITE_ROW && stepReturnValue != SQLITE_DONE)
{
throw SOMException("Error executing query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}


std::vector<base_station_stream_information> results;
SOM_TRY
results = basestationToSQLInterface->retrieve(resultPrimaryKeys);
SOM_CATCH("Error retrieving objects associated with query primary keys\n")



Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
for(int i=0; i<results.size(); i++)
{
results[i].set_uptime(timeValue-results[i].start_time());
}


SOM_TRY //Send back query results
sendReplyLambda(false, CLIENT_QUERY_REQUEST_DESERIALIZATION_FAILED, casterID, results);
SOM_CATCH("Error sending reply\n")
}

/**
This process checks if ZAPAuthenticationSocket has received a ZAP request.  If so, it checks to make sure that the first 32 bytes of the connection identity contains the public key in the credentials, so that the key associated with a connection can be checked using the identity at the authenticatedTransmitterRegistrationAndStreamingInterface using a credentials message sent over that connection

@throws: This function can throw exceptions
*/
void caster::processZAPAuthenticationRequest()
{
//Create lambda to make it easy to send replies
auto sendReplyLambda = [&] (const std::string &inputRequestID, int inputStatusCode)
{ //Reply
std::vector<std::string> contentToSend;
contentToSend.push_back(""); //0. Zero length frame
contentToSend.push_back("1.0"); //1. "1.0"
contentToSend.push_back(inputRequestID); //2. requestID (echoed)
contentToSend.push_back(std::to_string(inputStatusCode)); //3. status code -> "200" for success, "300" for temp error, "400" for auth failure, "500" for internal error
contentToSend.push_back(""); //4. Empty or custom error message
contentToSend.push_back(""); //5. Could contain metadata about user if status is "200", must be empty otherwise
contentToSend.push_back(""); //6. Empty or ZMQ format defined style metadata message 

for(int i=0; i<contentToSend.size(); i++)
{
SOM_TRY
ZAPAuthenticationSocket->send(contentToSend[i].c_str(), contentToSend[i].size());
SOM_CATCH("Error sending reply message\n")
}

};

std::vector<std::string> receivedContent; //Save all of the multi part message in a vector
auto sendSystemFailedReply = [&] () 
{
if(receivedContent.size() >= 3)
{
SOM_TRY
sendReplyLambda(receivedContent[1],500);
SOM_CATCH("Error sending reply\n")
}
else
{
SOM_TRY
sendReplyLambda("",500);
SOM_CATCH("Error sending reply\n")
}
};

std::unique_ptr<zmq::message_t> messageBuffer;

while(true)
{
SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive message
messageReceived = ZAPAuthenticationSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving server registration/deregistration message")

if(!messageReceived)
{
SOM_TRY
sendSystemFailedReply();
SOM_CATCH("Error sending reply\n")
}

receivedContent.push_back(std::string((const char *) messageBuffer->data(), messageBuffer->size()));

if(!messageBuffer->more())
{ //Multi-part message completed, so exit loop
break;
}

}

if(receivedContent.size() != 8)
{ //Received wrong number of messages
SOM_TRY
sendSystemFailedReply();
SOM_CATCH("Error sending reply\n")
}

//REP socket, should receive a 8 part message (http://rfc.zeromq.org/spec:27)
//Request:
//0. zero length frame
//1. "1.0"
//2. requestID (reply must echo)
//3. domain
//4. Origin IP address (IPv4 or IPv6 string)
//5. connectionIdentity (max 255 bytes)
//6. "CURVE"
//7. 32 byte long term public key
if(receivedContent[0] != "" || receivedContent[1] != "1.0" || receivedContent[5].size() > 255 || receivedContent[6] != "CURVE" || receivedContent[7].size() != 32 || receivedContent[2].size() < 32)
{
SOM_TRY //Send system failure message
sendReplyLambda(receivedContent[2],500); 
SOM_CATCH("Error sending reply\n")
}

if(receivedContent[2].find(receivedContent[7]) != 0)
{ //The identity does not have the key preappended, so it is invalid
SOM_TRY //Send system failure message
sendReplyLambda(receivedContent[2],400); //Authentication failure
SOM_CATCH("Error sending reply\n")
}

//Everything passed
SOM_TRY //Send system failure message
sendReplyLambda(receivedContent[2],200); //Authentication "succeeded"
SOM_CATCH("Error sending reply\n")
}

/**
This function processes messages from the either the  authenticatedTransmitterRegistrationAndStreamingInterface or the transmitterRegistrationAndStreamingInterface.  A connection is expected to start with a transmitter_registration_request, to which this object replies with a transmitter_registration_reply.  Thereafter, the messages received are forwarded to the associated publisher interfaces until the publisher stops sending for an unacceptably long period (SECONDS_BEFORE_CONNECTION_TIMEOUT), at which point the object erases the associated the associated metadata and publishes that the base station disconnected.  In the authenticated case, the permissions of the key associated with the connection are also checked.
@param inputEventQueue: The event queue to register events to
@param inputIsAuthenticated: True if the message is from the authenticated port

@throws: This function can throw exceptions
*/
void caster::processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage(std::priority_queue<event> &inputEventQueue, bool inputIsAuthenticated)
{
//Send reply
auto sendReplyLambda = [&] (const std::string &inputAddress, bool inputRequestSucceeded, zmq::socket_t *inputSocketToSendFrom, request_failure_reason inputFailureReason = MESSAGE_FORMAT_INVALID)
{
//Send: address, empty, data
transmitter_registration_reply reply;
std::string serializedReply;

reply.set_request_succeeded(inputRequestSucceeded);

if(!inputRequestSucceeded)
{ //Only set the reason if the request failed
reply.set_failure_reason(inputFailureReason);
}
reply.SerializeToString(&serializedReply);

SOM_TRY
inputSocketToSendFrom->send(inputAddress.c_str(), inputAddress.size(), ZMQ_SNDMORE);
SOM_CATCH("Error sending reply messages\n")

SOM_TRY
inputSocketToSendFrom->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply messages\n")
};

zmq::socket_t *sourceSocket = nullptr;
if(inputIsAuthenticated)
{
sourceSocket = authenticatedTransmitterRegistrationAndStreamingInterface.get();
}
else
{
sourceSocket = transmitterRegistrationAndStreamingInterface.get();
} 

std::map<std::string, connectionStatus> *associatedMap = nullptr;
if(inputIsAuthenticated)
{
associatedMap = &authenticatedConnectionIDToConnectionStatus;
}
else
{
associatedMap = &unauthenticatedConnectionIDToConnectionStatus;
} 


//Receive messages
std::vector<std::string> receivedContent;
bool messageRetrievalSuccessful = false;
SOM_TRY
messageRetrievalSuccessful = retrieveRouterMessage(*sourceSocket, receivedContent);
SOM_CATCH("Error retrieving router message\n")

if(!messageRetrievalSuccessful)
{ //Invalid message, so ignore
return;
}

std::string connectionID = receivedContent[0];
//Get current time
Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();


//See if this base station has been registered yet
if(associatedMap->count(connectionID) == 0)
{

//This connection has not been seen before, so it should have a transmitter_registration_request
//Attempt to deserialize
transmitter_registration_request request;
request.ParseFromArray(receivedContent[1].c_str(), receivedContent[1].size());

if(!request.IsInitialized())
{//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, MESSAGE_FORMAT_INVALID);
return;
SOM_CATCH("Error sending reply");
}

base_station_stream_information *streamInfo = request.mutable_stream_info();

if(!streamInfo->has_message_format())
{//Missing a required field
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, MISSING_REQUIRED_FIELD);
return;
SOM_CATCH("Error sending reply")
}


//Check permissions if authenticated
bool credentialsMessageIsValid = false;
bool signedByRecognizedOfficialKey = false;
bool signedByRecognizedCommunityKey = false;
authorized_permissions permissionsBuffer;
if(inputIsAuthenticated)
{
std::tie(credentialsMessageIsValid, signedByRecognizedOfficialKey, signedByRecognizedCommunityKey) = checkCredentials(*request.mutable_transmitter_credentials(), permissionsBuffer);

if(!credentialsMessageIsValid || connectionID.find(permissionsBuffer.public_key()) != 0)
{ //Either authorized_permissions could not be deserialized or one of the signatures didn't match or the key didn't match
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, CREDENTIALS_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}



if(permissionsBuffer.has_valid_until())
{ //Permissions recognized but expired
if(timeValue > permissionsBuffer.valid_until())
{
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, CREDENTIALS_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}
}
else
{
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, CREDENTIALS_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}

if(connectionID.find(permissionsBuffer.public_key()) != 0 || permissionsBuffer.public_key().size() != 32) //ZMQ key size
{ //The connection ID doesn't match the certificate key
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, CREDENTIALS_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}

//Connection appears to be valid, so add it
auto credentialsPointer = *request.mutable_transmitter_credentials();
std::vector<std::string> signingKeys;

for(int i=0; i<credentialsPointer.signatures_size(); i++)
{
signingKeys.push_back(credentialsPointer.signatures(i).public_key());
}

//Add connection key
if(addConnectionKey(permissionsBuffer.public_key(), permissionsBuffer.valid_until(), signingKeys, inputEventQueue) != true)
{
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, INSUFFICIENT_PERMISSIONS);
return;
SOM_CATCH("Error sending reply");
}
}


//Set caster assigned fields
if(!inputIsAuthenticated)
{
streamInfo->set_station_class(COMMUNITY);
}
else
{
if(signedByRecognizedOfficialKey)
{
streamInfo->set_station_class(OFFICIAL);
}
else if(signedByRecognizedCommunityKey)
{
streamInfo->set_station_class(REGISTERED_COMMUNITY);
}
else
{
SOM_TRY
sendReplyLambda(connectionID, false, sourceSocket, INSUFFICIENT_PERMISSIONS);
return;
SOM_CATCH("Error sending reply");
}
}

streamInfo->clear_signing_keys();
if(inputIsAuthenticated)
{ //Add signing keys
for(int i=0; i<request.mutable_transmitter_credentials()->signatures_size(); i++)
{
streamInfo->add_signing_keys(request.mutable_transmitter_credentials()->signatures(i).public_key());
}
}

auto streamID = getNewStreamID();
streamInfo->set_base_station_id(streamID); //Create/assign new stream id
streamInfo->clear_source_public_key();
streamInfo->clear_real_update_rate();
streamInfo->clear_uptime();
streamInfo->set_start_time(timeValue);

//Make/send request to database
if(!inputIsAuthenticated)
{
std::string serializedDatabaseRequest;
database_request databaseRequest;
(*databaseRequest.mutable_base_station_to_register()) = (*streamInfo);
databaseRequest.set_registration_connection_id(connectionID);

databaseRequest.SerializeToString(&serializedDatabaseRequest);

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
registrationDatabaseRequestSocket->send(serializedDatabaseRequest.c_str(), serializedDatabaseRequest.size());
SOM_CATCH("Error sending database request\n")
}

//Add to map
connectionStatus associatedConnectionStatus;
associatedConnectionStatus.requestToTheDatabaseHasBeenSent = true;
associatedConnectionStatus.baseStationID = streamID;
associatedConnectionStatus.timeLastMessageWasReceived = timeValue;
if(inputIsAuthenticated)
{
SOM_TRY
addAuthenticatedConnection(connectionID, permissionsBuffer.public_key(), associatedConnectionStatus, *streamInfo, inputEventQueue);
SOM_CATCH("Error adding authenticated connection\n")
}
else
{
unauthenticatedConnectionIDToConnectionStatus[connectionID] = associatedConnectionStatus;
}


if(!inputIsAuthenticated)
{
//Register possible stream timeout event
possible_base_station_event_timeout timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_id(connectionID);
timeoutEventSubMessage.set_is_authenticated(inputIsAuthenticated);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}


//Tell base station that its registration succeeded
SOM_TRY
sendReplyLambda(connectionID, true, sourceSocket);
SOM_CATCH("Error sending registration succeeded message")

//Make message to announce new station
std::string serializedUpdateMessage;
stream_status_update updateMessage;
(*updateMessage.mutable_new_base_station_info()) = (*streamInfo);

updateMessage.SerializeToString(&serializedUpdateMessage);

//Send the message
SOM_TRY
streamStatusNotificationInterface->send(serializedUpdateMessage.c_str(), serializedUpdateMessage.size());
SOM_CATCH("Error publishing new station registration\n")

return;//Registration finished
}//End station registration

//Base station has already been registered, so forward it and update the timeout info
//Allocate memory and copy the caster ID, stream ID and data to it
{
int totalMessageSize = receivedContent[1].size() + sizeof(Poco::Int64)+sizeof(Poco::Int64);
char *memoryBuffer = new char[totalMessageSize];
SOMScopeGuard memoryBufferScopeGuard([&]() { delete[] memoryBuffer; }); 

//Poco saves the day again (good serialization functions)
//Update map
Poco::Int64 streamID = 0;
streamID = associatedMap->at(connectionID).baseStationID;

*((Poco::Int64 *) memoryBuffer) = Poco::ByteOrder::toNetwork(Poco::Int64(casterID));
*(((Poco::Int64 *) memoryBuffer) + 1) = Poco::ByteOrder::toNetwork(Poco::Int64(streamID));

//Copy message to buffer
memcpy((void *) &memoryBuffer[sizeof(Poco::Int64)*2], (void *) receivedContent[1].c_str(), receivedContent[1].size());

//Forward message
SOM_TRY
clientStreamPublishingInterface->send(memoryBuffer, totalMessageSize);
SOM_CATCH("Error, unable to forward message\n")

SOM_TRY
proxyStreamPublishingInterface->send(memoryBuffer, totalMessageSize);
SOM_CATCH("Error, unable to forward message\n")

//Update map and register potential timeout event
//Update map
associatedMap->at(connectionID).timeLastMessageWasReceived = timeValue;

//Register possible stream timeout event
possible_base_station_event_timeout timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_id(connectionID);
timeoutEventSubMessage.set_is_authenticated(inputIsAuthenticated);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field)) = timeoutEventSubMessage;

inputEventQueue.push(timeoutEvent);
}

}


/**
This function processes reply messages sent to registrationDatabaseRequestSocket.  It expects database operations to succeed, so it throws an exception upon receiving a failure message.

@throws: This function can throw exceptions
*/
void caster::processTransmitterRegistrationAndStreamingDatabaseReply()
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive first message part (should be empty)
messageReceived = registrationDatabaseRequestSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving database reply message")

if(!messageReceived || messageBuffer->size() != 0 || !messageBuffer->more())
{
return;
}

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

messageReceived = false;
SOM_TRY //Receive message
messageReceived = registrationDatabaseRequestSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving server registration message")

database_reply reply;
reply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(reply.has_reason())
{
throw SOMException("Database request failed (" +std::to_string((int) reply.reason())+ "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

}

/**
This function processes key_management_request messages and accordingly modifies the list of accepted signing keys.  If a connection is reliant on a dropped signing key (has no other valid signing keys), then it will be dropped when the signing key is taken out of circulation.
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void caster::processKeyManagementRequest(std::priority_queue<event> &inputEventQueue)
{
//Create lambda to make it easy to send request failed replies
auto sendReplyLambda = [&] (bool inputRequestFailed, enum key_management_request_failure_reason inputReason = KEY_MESSAGE_FORMAT_INVALID)
{
key_management_reply reply;
std::string serializedReply;


reply.set_request_succeeded(!inputRequestFailed);

if(inputRequestFailed)
{ //Only set the reason if the request failed
reply.set_failure_reason(inputReason);
}

reply.SerializeToString(&serializedReply);

SOM_TRY
keyRegistrationAndRemovalInterface->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive first message part (should be empty)
messageReceived = keyRegistrationAndRemovalInterface->recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving key registration message")

if(!messageReceived)
{
return;
}

key_management_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
SOM_TRY
sendReplyLambda(true, KEY_MESSAGE_FORMAT_INVALID); //Couldn't deserialize message 
SOM_CATCH("Error, unable to send reply")
}

if(!request.has_serialized_key_status_changes() || !request.has_signature())
{
SOM_TRY
sendReplyLambda(true, KEY_MISSING_REQUIRED_FIELD); //One or more needed fields are missing 
SOM_CATCH("Error, unable to send reply")
} 

//Check signatures
if(request.signature().cryptographic_signature().size() != crypto_sign_BYTES || request.signature().public_key().size() != crypto_sign_PUBLICKEYBYTES)
{ //Signature or public key is wrong size
SOM_TRY
sendReplyLambda(true, KEY_MESSAGE_FORMAT_INVALID);
SOM_CATCH("Error, unable to send reply")
}

if(request.signature().public_key() != signingKeysManagementKey)
{
SOM_TRY
sendReplyLambda(true, KEY_INSUFFICIENT_PERMISSIONS);
SOM_CATCH("Error, unable to send reply")
}

if(crypto_sign_verify_detached((const unsigned char *) request.signature().cryptographic_signature().c_str(),(const unsigned char *) request.serialized_key_status_changes().c_str(), request.serialized_key_status_changes().size(), (const unsigned char *) request.signature().public_key().c_str()) != 0)
{ //Signature of permissions doesn't match (message invalid)
SOM_TRY
sendReplyLambda(true, KEY_MESSAGE_FORMAT_INVALID);
SOM_CATCH("Error, unable to send reply")
}

//Deserialized desired change
key_status_changes changes;
changes.ParseFromArray(request.serialized_key_status_changes().c_str(), request.serialized_key_status_changes().size());

if(!changes.IsInitialized())
{
SOM_TRY
sendReplyLambda(true, KEY_STATUS_CHANGE_DESERIALIZATION_FAILED); //Couldn't deserialize message 
SOM_CATCH("Error, unable to send reply")
}

//Validate changes message
if(changes.official_signing_keys_to_add_size() != changes.official_signing_keys_to_add_valid_until_size() || changes.registered_community_signing_keys_to_add_size() != changes.registered_community_signing_keys_to_add_valid_until_size() || changes.keys_to_add_to_blacklist_size() != changes.keys_to_add_to_blacklist_valid_until_size())
{
SOM_TRY
sendReplyLambda(true, KEY_MISSING_REQUIRED_FIELD); //One or more fields not right
SOM_CATCH("Error, unable to send reply")
}

//Process changes
//Handle blacklist entries
for(int i=0; i<changes.keys_to_add_to_blacklist_size(); i++)
{ 
//add key to blacklist and terminate any connections who were solely reliant on associated keys
addBlacklistKey(changes.keys_to_add_to_blacklist(i), changes.keys_to_add_to_blacklist_valid_until(i), inputEventQueue);
}

for(int i=0; i<changes.official_signing_keys_to_add_size(); i++)
{
addSigningKey(changes.official_signing_keys_to_add(i), true, changes.official_signing_keys_to_add_valid_until(i), inputEventQueue);
}

for(int i=0; i<changes.official_signing_keys_to_add_size(); i++)
{
addSigningKey(changes.registered_community_signing_keys_to_add(i), false,  changes.registered_community_signing_keys_to_add_valid_until(i), inputEventQueue);
}

SOM_TRY
sendReplyLambda(false); //Operation succeeded
SOM_CATCH("Error, unable to send reply")
}


/*
This function generates the complete query string required to get all of the ids of the stations that meet the query's requirements.
@param inputRequest: This is the request to generate the query string for
@param inputParameterCountBuffer: This is set to the total number of bound variables
@return: The query string

@throws: This function can throw exceptions
*/
std::string caster::generateClientQueryRequestSQLString(const client_query_request &inputRequest, int &inputParameterCountBuffer)
{
//Construct SQL query string from client query request
std::string primaryKeyFieldName;
SOM_TRY
primaryKeyFieldName = basestationToSQLInterface->getPrimaryKeyFieldName();
SOM_CATCH("Error retrieving primary key field name\n")

//Create and store subquery clauses
int parameterCount = 0;
std::vector<std::string> subQueryStrings;
for(int i=0; i<inputRequest.subqueries_size(); i++)
{
std::string subQueryString;
if(i!=0)
{
subQueryString += " OR ";
}
subQueryString += "(";

//Handle acceptable classes subquery
subQueryString += generateInSubquery(false, "station_class", inputRequest.subqueries(i).acceptable_classes_size());
parameterCount += inputRequest.subqueries(i).acceptable_classes_size();

//Handle acceptable_formats subquery
subQueryString += generateInSubquery(parameterCount > 0, "message_format", inputRequest.subqueries(i).acceptable_formats_size());
parameterCount += inputRequest.subqueries(i).acceptable_formats_size();

for(int a=0; a<inputRequest.subqueries(i).latitude_condition_size(); a++)
{ //Handle latitude conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "latitude", inputRequest.subqueries(i).latitude_condition(a).relation());
}
parameterCount += inputRequest.subqueries(i).latitude_condition_size();

for(int a=0; a<inputRequest.subqueries(i).longitude_condition_size(); a++)
{ //Handle longitude conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "longitude", inputRequest.subqueries(i).longitude_condition(a).relation());
}
parameterCount += inputRequest.subqueries(i).longitude_condition_size();

for(int a=0; a<inputRequest.subqueries(i).uptime_condition_size(); a++)
{ //Handle uptime conditions
//Have to flip relation due to startTime < requirement being related to uptime > requirement
subQueryString += generateRelationalSubquery(parameterCount > 0, "start_time", flipOperator(inputRequest.subqueries(i).uptime_condition(a).relation()));
}
parameterCount += inputRequest.subqueries(i).uptime_condition_size();

for(int a=0; a<inputRequest.subqueries(i).real_update_rate_condition_size(); a++)
{ //Handle real update rate conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "real_update_rate", inputRequest.subqueries(i).real_update_rate_condition(a).relation());
}
parameterCount += inputRequest.subqueries(i).real_update_rate_condition_size();

for(int a=0; a<inputRequest.subqueries(i).expected_update_rate_condition_size(); a++)
{ //Handle real update rate conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "expected_update_rate", inputRequest.subqueries(i).expected_update_rate_condition(a).relation());
}
parameterCount += inputRequest.subqueries(i).expected_update_rate_condition_size();

if(inputRequest.subqueries(i).has_informal_name_condition())
{ //Add informal name condition
if(parameterCount > 0)
{
subQueryString += " AND ";
}

subQueryString += "(informal_name " + sqlStringRelationalOperatorToSQLString(inputRequest.subqueries(i).informal_name_condition().relation()) + ")";
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).base_station_id_condition_size(); a++)
{ //Handle base station id conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "base_station_id", inputRequest.subqueries(i).base_station_id_condition(a).relation());
}
parameterCount += inputRequest.subqueries(i).base_station_id_condition_size();

for(int a=0; a<inputRequest.subqueries(i).source_public_keys_size(); a++)
{ //Handle source public keys restrictions
if(parameterCount > 0)
{
subQueryString += " AND ";
}

subQueryString += "(source_public_key == ?)";
}
parameterCount += inputRequest.subqueries(i).source_public_keys_size();

if(inputRequest.subqueries(i).has_circular_search_region())
{ //Handle requests for basestations within a radius of a particular location 35.779411, -78.648033
//SELECT * from coords WHERE id IN (SELECT id FROM (SELECT id, lat*lat + long*long AS distance FROM coords GROUP BY distance HAVING distance < 7740.0));
if(parameterCount > 0)
{
subQueryString += " AND ";
} //57.2957795130785 -> radian to degrees constant
subQueryString += "(base_station_id IN (SELECT base_station_id FROM (SELECT base_station_id, (6371000*acos(57.2957795130785*(cos(?)*cos(latitude)*cos(longitude-?) + sin(?)*sin(latitude)))) AS distance FROM " + basestationToSQLInterface->primaryTableName + " GROUP BY distance HAVING distance <= ?)))";
//params Qlatitude, Qlongitude, Qlatitude, distance constraint value
parameterCount += 4;
}

subQueryString += ")";

if(subQueryString != " OR ()")
{ //Only add subquery if it contains clauses
subQueryStrings.push_back(subQueryString);
}

} //End subquery string generation

std::string queryString = "SELECT " + primaryKeyFieldName + " FROM " + basestationToSQLInterface->primaryTableName;

if(subQueryStrings.size() > 0)
{
queryString +=" WHERE ";
}
for(int i=0; i<subQueryStrings.size(); i++)
{
queryString += subQueryStrings[i];
}
queryString += ";";

//Can't have more than 999 bound variables
inputParameterCountBuffer = parameterCount;

return queryString;
}

/**
This function makes it easier to process messages from a ZMQ router socket.  It retrieves the messages associated with a single router message (all parts of the multipart message) and stores them in the vector and returns true if the expected format was followed (3 part: addess, empty, message)
@param inputSocket: The socket to process messages from
@param inputMessageBuffer: The buffer to store messages in
@return: true if the message followed the expected format

@throws: This function can throw exceptions
*/
bool pylongps::retrieveRouterMessage(zmq::socket_t &inputSocket, std::vector<std::string> &inputMessageBuffer)
{
//Receive messages
std::unique_ptr<zmq::message_t> messageBuffer;
inputMessageBuffer.clear();

while(true)
{
SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive message
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving server registration message")

if(!messageReceived)
{
break;
}

inputMessageBuffer.push_back(std::string((const char *) messageBuffer->data(), messageBuffer->size()));

if(!messageBuffer->more())
{ //Multi-part message completed, so exit loop
break;
}

}

if(inputMessageBuffer.size() != 2)
{
return false; //Message is invalid, so mark it so
}

return true;
}

/**
This function helps with creating SQL query strings for client requests.
@param inputRelation: The relation to resolve into a SQL string part (such as "<= ?"
@return: The associated SQL string component 
*/
std::string pylongps::sqlRelationalOperatorToSQLString(sql_relational_operator inputRelation)
{
if(inputRelation == LESS_THAN)
{
return "< ?";
}
else if(inputRelation == LESS_THAN_EQUAL_TO)
{
return "<= ?";
}
else if(inputRelation == EQUAL_TO)
{
return "= ?";
}
else if(inputRelation == NOT_EQUAL_TO)
{
return "!= ?";
}
else if(inputRelation == GREATER_THAN)
{
return "> ?";
}
else//(inputRelation == GREATER_THAN_EQUAL_TO)
{
return ">= ?";
}
}

/**
This function binds the fields from the client_query_request to the associated prepared statement.
@param inputStatement: The statement to bind the fields for
@param inputRequest: The request to bind the fields with
@return: The number of bound parameters

@throws: This function can throw exceptions
*/
int caster::bindClientQueryRequestFields(std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement, const client_query_request &inputRequest)
{
int parameterCount = 0;
for(int i=0; i<inputRequest.subqueries_size(); i++)
{
//Handle acceptable classes subquery
for(int a=0; a<inputRequest.subqueries(i).acceptable_classes_size(); a++)
{
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue((::google::protobuf::int64) inputRequest.subqueries(i).acceptable_classes(a)));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

//Handle acceptable_formats subquery
for(int a=0; a<inputRequest.subqueries(i).acceptable_formats_size(); a++)
{
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue((::google::protobuf::int64) inputRequest.subqueries(i).acceptable_formats(a)));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}


for(int a=0; a<inputRequest.subqueries(i).latitude_condition_size(); a++)
{ //Handle latitude conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).latitude_condition(a).value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).longitude_condition_size(); a++)
{ //Handle longitude conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).longitude_condition(a).value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
for(int a=0; a<inputRequest.subqueries(i).uptime_condition_size(); a++)
{ //Handle uptime conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue((::google::protobuf::int64) (timeValue-inputRequest.subqueries(i).uptime_condition(a).value()*1000000.0)));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).real_update_rate_condition_size(); a++)
{ //Handle real_update_rate conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).real_update_rate_condition(a).value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).expected_update_rate_condition_size(); a++)
{ //Handle expected_update_rate conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).expected_update_rate_condition(a).value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

if(inputRequest.subqueries(i).has_informal_name_condition())
{ //Add informal name condition

SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).informal_name_condition().value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).base_station_id_condition_size(); a++)
{ //Handle base_station_id conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue((::google::protobuf::int64) inputRequest.subqueries(i).base_station_id_condition(a).value()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).source_public_keys_size(); a++)
{ //Handle source public key conditions
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).source_public_keys(a)));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

if(inputRequest.subqueries(i).has_circular_search_region())
{ //Handle requests for basestations within a radius of a particular 
//subQueryString += "(base_station_id IN (SELECT base_station_id FROM (SELECT base_station_id, (6371000*acos(cos(?)*cos(latitude)*cos(longitude-?) + sin(?)*sin(latitude))) AS distance FROM " + basestationToSQLInterface->primaryTableName + " GROUP BY distance HAVING distance <= ?)))";
//params Qlatitude, Qlongitude, Qlatitude, distance constraint value
SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).circular_search_region().latitude()));
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).circular_search_region().longitude()));
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).circular_search_region().latitude()));
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
basestationToSQLInterface->bindFieldToStatement(inputStatement.get(), parameterCount+1, fieldValue(inputRequest.subqueries(i).circular_search_region().radius()));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}
}

return parameterCount;
}

/**
This (non-threadsafe) function generates new unique (sequential) connection ids.
@return: A new connection ID to use
*/
int64_t caster::getNewStreamID()
{
lastAssignedConnectionID++;
return lastAssignedConnectionID;
}

/**
This function checks whether the permissions in a credentials message are considered valid according to the current lists of trusted keys.  
@param inputCredentials: The credentials giving the permissions/signing keys
@param inputAuthorizedPermissionsBuffer: The object to return the authorized_permissions with
@return: A tuple of <messageIsValid, isSignedByOfficialEntityKey, isSignedByRegisteredCommunityKey>
*/
std::tuple<bool, bool, bool> caster::checkCredentials(credentials &inputCredentials, authorized_permissions &inputAuthorizedPermissionsBuffer)
{
bool isSignedByOfficialEntityKey = false;
bool isSignedByRegisteredCommunityKey = false;

//Check signatures
for(int i=0; i<inputCredentials.signatures_size(); i++)
{
if(inputCredentials.signatures(i).cryptographic_signature().size() != crypto_sign_BYTES || inputCredentials.signatures(i).public_key().size() != crypto_sign_PUBLICKEYBYTES)
{ //Signature or public key is wrong size
return std::tuple<bool, bool, bool>(false, false, false);
}

if(crypto_sign_verify_detached((const unsigned char *) inputCredentials.signatures(i).cryptographic_signature().c_str(),(const unsigned char *) inputCredentials.permissions().c_str(), inputCredentials.permissions().size(), (const unsigned char *) inputCredentials.signatures(i).public_key().c_str()) != 0)
{ //Signature of permissions doesn't match (message invalid)
return std::tuple<bool, bool, bool>(false, false, false);
}

if(officialSigningKeys.count(inputCredentials.signatures(i).public_key()) > 0)
{
isSignedByOfficialEntityKey = true;
}

if(registeredCommunitySigningKeys.count(inputCredentials.signatures(i).public_key()) > 0)
{
isSignedByRegisteredCommunityKey = true;
}

}

return std::tuple<bool, bool, bool>(true, isSignedByOfficialEntityKey, isSignedByRegisteredCommunityKey);
}


/**
This function removes a basestation connection from both the maps and the database.
@param inputConnectionID: The connection to remove
@param inputIsAuthenticated: True if the associated connection is authenticated

@throws: This function can throw exceptions
*/
void caster::removeConnection(const std::string &inputConnectionID, bool inputIsAuthenticated)
{

if(inputIsAuthenticated)
{ //Handle if authenticated
SOM_TRY
removeAuthenticatedConnection(inputConnectionID);
SOM_CATCH("Error, unable to remove authenticated connection\n")
}
else
{
removeUnauthenticatedConnection(inputConnectionID);
}
}


/**
This function helps with creating SQL query strings for client requests.
@param inputStringRelation: The relation to resolve into a SQL string part (such as "LIKE ?"
@return: The associated SQL string component 
*/
std::string pylongps::sqlStringRelationalOperatorToSQLString(sql_string_relational_operator inputStringRelation)
{
if(inputStringRelation == IDENTICAL)
{
return "= ?";
}
else //LIKE
{
return "LIKE ?";
}
}

/**
This function generates a sql query subpart of the form (fieldName IN (?, ?, etc))
@param inputPreappendAND: True if an " AND " should be added before the subquery part
@param inputFieldName: The field name to have conditions on
@param inputNumberOfFields: The number of entries in the "IN" set
@return: The query subpart
*/
std::string pylongps::generateInSubquery(bool inputPreappendAND, const std::string &inputFieldName, int inputNumberOfFields)
{
std::string querySubPart;

if(inputNumberOfFields <= 0)
{
return "";
}

if(inputPreappendAND)
{
querySubPart += " AND ";
}

querySubPart += "(" + inputFieldName + " IN (";

for(int i=0; i<inputNumberOfFields; i++)
{
if(i!=0)
{
querySubPart += ", ";
}

querySubPart += "?";
}
querySubPart += "))";

return querySubPart;
}

/**
This function generates a sql query subpart of the form (fieldName IN (?, ?, etc))
@param inputPreappendAND: True if an " AND " should be added before the subquery part
@param inputFieldName: The field name to have conditions on
@param inputRelationalOperator: The relational operator to impose using the value
@return: The query subpart
*/
std::string pylongps::generateRelationalSubquery(bool inputPreappendAND, const std::string &inputFieldName, sql_relational_operator inputRelationalOperator)
{
std::string querySubPart;

if(inputPreappendAND)
{
querySubPart += " AND ";
}

querySubPart += "("+inputFieldName + " " + sqlRelationalOperatorToSQLString(inputRelationalOperator) + ")";
return querySubPart;
}

/**
This function flips the sign of the given relational operator.  Give >=, it will return <=, etc.
@param inputSQLRelationalOperator: The operator to flip
@return: The flipped operator
*/
sql_relational_operator pylongps::flipOperator(sql_relational_operator inputSQLRelationalOperator)
{
if(inputSQLRelationalOperator == LESS_THAN)
{
return GREATER_THAN;
}
else if(inputSQLRelationalOperator == LESS_THAN_EQUAL_TO)
{
return GREATER_THAN_EQUAL_TO;
}
else if(inputSQLRelationalOperator == GREATER_THAN)
{
return LESS_THAN;
}
else if(inputSQLRelationalOperator == GREATER_THAN_EQUAL_TO)
{
return LESS_THAN_EQUAL_TO;
}

return inputSQLRelationalOperator; //No sign to change
}

/**
This function can be used to add the "sin" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void pylongps::SQLiteSinFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues)
{

if(inputArraySize > 0)
{
sqlite3_result_double(inputContext, sin(sqlite3_value_double(inputValues[0])*DEGREES_TO_RADIANS_CONSTANT));
}
else
{
sqlite3_result_double(inputContext, 0.0);
}
}

/**
This function can be used to add the "cos" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void pylongps::SQLiteCosFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues)
{

if(inputArraySize > 0)
{
sqlite3_result_double(inputContext, cos(sqlite3_value_double(inputValues[0])*DEGREES_TO_RADIANS_CONSTANT));
}
else
{
sqlite3_result_double(inputContext, 0.0);
}
}

/**
This function can be used to add the "acos" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void pylongps::SQLiteAcosFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues)
{
if(inputArraySize > 0)
{
sqlite3_result_double(inputContext, acos(sqlite3_value_double(inputValues[0])*DEGREES_TO_RADIANS_CONSTANT));
}
else
{
sqlite3_result_double(inputContext, 0.0);
}
}

/**
This function removes all entries of the map with the specific key/value.
@param inputMultimap: the multimap to delete from
@param inputKey: the key of the key/value pair
@param inputValue: The value of the key/value pair
*/
void pylongps::removeKeyValuePairFromStringMultimap(std::multimap<std::string, std::string> &inputMultimap, const std::string &inputKey, const std::string &inputValue)
{
auto equalRange = inputMultimap.equal_range(inputKey);
for(auto iter = equalRange.first; iter!=equalRange.second;)
{
if(iter->second == inputValue)
{
iter = inputMultimap.erase(iter); //Erase the pair and move to the next position
}
else
{
iter++; //Just go to the next entry to check
}
}

}

