#include "caster.hpp"

using namespace pylongps;

/**
This function initializes the class, creates the associated database, and starts the two threads associated with it.
@param inputContext: The ZMQ context that this object should use
@param inputCasterID: The 64 bit ID associated with this caster (make sure it does not collide with caster IDs the clients are likely to run into)
@param inputTransmitterRegistrationAndStreamingPortNumber: The port number to register the ZMQ router socket used for receiving PylonGPS transmitter registrations/streams
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
caster::caster(zmq::context_t *inputContext, int64_t inputCasterID, uint32_t inputTransmitterRegistrationAndStreamingPortNumber, uint32_t inputClientRequestPortNumber, uint32_t inputClientStreamPublishingPortNumber, uint32_t inputProxyStreamPublishingPortNumber, uint32_t inputStreamStatusNotificationPortNumber, uint32_t inputKeyRegistrationAndRemovalPortNumber, const std::string &inputCasterPublicKey, const std::string &inputCasterSecretKey, const std::string &inputSigningKeysManagementKey, const std::vector<std::string> &inputOfficialSigningKeys, const std::vector<std::string> &inputRegisteredCommunitySigningKeys, const std::vector<std::string> &inputBlacklistedKeys, const std::string &inputCasterSQLITEConnectionString)  : databaseConnection(nullptr, &sqlite3_close_v2)
{
SOM_TRY
commonConstructor(inputContext, inputCasterID, inputTransmitterRegistrationAndStreamingPortNumber, inputClientRequestPortNumber, inputClientStreamPublishingPortNumber, inputProxyStreamPublishingPortNumber, inputStreamStatusNotificationPortNumber, inputKeyRegistrationAndRemovalPortNumber, inputCasterPublicKey, inputCasterSecretKey, inputSigningKeysManagementKey, inputOfficialSigningKeys, inputRegisteredCommunitySigningKeys, inputBlacklistedKeys, inputCasterSQLITEConnectionString);
SOM_CATCH("Error in subconstructor\n")
}

/**
This function intializes the object based on the parameters in a protobuf message (which allows serialization/deserialization of configuration parameters).
@param inputContext: The ZMQ context to use
@param inputConfiguration: The protobuf message containing the configuration information

@throws: This function can throw exceptions
*/
caster::caster(zmq::context_t *inputContext, const caster_configuration &inputConfiguration)  : databaseConnection(nullptr, &sqlite3_close_v2)
{
//Convert repeated fields into std::vectors
std::vector<std::string> officialSigningKeys;
for(int i=0; i<inputConfiguration.official_signing_keys_size(); i++)
{
officialSigningKeys.push_back(inputConfiguration.official_signing_keys(i));
}

std::vector<std::string> registeredCommunitySigningKeys;
for(int i=0; i<inputConfiguration.registered_community_signing_keys_size(); i++)
{
registeredCommunitySigningKeys.push_back(inputConfiguration.registered_community_signing_keys(i));
}

std::vector<std::string> blacklistedKeys;
for(int i=0; i<inputConfiguration.blacklisted_keys_size(); i++)
{
blacklistedKeys.push_back(inputConfiguration.blacklisted_keys(i));
}


SOM_TRY
commonConstructor(inputContext, inputConfiguration.caster_id(), inputConfiguration.transmitter_registration_and_streaming_port_number(), inputConfiguration.client_request_port_number(), inputConfiguration.client_stream_publishing_port_number(), inputConfiguration.proxy_stream_publishing_port_number(), inputConfiguration.stream_status_notification_port_number(), inputConfiguration.key_registration_and_removal_port_number(), inputConfiguration.caster_public_key(), inputConfiguration.caster_secret_key(), inputConfiguration.signing_keys_management_key(), officialSigningKeys, registeredCommunitySigningKeys, blacklistedKeys);
SOM_CATCH("Error in subconstructor\n")
}

/**
This function initializes the class, creates the associated database, and starts the two threads associated with it (used in constructors).
@param inputContext: The ZMQ context that this object should use
@param inputCasterID: The 64 bit ID associated with this caster (make sure it does not collide with caster IDs the clients are likely to run into)
@param inputTransmitterRegistrationAndStreamingPortNumber: The port number to register the ZMQ router socket used for receiving PylonGPS transmitter registrations/streams
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
void caster::commonConstructor(zmq::context_t *inputContext, int64_t inputCasterID, uint32_t inputTransmitterRegistrationAndStreamingPortNumber, uint32_t inputClientRequestPortNumber, uint32_t inputClientStreamPublishingPortNumber, uint32_t inputProxyStreamPublishingPortNumber, uint32_t inputStreamStatusNotificationPortNumber, uint32_t inputKeyRegistrationAndRemovalPortNumber, const std::string &inputCasterPublicKey, const std::string &inputCasterSecretKey, const std::string &inputSigningKeysManagementKey, const std::vector<std::string> &inputOfficialSigningKeys, const std::vector<std::string> &inputRegisteredCommunitySigningKeys, const std::vector<std::string> &inputBlacklistedKeys, const std::string &inputCasterSQLITEConnectionString)
{
if(inputContext == nullptr)
{
throw SOMException("Invalid ZMQ context\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Check that keys are the right size
if((inputCasterPublicKey.size() != crypto_sign_PUBLICKEYBYTES) || (inputCasterSecretKey.size() != crypto_sign_SECRETKEYBYTES ))
{
throw SOMException("Invalid ZMQ key(s)\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Save given parameters
context = inputContext;
casterID = inputCasterID;
transmitterRegistrationAndStreamingPortNumber = inputTransmitterRegistrationAndStreamingPortNumber;
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

//Make sure foreign key relationships will be honored
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


//Initialize and bind database access socket
 //A inproc REP socket that handles requests to make changes to the database.  Used by clientRequestHandlingReactor.
std::unique_ptr<zmq::socket_t> databaseAccessSocket;
SOM_TRY
databaseAccessSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing databaseAccessSocket\n")

SOM_TRY //Bind to an dynamically generated address
std::tie(databaseAccessConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*databaseAccessSocket, "databaseAccessAddress");
SOM_CATCH("Error binding databaseAccessSocket\n")

//Initialize and connect the registration thread's database request socket
//A inproc dealer socket used in the streamRegistrationAndPublishingThread to send database requests/get replies
std::unique_ptr<zmq::socket_t> registrationDatabaseRequestSocket; 
SOM_TRY
registrationDatabaseRequestSocket.reset(new zmq::socket_t(*(context), ZMQ_DEALER));
SOM_CATCH("Error intializing registrationDatabaseRequestSocket\n")

SOM_TRY
registrationDatabaseRequestSocket->connect(databaseAccessConnectionString.c_str());
SOM_CATCH("Error connecting registration database request socket")

//Initialize and connect the statistics thread's database request socket
//A inproc dealer socket used in the statisticsGatheringThread to send database requests/get replies
std::unique_ptr<zmq::socket_t> statisticsDatabaseRequestSocket; 
SOM_TRY
statisticsDatabaseRequestSocket.reset(new zmq::socket_t(*(context), ZMQ_DEALER));
SOM_CATCH("Error intializing statisticsDatabaseRequestSocket\n")

SOM_TRY
statisticsDatabaseRequestSocket->connect(databaseAccessConnectionString.c_str());
SOM_CATCH("Error connecting statistics database request socket")

//Initialize and bind transmitterRegistrationAndStreaming socket
///A ZMQ ROUTER socket which expects a transmitter_registration_request to which it responses with a transmitter_registration_reply. If accepted, the request is followed by the data to broadcast.  If the data is athenticated, the data message has a preappended sodium signature of length crypto_sign_BYTES.  Used by streamRegistrationAndPublishingReactor.
std::unique_ptr<zmq::socket_t> transmitterRegistrationAndStreamingInterface; 
SOM_TRY
transmitterRegistrationAndStreamingInterface.reset(new zmq::socket_t(*(context), ZMQ_ROUTER));
SOM_CATCH("Error intializing transmitterRegistrationAndStreamingInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(transmitterRegistrationAndStreamingPortNumber);
transmitterRegistrationAndStreamingInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding transmitterRegistrationAndStreamingInterface\n")



//Initialize and add filter for proxiesUpdatesListeningSocket
//A TCP SUB socket which subscribes to other casters so that it can republish the updates it received.  Used in the streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> proxiesUpdatesListeningSocket; 
SOM_TRY
proxiesUpdatesListeningSocket.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error intializing proxiesUpdatesListeningSocket\n")

SOM_TRY //Set filter to allow any published messages to be received
proxiesUpdatesListeningSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for proxiesUpdatesListeningSocket\n")

//Initialize and add filter for proxiesNotificationsListeningSocket
 //A TCP SUB socket which subscribes to other casters so that it can know when they add/remove a basestation. Used in the streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> proxiesNotificationsListeningSocket;
SOM_TRY
proxiesNotificationsListeningSocket.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error intializing proxiesNotificationsListeningSocket\n")

SOM_TRY //Set filter to allow any published messages to be received
proxiesNotificationsListeningSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for proxiesNotificationsListeningSocket\n")



//A inproc REP socket which accepts add_remove_proxy_requests and responds with a add_remove_proxy_reply.  Used in the streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> addRemoveProxiesSocket; 
SOM_TRY
addRemoveProxiesSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing shutdownPublishingSocket\n")

extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(addRemoveProxyConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*addRemoveProxiesSocket, "addRemoveProxiesSocket");
SOM_CATCH("Error binding addRemoveProxiesSocket\n")


//Initialize and bind keyRegistrationAndRemoval socket
///A ZMQ REP socket which expects a key_management_request message and sends back a key_management_reply message.  Used by streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> keyRegistrationAndRemovalInterface;
SOM_TRY
keyRegistrationAndRemovalInterface.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error intializing keyRegistrationAndRemovalInterface\n")

SOM_TRY
std::string bindingAddress = "tcp://*:" + std::to_string(inputKeyRegistrationAndRemovalPortNumber);
keyRegistrationAndRemovalInterface->bind(bindingAddress.c_str());
SOM_CATCH("Error binding keyRegistrationAndRemovalInterface\n")

//Initialize and bind clientRequestInterface socket
///A ZMQ REP socket which expects a client_query_request and responds with a client_query_reply.  Used by clientRequestHandlingThread.
std::unique_ptr<zmq::socket_t> clientRequestInterface;  
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

//Initialize and bind internalNotificationPublisher socket
SOM_TRY
internalNotificationPublisher.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error intializing internalNotificationPublisher\n")

extensionStringNumber = 0;
SOM_TRY //Bind to an dynamically generated address
std::tie(internalNotificationPublisherConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*internalNotificationPublisher, "internalNotificationPublisher");
SOM_CATCH("Error binding internalNotificationPublisher\n")

//Connect to internal notifications socket to hear internally generated notifications about casters this one is proxying
SOM_TRY
proxiesNotificationsListeningSocket->connect(internalNotificationPublisherConnectionString.c_str());
SOM_CATCH("Error connecting to internal notification publisher\n")

//Initialize and connect the statistic reactor's streamStatusNotificationListener
//A TCP SUB socket used in the statisticsGatheringThread to listen to the streamStatusNotificationInterface
std::unique_ptr<zmq::socket_t> streamStatusNotificationListener;
SOM_TRY
streamStatusNotificationListener.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error intializing streamStatusNotificationListener\n")

SOM_TRY //Set filter to allow any published messages to be received
streamStatusNotificationListener->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for socket listening for socket notifications\n")

SOM_TRY
std::string connectionAddress = "tcp://127.0.0.1:" + std::to_string(streamStatusNotificationPortNumber);
streamStatusNotificationListener->connect(connectionAddress.c_str());
SOM_CATCH("Error connecting streamStatusNotificationListener socket")

//Initialize and connect the statistic reactor's proxyStreamListener
//A TCP SUB socket used in the statisticsGatheringThread to listen to the proxyStreamPublishingInterface
std::unique_ptr<zmq::socket_t> proxyStreamListener; 
SOM_TRY
proxyStreamListener.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error intializing proxyStreamListener\n")

SOM_TRY //Set filter to allow any published messages to be received
proxyStreamListener->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for socket listening for proxy stream\n")

SOM_TRY 
std::string connectionAddress = "tcp://127.0.0.1:" + std::to_string(proxyStreamPublishingPortNumber);
proxyStreamListener->connect(connectionAddress.c_str());
SOM_CATCH("Error connecting proxyStreamListener socket")

//Responsible for streamStatusNotificationListener, proxyStreamListener, statisticsDatabaseRequestSocket
SOM_TRY
statisticsGatheringReactor.reset(new reactor<caster>(context, this, &caster::handleReactorEvents));
SOM_CATCH("Error creating reactor\n")

SOM_TRY
statisticsGatheringReactor->addInterface(streamStatusNotificationListener, &caster::statisticsProcessStreamStatusNotification, "streamStatusNotificationListener"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
statisticsGatheringReactor->addInterface(proxyStreamListener, &caster::statisticsProcessStreamMessage, "proxyStreamListener"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
statisticsGatheringReactor->addInterface(statisticsDatabaseRequestSocket, &caster::statisticsProcessDatabaseReply, "statisticsDatabaseRequestSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

//Add event to manage the update cycle
Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
update_statistics_event updateEventSubMessage;
event updateEvent(timeValue + 1000000.0); //Active in 1 second
(*updateEvent.MutableExtension(update_statistics_event::update_statistics_event_field)) = updateEventSubMessage;

std::vector<event> statisticsStartingEvents;
statisticsStartingEvents.push_back(updateEvent);

SOM_TRY
statisticsGatheringReactor->start(statisticsStartingEvents);
SOM_CATCH("Error starting reactor\n")

//Create reactor to handle client requests
//Responsible for databaseAccessSocket, clientRequestInterface
SOM_TRY
clientRequestHandlingReactor.reset(new reactor<caster>(context, this, &caster::handleReactorEvents));
SOM_CATCH("Error creating reactor\n")

SOM_TRY
clientRequestHandlingReactor->addInterface(databaseAccessSocket, &caster::processDatabaseRequest, "databaseAccessSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
clientRequestHandlingReactor->addInterface(clientRequestInterface, &caster::processClientQueryRequest, "clientRequestInterface"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
clientRequestHandlingReactor->start();
SOM_CATCH("Error starting reactor\n")

//Create reactor to handle registrations, key addition/deletion, and update publishing
//Responsible for transmitterRegistrationAndStreamingInterface, registrationDatabaseRequestSocket, keyRegistrationAndRemovalInterface, addRemoveProxiesSocket, proxiesUpdatesListeningSocket, proxiesNotificationsListeningSocket
//Publishes to clientStreamPublishingInterface, proxyStreamPublishingInterface, streamStatusNotificationInterface
SOM_TRY
streamRegistrationAndPublishingReactor.reset(new reactor<caster>(context, this, &caster::handleReactorEvents));
SOM_CATCH("Error creating reactor\n")

SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(transmitterRegistrationAndStreamingInterface, &caster::processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage, "transmitterRegistrationAndStreamingInterface"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(registrationDatabaseRequestSocket, &caster::processTransmitterRegistrationAndStreamingDatabaseReply, "registrationDatabaseRequestSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(keyRegistrationAndRemovalInterface, &caster::processKeyManagementRequest, "keyRegistrationAndRemovalInterface"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")


SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(addRemoveProxiesSocket, &caster::processAddRemoveProxyRequest, "addRemoveProxiesSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(proxiesUpdatesListeningSocket, &caster::listenForProxyUpdates, "proxiesUpdatesListeningSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")

SOM_TRY
streamRegistrationAndPublishingReactor->addInterface(proxiesNotificationsListeningSocket, &caster::listenForProxyNotifications, "proxiesNotificationsListeningSocket"); //Reactor takes ownership
SOM_CATCH("Error adding interface to reactor\n")


SOM_TRY
streamRegistrationAndPublishingReactor->start();
SOM_CATCH("Error starting reactor\n")
}

/**
This thread safe function adds a new caster to proxy.  The function may have some lag as a query needs to be sent to the caster to proxy and the results returned before this function does.
@param inputClientRequestConnectionString: The ZMQ connection string to use to connect to the client query answering port of the caster to proxy
@param inputBasestationPublishingConnectionString: The ZMQ connection string to use to connect to the interface that publishes the basestation updates
@param inputConnectionDisconnectionNotificationConnectionString: The ZMQ connection string to use to connect to the basestation connect/disconnect notification port on the caster to proxy

@throws: This function can throw exceptions
*/
void caster::addProxy(const std::string &inputClientRequestConnectionString, const std::string &inputBasestationPublishingConnectionString, const std::string &inputConnectDisconnectNotificationConnectionString)
{
//Create request socket to send request to caster threads
std::unique_ptr<zmq::socket_t> addProxyRequestSocket;
SOM_TRY
addProxyRequestSocket.reset(new zmq::socket_t(*(context), ZMQ_REQ));
SOM_CATCH("Error intializing addProxyRequestSocket\n")

SOM_TRY
addProxyRequestSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &PROXY_CLIENT_REQUEST_MAX_WAIT_TIME, sizeof(PROXY_CLIENT_REQUEST_MAX_WAIT_TIME));
SOM_CATCH("Error setting timeout time\n")

SOM_TRY
addProxyRequestSocket->connect(addRemoveProxyConnectionString.c_str());
SOM_CATCH("Error connecting addProxyRequestSocket")

//Make request
add_remove_proxy_request request;
request.set_client_request_connection_string(inputClientRequestConnectionString);
request.set_connect_disconnect_notification_connection_string(inputConnectDisconnectNotificationConnectionString);
request.set_base_station_publishing_connection_string(inputBasestationPublishingConnectionString);

//Get send request/get reply, causing the caster to start listening to the other caster's notifications and updates
add_remove_proxy_reply reply;
bool replyReceived = false;
bool replyDeserializedCorrectly = false;

SOM_TRY
std::tie(replyReceived, replyDeserializedCorrectly) = remoteProcedureCall(*addProxyRequestSocket, request, reply);
SOM_CATCH("Error with RPC\n")

if(!replyReceived)
{
throw SOMException("Local caster timed out\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

if(!replyDeserializedCorrectly)
{
throw SOMException("Invalid response from caster\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

if(reply.has_reason())
{ //Request failed
throw SOMException("Invalid response from caster\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}

//Wait for a millisecond or two to allow the subscriptions to take effect
std::this_thread::sleep_for(std::chrono::milliseconds(2));

//The caster is listening, so it will start registering/forwarding basestations that have registered at the other caster after it started listening.  Removals will be ignored and entries that are removed before the whole set of basestations are registered will have to be removed by timeout

//Send query to get all of the metadata for the basestations in the other caster
//Create/connect ephemeral socket to send query to source caster
std::unique_ptr<zmq::socket_t> ephemeralQuerySocket;
SOM_TRY
ephemeralQuerySocket.reset(new zmq::socket_t(*(context), ZMQ_REQ));
SOM_CATCH("Error intializing ephemeralQuerySocket\n")

SOM_TRY
ephemeralQuerySocket->setsockopt(ZMQ_RCVTIMEO, (void *) &PROXY_CLIENT_REQUEST_MAX_WAIT_TIME, sizeof(PROXY_CLIENT_REQUEST_MAX_WAIT_TIME));
SOM_CATCH("Error setting timeout time\n")

SOM_TRY
ephemeralQuerySocket->connect(request.client_request_connection_string().c_str());
SOM_CATCH("Error connecting ephemeralQuerySocket\n")

client_query_request queryRequest;
client_query_reply queryReply;

SOM_TRY
std::tie(replyReceived, replyDeserializedCorrectly) = remoteProcedureCall(*ephemeralQuerySocket, queryRequest, queryReply);
SOM_CATCH("Error with RPC\n")

if(!replyReceived || !replyDeserializedCorrectly)
{
throw SOMException("Invalid response from caster\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

if(queryReply.has_failure_reason() || !queryReply.has_caster_id())
{
throw SOMException("Query to caster to proxy failed\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}

//Publish all basestations on the internal proxy notification socket
int64_t foreignCasterID = queryReply.caster_id();
Poco::Int64 header[2] = {0, 0};
header[0] = Poco::ByteOrder::toNetwork(Poco::Int64(foreignCasterID));
for(int i=0; i < queryReply.base_stations_size(); i++)
{
//Make status update message
base_station_stream_information *basestation = queryReply.mutable_base_stations(i);

if(!basestation->has_base_station_id())
{
throw SOMException("Caster returned base station without id\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

int64_t baseStationID = basestation->base_station_id();
header[1] = Poco::ByteOrder::toNetwork(Poco::Int64(baseStationID));

stream_status_update updateMessage;
*updateMessage.mutable_new_base_station_info() = *basestation;

//Send notification with caster ID and stream ID preappended
SOM_TRY
sendProtobufMessage(*internalNotificationPublisher, updateMessage, std::string((const char *) header, sizeof(Poco::Int64)*2));
SOM_CATCH("Error sending notification\n")
}

//The caster has been subscribed to an all basestation metadata retrieved, so the proxy is established
}

/**
This thread safe function removes a foreign caster from monitoring by this caster.
@param inputClientRequestConnectionString: The ZMQ connection string used to send a query to the foreign caster

@throws: This function can throw exceptions
*/
void caster::removeProxy(const std::string &inputClientRequestConnectionString)
{
//Create request socket to send request to caster
std::unique_ptr<zmq::socket_t> addProxyRequestSocket;
SOM_TRY
addProxyRequestSocket.reset(new zmq::socket_t(*(context), ZMQ_REQ));
SOM_CATCH("Error intializing addProxyRequestSocket\n")

SOM_TRY
addProxyRequestSocket->connect(addRemoveProxyConnectionString.c_str());
SOM_CATCH("Error connecting addProxyRequestSocket")

//Make request
add_remove_proxy_request request;
request.set_client_request_connection_string_for_caster_to_remove(inputClientRequestConnectionString);

//Get send request/get reply, causing the caster to stop listening to the other caster's notifications and updates
add_remove_proxy_reply reply;
bool replyReceived = false;
bool replyDeserializedCorrectly = false;

SOM_TRY
std::tie(replyReceived, replyDeserializedCorrectly) = remoteProcedureCall(*addProxyRequestSocket, request, reply);
SOM_CATCH("Error with RPC\n")

if(!replyReceived || !replyDeserializedCorrectly)
{
throw SOMException("Invalid response from caster\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

if(reply.has_reason())
{ //Request failed
throw SOMException("Invalid response from caster\n", SERVER_REQUEST_FAILED, __FILE__, __LINE__);
}

//All of the basestations from the foreign caster will timeout shortly because it will no longer receive updates for them
}

/**
This function signals for the threads to shut down and then waits for them to do so.
*/
caster::~caster()
{

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
}


/**
This function processes any events that are scheduled to have occurred by now and returns when the next event is scheduled to occur.
@param inputReactor: The reactor to process events for
@return: The time point associated with the soonest event timeout (negative if there are no outstanding events)

@throws: This function can throw exceptions
*/
Poco::Timestamp caster::handleReactorEvents(reactor<caster> &inputReactor)
{
while(true)
{//Process an event if its time is less than the current timestamp

if(inputReactor.eventQueue.size() == 0)
{//No events left, so return negative
return Poco::Timestamp(-1);
}

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();

if(inputReactor.eventQueue.top().time > currentTime )
{ //Next event isn't happening yet
return inputReactor.eventQueue.top().time;
}

//There is an event to process
event eventToProcess = inputReactor.eventQueue.top();
inputReactor.eventQueue.pop(); //Remove event from queue


//Process events
if(eventToProcess.HasExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field))
{ //possible_base_station_event_timeout
possible_base_station_event_timeout eventInstance = eventToProcess.GetExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field);

if((connectionIDToConnectionStatus.at(eventInstance.connection_id()).timeLastMessageWasReceived.epochMicroseconds() + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0) <= eventToProcess.time.epochMicroseconds())
{//It has been more than SECONDS_BEFORE_CONNECTION_TIMEOUT since a message was received, so drop connection
if(eventInstance.is_authenticated())
{
SOM_TRY //Remove from database
removeAuthenticatedConnection(eventInstance.connection_id(), inputReactor);
SOM_CATCH("Error, unable to remove authenticated connection\n")
continue;
}
else
{
SOM_TRY
removeUnauthenticatedConnection(eventInstance.connection_id(), inputReactor);
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
removeConnectionKey(eventToProcess.GetExtension(connection_key_timeout_event::connection_key_timeout_event_field).connection_key(), inputReactor);
SOM_CATCH("Error removing connection key\n")
continue;
}

if(eventToProcess.HasExtension(signing_key_timeout_event::signing_key_timeout_event_field))
{ //A signing key has timed out, so remove it
SOM_TRY
removeSigningKey(eventToProcess.GetExtension(signing_key_timeout_event::signing_key_timeout_event_field).key(), inputReactor);
SOM_CATCH("Error removing signing key\n")
continue;
}

if(eventToProcess.HasExtension(update_statistics_event::update_statistics_event_field))
{ //It is time to update the real update rates in the database
//Update database lambda
zmq::socket_t *statisticsDatabaseRequestSocket = nullptr;

SOM_TRY
statisticsDatabaseRequestSocket = inputReactor.getSocket("statisticsDatabaseRequestSocket");
SOM_CATCH("Error resolving socket")

auto updateBasestationEntryLambda = [&] (int64_t inputBasestationID, double inputRealUpdateRate)
{
//Send registration request to database
std::string serializedDatabaseRequest;
database_request databaseRequest;
databaseRequest.set_base_station_to_update_id(inputBasestationID);
databaseRequest.set_real_update_rate(inputRealUpdateRate);

databaseRequest.SerializeToString(&serializedDatabaseRequest);

SOM_TRY
statisticsDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
statisticsDatabaseRequestSocket->send(serializedDatabaseRequest.c_str(), serializedDatabaseRequest.size());
SOM_CATCH("Error sending database request\n")
};


Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();

if(basestationIDToCreationTime.size() < UPDATE_RATES_TO_UPDATE_PER_SECOND)
{ //We can just update all of the entries
for(auto iter = basestationIDToCreationTime.begin(); iter != basestationIDToCreationTime.end(); iter++)
{
double updateRate = basestationIDToNumberOfSentMessages.at(iter->first)/((timeValue-iter->second)*1000000.0);
SOM_TRY
updateBasestationEntryLambda(iter->first, updateRate);
SOM_CATCH("Error updating update rate")
} 
mapUpdateIndex = 0;
}
else
{ //Update subset
if(mapUpdateIndex >= basestationIDToCreationTime.size())
{
mapUpdateIndex = 0;
}

auto iter = basestationIDToCreationTime.begin();
for(int i=0; i<mapUpdateIndex; i++)
{
iter++; //Advance until the index is met
}

int count = 0; //Number of updates performed
for(; iter != basestationIDToCreationTime.end() && count < UPDATE_RATES_TO_UPDATE_PER_SECOND; count++)
{
if(iter == basestationIDToCreationTime.end())
{ //Modulo increment
iter = basestationIDToCreationTime.begin();
}

double updateRate = basestationIDToNumberOfSentMessages.at(iter->first)/((timeValue-iter->second)*1000000.0);
SOM_TRY
updateBasestationEntryLambda(iter->first, updateRate);
SOM_CATCH("Error updating update rate")

iter++;
mapUpdateIndex++; //Update index
} 


}

//add new update_statistics_event to trigger next update
update_statistics_event updateEventSubMessage;

event updateEvent(timeValue + 1000000.0); //Active in 1 second
(*updateEvent.MutableExtension(update_statistics_event::update_statistics_event_field)) = updateEventSubMessage;

inputReactor.eventQueue.push(updateEvent);
}

if(eventToProcess.HasExtension(possible_proxy_stream_timeout_event::possible_proxy_stream_timeout_event_field))
{ //Check if a proxy stream has timed out and delete it if so
int64_t foreignCasterID = eventToProcess.GetExtension(possible_proxy_stream_timeout_event::possible_proxy_stream_timeout_event_field).caster_id();
int64_t foreignStreamID = eventToProcess.GetExtension(possible_proxy_stream_timeout_event::possible_proxy_stream_timeout_event_field).stream_id();
int64_t localStreamID = 0;

//Ignore timeout if the foreign stream isn't in the map
if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.count(foreignCasterID) != 0)
{
if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(foreignCasterID).count(foreignStreamID) == 0)
{
continue;
}
}
else
{
continue;
}

localStreamID = casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(foreignCasterID).at(foreignStreamID);

if(localBasestationIDToLastMessageTimestamp.count(localStreamID) == 0)
{
continue;
}

Poco::Timestamp currentTime;

if((currentTime - eventToProcess.time) > SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0)
{ //Delete basestation
SOM_TRY
deleteProxyStream(inputReactor, foreignCasterID, foreignStreamID, BASE_STATION_TIMED_OUT);
SOM_CATCH("Error removing proxy basestation\n")
}  

}//end possible_proxy_stream_timeout_event

}//end while

}


/**
This function adds a authenticated connection by placing it in the associated maps/sets and the database.  The call is ignored if the connection key is not found in connectionKeyToSigningKeys, so addConnectionKey should have been called first for the connection key.
@param inputConnectionID: The ZMQ connection ID string of the connection to add
@param inputConnectionKey: The ZMQ CURVE key that is being used with this connection (connection key)
@param inputConnectionStatus: The current status of the connection
@param inputBaseStationStreamInfo: The connection's details to register with the database
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::addAuthenticatedConnection(const std::string &inputConnectionID,  const std::string &inputConnectionKey, const connectionStatus &inputConnectionStatus, const base_station_stream_information &inputBaseStationStreamInfo, reactor<caster> &inputReactor)
{
if(connectionKeyToSigningKeys.count(inputConnectionID))
{
return; //Connection key entry not found, so the connection cannot be registered
}

zmq::socket_t *registrationDatabaseRequestSocket;
SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error getting socket\n")

//Add to maps/sets
authenticatedConnectionIDToConnectionKey.emplace(inputConnectionID, inputConnectionKey);
connectionKeyToAuthenticatedConnectionIDs.emplace(inputConnectionKey, inputConnectionID);
connectionIDToConnectionStatus[inputConnectionID] = inputConnectionStatus;

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

inputReactor.eventQueue.push(timeoutEvent);
}

/**
This function removes a authenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::removeAuthenticatedConnection(const std::string &inputConnectionID, reactor<caster> &inputReactor)
{
//Check if it has already been erased
if(authenticatedConnectionIDToConnectionKey.count(inputConnectionID) == 0)
{
return;
}

zmq::socket_t *registrationDatabaseRequestSocket = nullptr;
SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error, unable to get socket\n")

//Remove from maps/sets
std::string connectionKey(authenticatedConnectionIDToConnectionKey.at(inputConnectionID));
authenticatedConnectionIDToConnectionKey.erase(inputConnectionID);

removeKeyValuePairFromStringMultimap(connectionKeyToAuthenticatedConnectionIDs, connectionKey, inputConnectionID);

auto basestationID = connectionIDToConnectionStatus.at(inputConnectionID).baseStationID;
connectionIDToConnectionStatus.erase(inputConnectionID);

//Remove from database
std::string serializedDatabaseRequest;
database_request databaseRequest;
databaseRequest.add_delete_base_station_ids(basestationID);

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
@param inputReactor: The reactor that is calling the function

@return: true if the key had valid signing keys and was added
*/
bool caster::addConnectionKey(const std::string &inputConnectionKey, int64_t inputExpirationTime, const std::vector<std::string> &inputSigningKeys, reactor<caster> &inputReactor)
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

if(listOfOfficialSigningKeys.size() == 0 && listOfRegisteredCommunitySigningKeys.size() == 0)
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

inputReactor.eventQueue.push(timeoutEvent);
}

return true;
}

/**
This function removes the given connection key from the maps and removes all associated connections.
@param inputConnectionKey: The connection key to remove
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::removeConnectionKey(const std::string &inputConnectionKey, reactor<caster> &inputReactor)
{
//Remove/delete all affiliated connections
auto equal_range = connectionKeyToAuthenticatedConnectionIDs.equal_range(inputConnectionKey);
for(auto iter = equal_range.first; iter != equal_range.second;)
{
auto buffer = iter++; //Iter's entry gets deleted, so we have to increment before that happens
SOM_TRY
removeAuthenticatedConnection(iter->second, inputReactor);
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
@param inputReactor: The reactor that is calling the function

@return: true if the key was added successfully or is already present
*/
bool caster::addSigningKey(const std::string &inputSigningKey, bool inputIsOfficialSigningKey, int64_t inputExpirationTime, reactor<caster> &inputReactor)
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

inputReactor.eventQueue.push(timeoutEvent);
}

/**
This function removes a signing key.  This can cause a cascade where a connection key which is reliant on it is removed, which can in turn cause many connections to be removed.
@param inputSigningKey: The signing key to remove
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::removeSigningKey(const std::string &inputSigningKey, reactor<caster> &inputReactor)
{
//Remove any affected connection keys and delete all references to this key
auto equal_range = signingKeyToConnectionKeys.equal_range(inputSigningKey);
for(auto iter = equal_range.first; iter != equal_range.second;)
{
auto buffer = iter++; //Object refered to by iter will be removed
if(connectionKeyToSigningKeys.count(iter->second) < 2)
{//If this connection key is only signed by this signing key, remove it
SOM_TRY
removeConnectionKey(iter->second, inputReactor);
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
@param inputEventQueue: The queue to to add the expiration event to
@param inputReactor: The reactor that is calling the function

@throws: this function can throw exceptions
*/
void caster::addBlacklistKey(const std::string &inputBlacklistKey, int64_t inputExpirationTime, reactor<caster> &inputReactor)
{
SOM_TRY //Remove the key from list of signing keys
removeSigningKey(inputBlacklistKey, inputReactor);
SOM_CATCH("Error removing blacklist key\n")

//Add to the list of blacklisted keys
blacklistedSigningKeys.insert(inputBlacklistKey);

//Add timeout event
blacklist_key_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_blacklist_key(inputBlacklistKey);

event timeoutEvent(inputExpirationTime);
(*timeoutEvent.MutableExtension(blacklist_key_timeout_event::blacklist_key_timeout_event_field)) = timeoutEventSubMessage;

inputReactor.eventQueue.push(timeoutEvent);
}

/**
This function removes a unauthenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::removeUnauthenticatedConnection(const std::string &inputConnectionID, reactor<caster> &inputReactor)
{
//Check if it has already been erased
if(connectionIDToConnectionStatus.count(inputConnectionID) == 0)
{
return;
}

zmq::socket_t *registrationDatabaseRequestSocket = nullptr;
SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error getting socket\n")

//Remove from maps/sets
auto basestationID = connectionIDToConnectionStatus.at(inputConnectionID).baseStationID;
connectionIDToConnectionStatus.erase(inputConnectionID);

//Remove from database
std::string serializedDatabaseRequest;
database_request databaseRequest;
databaseRequest.add_delete_base_station_ids(basestationID);

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
basestationToSQLInterface.reset(new messageDatabaseDefinition(*databaseConnection, *base_station_stream_information::descriptor()));
SOM_CATCH("Error, unable to intialize message/SQL interface\n")
}

/**
This function handles requests to add or remove a caster proxy.  The socket it handles is typically called "addRemoveProxiesSocket".
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processAddRemoveProxyRequest(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{


//Create lambda to make it easy to send request failed replies
auto sendReplyLambda = [&] (bool inputRequestFailed, enum proxy_request_failure_reason inputReason = PROXY_REQUEST_DESERIALIZATION_FAILED)
{
add_remove_proxy_reply reply;

if(inputRequestFailed)
{ //Only set the reason if the request failed
reply.set_reason(inputReason);
}

SOM_TRY
sendProtobufMessage(inputSocket, reply);
SOM_CATCH("Error sending reply\n")
};

//Receive request
bool messageReceived = false;
bool messageDeserializedCorrectly = false;
add_remove_proxy_request request;

SOM_TRY
std::tie(messageReceived, messageDeserializedCorrectly) = receiveProtobufMessage(inputSocket, request, ZMQ_DONTWAIT);
SOM_CATCH("Error receiving request\n")

if(!messageReceived)
{ //False alarm, no message to get
return false;
}

if(!messageDeserializedCorrectly)
{
SOM_TRY
sendReplyLambda(true, PROXY_REQUEST_DESERIALIZATION_FAILED);
SOM_CATCH("Error sending reply\n")
}

bool isAddRequest = request.has_client_request_connection_string() && request.has_connect_disconnect_notification_connection_string() && request.has_base_station_publishing_connection_string();

if(!(isAddRequest) && !(request.has_client_request_connection_string_for_caster_to_remove()) )
{
SOM_TRY //Isn't proper add request or proper remove request
sendReplyLambda(true, PROXY_REQUEST_FORMAT_INVALID);
SOM_CATCH("Error sending reply\n")
}

//Get sockets
zmq::socket_t *proxiesNotificationsListeningSocket = nullptr;
SOM_TRY
proxiesNotificationsListeningSocket = inputReactor.getSocket("proxiesNotificationsListeningSocket");
SOM_CATCH("Error getting socket\n")

if(request.has_client_request_connection_string_for_caster_to_remove() && !isAddRequest)
{
if(clientRequestConnectionStringToCasterConnectionStrings.count(request.client_request_connection_string_for_caster_to_remove()) == 0)
{ //We don't have that caster, so removal succeeded
SOM_TRY
sendReplyLambda(false, PROXY_REQUEST_FORMAT_INVALID);
SOM_CATCH("Error sending reply\n")
return false; 
}

auto casterConnectionStrings = clientRequestConnectionStringToCasterConnectionStrings.at(request.client_request_connection_string_for_caster_to_remove());

zmq::socket_t *proxiesUpdatesListeningSocket = nullptr;
SOM_TRY
proxiesUpdatesListeningSocket = inputReactor.getSocket("proxiesUpdatesListeningSocket");
SOM_CATCH("Error getting socket\n")

SOM_TRY //Disconnect from caster
proxiesUpdatesListeningSocket->disconnect(std::get<2>(casterConnectionStrings).c_str());
SOM_CATCH("Error disconnecting socket\n")

SOM_TRY //Disconnect from caster
proxiesNotificationsListeningSocket->disconnect(std::get<1>(casterConnectionStrings).c_str());
SOM_CATCH("Error disconnecting socket\n")

//Removal of basestations will be handled by timeout mechanism

SOM_TRY //Operation succeeded, so inform requester
sendReplyLambda(false, PROXY_REQUEST_FORMAT_INVALID);
SOM_CATCH("Error sending reply\n")
return false; 
}

//Handle add request

//Connect to the notification socket and update socket so that updates from new foreign basestations  can be handled 

SOM_TRY
proxiesNotificationsListeningSocket->connect(request.connect_disconnect_notification_connection_string().c_str());
SOM_CATCH("Error connecting proxiesNotificationsListeningSocket\n")

zmq::socket_t *proxiesUpdatesListeningSocket = nullptr;
SOM_TRY
proxiesUpdatesListeningSocket = inputReactor.getSocket("proxiesUpdatesListeningSocket");
SOM_CATCH("Error getting socket\n")

SOM_TRY //Disconnect from caster
proxiesUpdatesListeningSocket->connect(request.base_station_publishing_connection_string().c_str());
SOM_CATCH("Error connecting update listening socket\n")

//Update map
clientRequestConnectionStringToCasterConnectionStrings.emplace(request.client_request_connection_string(), std::tuple<std::string, std::string, std::string>(request.client_request_connection_string(), request.connect_disconnect_notification_connection_string(), request.base_station_publishing_connection_string()));

SOM_TRY
sendReplyLambda(false, PROXY_REQUEST_FORMAT_INVALID);
SOM_CATCH("Error sending reply\n")
return false; //Proxy addition succeded
}


/**
This function handles notifications of new or removed sockets from casters that this caster is proxying.  It expects to receive stream_status_update messages with caster ID and stream ID preappended.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::listenForProxyNotifications(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
//Receive request
bool messageReceived = false;
bool messageDeserializedCorrectly = false;
stream_status_update notification;

Poco::Int64 header[2] = {0,0};

SOM_TRY
std::tie(messageReceived, messageDeserializedCorrectly) = receiveProtobufMessage(inputSocket, notification, ZMQ_DONTWAIT, (char *) header, sizeof(Poco::Int64)*2);
SOM_CATCH("Error receiving request\n")

if(!messageReceived || !messageDeserializedCorrectly)
{ //False alarm, no valid message to process
return false;
}

//Get database socket
zmq::socket_t *registrationDatabaseRequestSocket = nullptr;
SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error getting required socket\n")

int64_t foreignCasterID = Poco::ByteOrder::fromNetwork(Poco::Int64(header[0]));
int64_t foreignStreamID = Poco::ByteOrder::fromNetwork(Poco::Int64(header[1]));
int64_t localStreamID = 0;

if(notification.has_base_station_removed())
{//This is an update about a removed basestation, so remove the associated entries

SOM_TRY
deleteProxyStream(inputReactor, foreignCasterID, foreignStreamID, notification.removal_reason());
SOM_CATCH("Error removing foreign basestation\n")

return false;
}

if(notification.has_new_base_station_info())
{ //This is an update about a new basestation that was added
localStreamID = getNewStreamID();

//Update maps
Poco::Timestamp currentTime;
int64_t timeValue = currentTime.epochMicroseconds();
localBasestationIDToLastMessageTimestamp[localStreamID] = timeValue; //Count notification as message

if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.count(foreignCasterID) == 0)
{ //Add map if there isn't one
casterIDToMapFromOriginalBasestationIDToLocalBasestationID[foreignCasterID] = std::map<int64_t, int64_t>();
}

casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(foreignCasterID).emplace(foreignStreamID, localStreamID);

//Send notification regarding new local stream
stream_status_update localCasterNotification;
*localCasterNotification.mutable_new_base_station_info() = *notification.mutable_new_base_station_info();
localCasterNotification.mutable_new_base_station_info()->set_base_station_id(localStreamID);

SOM_TRY
sendProtobufMessage(*streamStatusNotificationInterface, localCasterNotification);
SOM_CATCH("Error sending notification out about proxy stream addition\n")

//Update database
database_request databaseRequest;
*databaseRequest.mutable_base_station_to_register() = *localCasterNotification.mutable_new_base_station_info();

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
sendProtobufMessage(*registrationDatabaseRequestSocket, databaseRequest);
SOM_CATCH("Error sending database request\n")

//Add timeout event so it will be removed if it doesn't update within the allowed period
possible_proxy_stream_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_caster_id(foreignCasterID);
timeoutEventSubMessage.set_caster_id(foreignStreamID);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_proxy_stream_timeout_event::possible_proxy_stream_timeout_event_field)) = timeoutEventSubMessage;

inputReactor.eventQueue.push(timeoutEvent);

return false;
}


return false;
}

/**
This function handles updates from the foreign casters this caster has started proxying.  It expects binary blobs with casterID, streamID preappended.  It also schedules timeout events for each stream and updates their last message received times.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::listenForProxyUpdates(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
//Receive request
bool messageReceived = false;
zmq::message_t messageBuffer;

SOM_TRY
messageReceived = inputSocket.recv(&messageBuffer, ZMQ_DONTWAIT);
SOM_CATCH("Error, unable to receive message\n")

if(!messageReceived || messageBuffer.size() < sizeof(Poco::Int64)*2)
{
return false; //No message to get or too small
}

//Get time of reception
Poco::Timestamp currentTime;

int64_t foreignCasterID = Poco::ByteOrder::fromNetwork(((Poco::Int64 *) messageBuffer.data())[0]);
int64_t foreignStreamID = Poco::ByteOrder::fromNetwork(((Poco::Int64 *) messageBuffer.data())[1]);

//See if we have metadata for that entry
if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.count(foreignCasterID) != 0)
{
if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(foreignCasterID).count(foreignStreamID) == 0)
{
return false; //Don't have it, so ignore message
}
}
else
{
return false; //Don't have it, so ignore message
}

int64_t localID = casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(foreignCasterID).at(foreignStreamID);

//Replace header with local casterID/stream ID
((Poco::Int64 *) messageBuffer.data())[0] = Poco::ByteOrder::toNetwork(casterID);
((Poco::Int64 *) messageBuffer.data())[1] = Poco::ByteOrder::toNetwork(localID);

//Forward message
SOM_TRY
clientStreamPublishingInterface->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error sending message\n")

SOM_TRY
proxyStreamPublishingInterface->send(messageBuffer.data(), messageBuffer.size());
SOM_CATCH("Error sending message\n")

//Update last message received time
localBasestationIDToLastMessageTimestamp[localID] = currentTime;

//Schedule timeout associated with the current update
possible_proxy_stream_timeout_event timeoutEventSubMessage;
timeoutEventSubMessage.set_caster_id(foreignCasterID);
timeoutEventSubMessage.set_caster_id(foreignStreamID);

event timeoutEvent(currentTime + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_proxy_stream_timeout_event::possible_proxy_stream_timeout_event_field)) = timeoutEventSubMessage;

inputReactor.eventQueue.push(timeoutEvent);

return false;
}

/**
This function checks if the databaseAccessSocket has received a database_request message and (if so) processes the message and sends a database_reply in response.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processDatabaseRequest(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return false; //No message to be had
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
inputSocket.send(serializedReply.c_str(), serializedReply.size());
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
return false;
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
return false;
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
return false;
}

if(request.delete_base_station_ids_size() > 0)
{//Perform delete operation
for(int i=0; i<request.delete_base_station_ids_size(); i++)
{
SOM_TRY
basestationToSQLInterface->deleteMessage(request.delete_base_station_ids(i));
SOM_CATCH("Error deleting from database\n")
}


SOM_TRY
sendReplyLambda(false); //Request succeeded
SOM_CATCH("Error sending reply\n")
return false;
}

if(request.has_base_station_to_update_id() && request.has_real_update_rate())
{ //Perform update operation
SOM_TRY //TODO: Might want to double check field number
basestationToSQLInterface->update(request.base_station_to_update_id(), 9, request.real_update_rate());
SOM_CATCH("Error updating database\n")

SOM_TRY
sendReplyLambda(false); //Request succedded
SOM_CATCH("Error sending reply\n")
return false;
}

SOM_TRY
sendReplyLambda(true, DATABASE_REQUEST_FORMAT_INVALID); //Request failed
SOM_CATCH("Error sending reply\n")

return false;
}

/**
This function checks if the clientRequestInterface has received a client_query_request message and (if so) processes the message and sends a client_query_reply in response.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processClientQueryRequest(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
//Receive message
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return false; //No message to be had
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
inputSocket.send(serializedReply.c_str(), serializedReply.size());
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
return false;
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
return false;
SOM_CATCH("Error sending reply");
}


std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> clientQueryStatement(nullptr, &sqlite3_finalize);

SOM_TRY
prepareStatement(clientQueryStatement, sqlQueryString, *databaseConnection);
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
std::vector<int64_t> resultPrimaryKeys;
while(true)
{
if(stepReturnValue == SQLITE_DONE)
{ //All results for this field have been retrieved
break; 
}

resultPrimaryKeys.push_back((int64_t) sqlite3_column_int64(clientQueryStatement.get(), 0));

stepReturnValue = sqlite3_step(clientQueryStatement.get());

if(stepReturnValue != SQLITE_ROW && stepReturnValue != SQLITE_DONE)
{
throw SOMException("Error executing query\n", SQLITE3_ERROR, __FILE__, __LINE__);
}
}

std::vector<base_station_stream_information> results;
for(int i=0; i<resultPrimaryKeys.size(); i++)
{
base_station_stream_information messageBuffer;
results.push_back(messageBuffer);
SOM_TRY
basestationToSQLInterface->retrieve(resultPrimaryKeys[i], results.back());
SOM_CATCH("Error retrieving object associated with a query primary key\n")
}

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
for(int i=0; i<results.size(); i++)
{
results[i].set_uptime(timeValue-results[i].start_time());
}


SOM_TRY //Send back query results
sendReplyLambda(false, CLIENT_QUERY_REQUEST_DESERIALIZATION_FAILED, casterID, results);
SOM_CATCH("Error sending reply\n")

return false;
}

/**
This function processes messages from the transmitterRegistrationAndStreamingInterface.  A connection is expected to start with a transmitter_registration_request, to which this object replies with a transmitter_registration_reply.  Thereafter, the messages received are forwarded to the associated publisher interfaces until the publisher stops sending for an unacceptably long period (SECONDS_BEFORE_CONNECTION_TIMEOUT), at which point the object erases the associated the associated metadata and publishes that the base station disconnected.  In the authenticated case, the preapended signature is removed and checked.  If authentication fails, packet is dropped (eventually timing out).
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
zmq::socket_t *registrationDatabaseRequestSocket = nullptr;

SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error getting required socket\n")


//Send reply
auto sendReplyLambda = [&] (const std::string &inputAddress, bool inputRequestSucceeded, request_failure_reason inputFailureReason = MESSAGE_FORMAT_INVALID)
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
inputSocket.send(inputAddress.c_str(), inputAddress.size(), ZMQ_SNDMORE);
SOM_CATCH("Error sending reply messages\n")

SOM_TRY
inputSocket.send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply messages\n")
};


//Receive messages
std::vector<std::string> receivedContent;
bool messageRetrievalSuccessful = false;
SOM_TRY
messageRetrievalSuccessful = retrieveRouterMessage(inputSocket, receivedContent);
SOM_CATCH("Error retrieving router message\n")

if(!messageRetrievalSuccessful)
{ //Invalid message, so ignore
return false;
}


std::string connectionID = receivedContent[0];
//Get current time
Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();

//Check if 
bool connectionIsAuthenticated = false;

//See if this base station has been registered yet
if(connectionIDToConnectionStatus.count(connectionID) == 0)
{

//This connection has not been seen before, so it should have a transmitter_registration_request
//Attempt to deserialize
transmitter_registration_request request;
request.ParseFromArray(receivedContent[1].c_str(), receivedContent[1].size());

if(!request.IsInitialized())
{//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendReplyLambda(connectionID, false, MESSAGE_FORMAT_INVALID);
return false;
SOM_CATCH("Error sending reply");
}

credentials *credentialsPointer = nullptr;
if(request.has_transmitter_credentials())
{ //This is a request to register an authenticated connection
connectionIsAuthenticated = true;
credentialsPointer = request.mutable_transmitter_credentials();
}


base_station_stream_information *streamInfo = request.mutable_stream_info();

if(!streamInfo->has_message_format())
{//Missing a required field
SOM_TRY
sendReplyLambda(connectionID, false, MISSING_REQUIRED_FIELD);
return false;
SOM_CATCH("Error sending reply")
}


//Check permissions if authenticated
bool credentialsMessageIsValid = false;
bool signedByRecognizedOfficialKey = false;
bool signedByRecognizedCommunityKey = false;
authorized_permissions permissionsBuffer;
if(connectionIsAuthenticated)
{
std::tie(credentialsMessageIsValid, signedByRecognizedOfficialKey, signedByRecognizedCommunityKey) = checkCredentials(*credentialsPointer, permissionsBuffer);

if(!credentialsMessageIsValid)
{ //Either authorized_permissions could not be deserialized or one of the signatures didn't match or the key didn't match
SOM_TRY
sendReplyLambda(connectionID, false, CREDENTIALS_DESERIALIZATION_FAILED);
return false;
SOM_CATCH("Error sending reply");
}


if(permissionsBuffer.has_valid_until())
{ 
if(timeValue > permissionsBuffer.valid_until())
{ //Permissions recognized but expired
SOM_TRY
sendReplyLambda(connectionID, false, CREDENTIALS_DESERIALIZATION_FAILED);
return false;
SOM_CATCH("Error sending reply");
}
}
else
{
SOM_TRY
sendReplyLambda(connectionID, false, CREDENTIALS_DESERIALIZATION_FAILED);
return false;
SOM_CATCH("Error sending reply");
}


if(permissionsBuffer.public_key().size() != crypto_sign_PUBLICKEYBYTES) 
{ //The public key is not a valid size
SOM_TRY
sendReplyLambda(connectionID, false, CREDENTIALS_DESERIALIZATION_FAILED);
return false;
SOM_CATCH("Error sending reply");
}

//Connection appears to be valid, so add it
std::vector<std::string> signingKeys;

for(int i=0; i<credentialsPointer->signatures_size(); i++)
{
signingKeys.push_back(credentialsPointer->signatures(i).public_key());
}


//Add connection key
if(addConnectionKey(permissionsBuffer.public_key(), permissionsBuffer.valid_until(), signingKeys, inputReactor) != true)
{
SOM_TRY
sendReplyLambda(connectionID, false, INSUFFICIENT_PERMISSIONS);
return false;
SOM_CATCH("Error sending reply");
}
}

//Set caster assigned fields
if(!connectionIsAuthenticated)
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
sendReplyLambda(connectionID, false, INSUFFICIENT_PERMISSIONS);
return false;
SOM_CATCH("Error sending reply");
}
}



streamInfo->clear_signing_keys();
if(connectionIsAuthenticated)
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
if(!connectionIsAuthenticated)
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
if(connectionIsAuthenticated)
{
SOM_TRY
addAuthenticatedConnection(connectionID, permissionsBuffer.public_key(), associatedConnectionStatus, *streamInfo, inputReactor);
SOM_CATCH("Error adding authenticated connection\n")
}
else
{
connectionIDToConnectionStatus[connectionID] = associatedConnectionStatus;
}


if(!connectionIsAuthenticated)
{
//Register possible stream timeout event
possible_base_station_event_timeout timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_id(connectionID);
timeoutEventSubMessage.set_is_authenticated(connectionIsAuthenticated);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field)) = timeoutEventSubMessage;

inputReactor.eventQueue.push(timeoutEvent);
}


//Tell base station that its registration succeeded
SOM_TRY
sendReplyLambda(connectionID, true);
SOM_CATCH("Error sending registration succeeded message")

//Make message to announce new station
std::string serializedUpdateMessage;
stream_status_update updateMessage;
(*updateMessage.mutable_new_base_station_info()) = (*streamInfo);

updateMessage.SerializeToString(&serializedUpdateMessage);

//Preappend casterID, streamID
auto networkOrderCasterID = Poco::ByteOrder::toNetwork(Poco::Int64(casterID));
auto networkOrderStreamID = Poco::ByteOrder::toNetwork(Poco::Int64(streamID));

std::string notificiationMessage = std::string((const char *) &networkOrderCasterID, sizeof(networkOrderCasterID)) + std::string((const char *) &networkOrderStreamID, sizeof(networkOrderStreamID)) + serializedUpdateMessage;

//Send the message
SOM_TRY
streamStatusNotificationInterface->send(notificiationMessage.c_str(), notificiationMessage.size());
SOM_CATCH("Error publishing new station registration\n")

return false;//Registration finished
}//End station registration


//Base station has already been registered, so forward it and update the timeout info

//Mark if it is an authenticated connection
connectionIsAuthenticated = authenticatedConnectionIDToConnectionKey.count(connectionID) != 0;

if(connectionIsAuthenticated && receivedContent[1].size() < crypto_sign_BYTES)
{ //Authenticated message isn't long enough to have a signature, so ignore it
return false; 
}

if(connectionIsAuthenticated)
{//Check the signature
std::string messageSignature = receivedContent[1].substr(0,crypto_sign_BYTES);

if(crypto_sign_verify_detached((const unsigned char *) messageSignature.c_str(), (const unsigned char *) receivedContent[1].c_str() + crypto_sign_BYTES, receivedContent[1].size() - crypto_sign_BYTES, (const unsigned char *) authenticatedConnectionIDToConnectionKey.at(connectionID).c_str()) != 0) 
{ //Signature did not match, so ignore invalid message
return false;
}
}

//Allocate memory and copy the caster ID, stream ID and data to it
int totalMessageSize = 0;

if(connectionIsAuthenticated)
{
totalMessageSize = receivedContent[1].size() + sizeof(Poco::Int64)+sizeof(Poco::Int64) - crypto_sign_BYTES; //Subtract size of signature since it is not forwarded
}
else
{
totalMessageSize = receivedContent[1].size() + sizeof(Poco::Int64)+sizeof(Poco::Int64);
}

char *memoryBuffer = new char[totalMessageSize];
SOMScopeGuard memoryBufferScopeGuard([&]() { delete[] memoryBuffer; }); 

//Poco saves the day again (good serialization functions)
//Update map
Poco::Int64 streamID = 0;
streamID = connectionIDToConnectionStatus.at(connectionID).baseStationID;

*((Poco::Int64 *) memoryBuffer) = Poco::ByteOrder::toNetwork(Poco::Int64(casterID));
*(((Poco::Int64 *) memoryBuffer) + 1) = Poco::ByteOrder::toNetwork(Poco::Int64(streamID));

//Copy message to buffer
if(connectionIsAuthenticated)
{
memcpy((void *) &memoryBuffer[sizeof(Poco::Int64)*2], (void *) receivedContent[1].c_str() + crypto_sign_BYTES, receivedContent[1].size() - crypto_sign_BYTES);
}
else
{
memcpy((void *) &memoryBuffer[sizeof(Poco::Int64)*2], (void *) receivedContent[1].c_str(), receivedContent[1].size());
}

//Forward message
SOM_TRY
clientStreamPublishingInterface->send(memoryBuffer, totalMessageSize);
SOM_CATCH("Error, unable to forward message\n")

SOM_TRY
proxyStreamPublishingInterface->send(memoryBuffer, totalMessageSize);
SOM_CATCH("Error, unable to forward message\n")

//Update map and register potential timeout event
//Update map
connectionIDToConnectionStatus.at(connectionID).timeLastMessageWasReceived = timeValue;

//Register possible stream timeout event
possible_base_station_event_timeout timeoutEventSubMessage;
timeoutEventSubMessage.set_connection_id(connectionID);
timeoutEventSubMessage.set_is_authenticated(connectionIsAuthenticated);

event timeoutEvent(timeValue + SECONDS_BEFORE_CONNECTION_TIMEOUT*1000000.0);
(*timeoutEvent.MutableExtension(possible_base_station_event_timeout::possible_base_station_event_timeout_field)) = timeoutEventSubMessage;

inputReactor.eventQueue.push(timeoutEvent);
return false;
}


/**
This function processes reply messages sent to registrationDatabaseRequestSocket.  It expects database operations to succeed, so it throws an exception upon receiving a failure message.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processTransmitterRegistrationAndStreamingDatabaseReply(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive first message part (should be empty)
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving database reply message")

if(!messageReceived || messageBuffer->size() != 0 || !messageBuffer->more())
{
return false;
}

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

messageReceived = false;
SOM_TRY //Receive message
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving server registration message")

database_reply reply;
reply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(reply.has_reason())
{
throw SOMException("Database request failed (" +std::to_string((int) reply.reason())+ "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

return false;
}

/**
This function processes key_management_request messages and accordingly modifies the list of accepted signing keys.  If a connection is reliant on a dropped signing key (has no other valid signing keys), then it will be dropped when the signing key is taken out of circulation.
@param inputReactor: The reactor that is calling the function
@param inputSocket: The socket
@return: true if the polling cycle should restart before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::processKeyManagementRequest(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
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
inputSocket.send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive first message part (should be empty)
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving key registration message")

if(!messageReceived)
{
return false;
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
addBlacklistKey(changes.keys_to_add_to_blacklist(i), changes.keys_to_add_to_blacklist_valid_until(i), inputReactor);
}

for(int i=0; i<changes.official_signing_keys_to_add_size(); i++)
{
addSigningKey(changes.official_signing_keys_to_add(i), true, changes.official_signing_keys_to_add_valid_until(i), inputReactor);
}

for(int i=0; i<changes.registered_community_signing_keys_to_add_size(); i++)
{
addSigningKey(changes.registered_community_signing_keys_to_add(i), false,  changes.registered_community_signing_keys_to_add_valid_until(i), inputReactor);
}

SOM_TRY
sendReplyLambda(false); //Operation succeeded
SOM_CATCH("Error, unable to send reply")

return false;
}

/**
This function processes stream status notification messages from the streamStatusNotificationListener socket in the statistics gathering thread.
@param inputReactor: The reactor this function is processing messages for
@param inputSocket: The socket which probably has a message waiting
@return: true if the function would like the reactor message processing loop to start at the beginning before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::statisticsProcessStreamStatusNotification(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving key registration message")

if(!messageReceived)
{
return false;
}

if(messageBuffer->size() < 2*sizeof(Poco::Int64))
{
return false; //Message is invalid
}

Poco::Int64 casterID = Poco::ByteOrder::fromNetwork(*((Poco::Int64 *) messageBuffer->data()));
Poco::Int64 streamID = Poco::ByteOrder::fromNetwork(*(((Poco::Int64 *) messageBuffer->data())+1));

stream_status_update update;
update.ParseFromArray(((const char *) messageBuffer->data())+2*sizeof(Poco::Int64), messageBuffer->size() - 2*sizeof(Poco::Int64));

if(!update.IsInitialized())
{
return false; //Invalid message
}

if(update.has_new_base_station_info())
{ //Add basestation to maps
Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
basestationIDToCreationTime[streamID]  = timeValue;
basestationIDToNumberOfSentMessages[streamID] = 0;
}

if(update.has_base_station_removed())
{//Remove from maps
basestationIDToCreationTime.erase(update.base_station_removed());
basestationIDToNumberOfSentMessages.erase(update.base_station_removed());
}

return false;
}

/**
This function processes stream messages from the proxyStreamListener socket in the statistics gathering thread.
@param inputReactor: The reactor this function is processing messages for
@param inputSocket: The socket which probably has a message waiting
@return: true if the function would like the reactor message processing loop to start at the beginning before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::statisticsProcessStreamMessage(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving key registration message")

if(!messageReceived)
{
return false;
}

if(messageBuffer->size() < 2*sizeof(Poco::Int64))
{
return false; //Message is invalid
}

Poco::Int64 casterID = Poco::ByteOrder::fromNetwork(*((Poco::Int64 *) messageBuffer->data()));
Poco::Int64 streamID = Poco::ByteOrder::fromNetwork(*(((Poco::Int64 *) messageBuffer->data())+1));

//Got a message, so update associated map
if(basestationIDToNumberOfSentMessages.count(streamID) != 0)
{//Add one to the message count for that stream
basestationIDToNumberOfSentMessages[streamID]++;
}

return false;
}

/**
This function processes database_reply messages from the statisticsDatabaseRequestSocket socket in the statistics gathering thread.
@param inputReactor: The reactor this function is processing messages for
@param inputSocket: The socket which probably has a message waiting
@return: true if the function would like the reactor message processing loop to start at the beginning before processing any more messages

@throws: This function can throw exceptions
*/
bool caster::statisticsProcessDatabaseReply(reactor<caster> &inputReactor, zmq::socket_t &inputSocket)
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

bool messageReceived = false;
SOM_TRY //Receive first message part (should be empty)
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving database reply message")

if(!messageReceived || messageBuffer->size() != 0 || !messageBuffer->more())
{
return false;
}

SOM_TRY //Receive message
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

messageReceived = false;
SOM_TRY //Receive message
messageReceived = inputSocket.recv(messageBuffer.get(), ZMQ_DONTWAIT);
SOM_CATCH("Error receiving server registration message")

database_reply reply;
reply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(reply.has_reason())
{
throw SOMException("Database request failed (" +std::to_string((int) reply.reason())+ "\n", SQLITE3_ERROR, __FILE__, __LINE__);
}

return false;
}


/**
This function is used inside the streamRegistrationAndPublishingReactor to remove a foreign stream from consideration and publish the associated notification.
@param inputReactor: The reactor to communicate with
@param inputCasterID: The ID of the foreign caster
@param inputStreamID: The ID of the foreign stream
@param inputReason: The reason the stream was removed

@throw: This function can throw exceptions
*/
void caster::deleteProxyStream(reactor<caster> &inputReactor, int64_t inputCasterID, int64_t inputStreamID, base_station_removal_reason inputReason)
{

if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.count(inputCasterID) == 0)
{
return; //Can't remove what isn't there
}
else
{
if(casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(inputCasterID).count(inputStreamID) == 0)
{
return; //Can't remove what isn't there
}
}

int64_t localStreamID = casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(inputCasterID).at(inputStreamID);

//Remove from database
zmq::socket_t *registrationDatabaseRequestSocket = nullptr;

SOM_TRY
registrationDatabaseRequestSocket = inputReactor.getSocket("registrationDatabaseRequestSocket");
SOM_CATCH("Error getting required socket\n")

database_request databaseRequest;
databaseRequest.add_delete_base_station_ids(localStreamID);

SOM_TRY
registrationDatabaseRequestSocket->send(nullptr, 0, ZMQ_SNDMORE);
sendProtobufMessage(*registrationDatabaseRequestSocket, databaseRequest);
SOM_CATCH("Error sending database request\n")

//Send notification regarding local stream removal
stream_status_update localCasterNotification;
localCasterNotification.set_base_station_removed(localStreamID);
localCasterNotification.set_removal_reason(inputReason);

//Add preappended casterID, streamID
Poco::Int64 header[2] = {0, 0};
header[0] = Poco::ByteOrder::toNetwork(Poco::Int64(casterID));
header[1] = Poco::ByteOrder::toNetwork(Poco::Int64(localStreamID));

zmq::socket_t *streamStatusNotificationInterface = nullptr;

SOM_TRY
streamStatusNotificationInterface = inputReactor.getSocket("streamStatusNotificationInterface");
SOM_CATCH("Error getting required socket\n")

SOM_TRY
sendProtobufMessage(*streamStatusNotificationInterface, localCasterNotification, std::string((char *) header, sizeof(Poco::Int64)*2));
SOM_CATCH("Error sending notification out proxy stream removal\n")

//update maps
localBasestationIDToLastMessageTimestamp.erase(localStreamID);
casterIDToMapFromOriginalBasestationIDToLocalBasestationID.at(inputCasterID).erase(localStreamID);
}


/*
This function generates the complete query string required to get all of the ids of the stations that meet the query's requirements.
@param inputRequest: This is the request to generate the query string for
@param inputParameterCountBuffer: This is set to the total number of bound variables
@return: The query string

@throws: This function can throw exceptions
*/
std::string caster::generateClientQueryRequestSQLString(const client_query_request &inputRequest, int &inputParameterCountBuffer)
{//TODO: Probably going to have to double check this
//Construct SQL query string from client query request
std::string primaryKeyFieldName = basestationToSQLInterface->messageDescriptor->field(basestationToSQLInterface->primaryKeyFieldNumber)->name();

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
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).longitude_condition_size(); a++)
{ //Handle longitude conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "longitude", inputRequest.subqueries(i).longitude_condition(a).relation());
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).uptime_condition_size(); a++)
{ //Handle uptime conditions
//Have to flip relation due to startTime < requirement being related to uptime > requirement
subQueryString += generateRelationalSubquery(parameterCount > 0, "start_time", flipOperator(inputRequest.subqueries(i).uptime_condition(a).relation()));
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).real_update_rate_condition_size(); a++)
{ //Handle real update rate conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "real_update_rate", inputRequest.subqueries(i).real_update_rate_condition(a).relation());
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).expected_update_rate_condition_size(); a++)
{ //Handle real update rate conditions
subQueryString += generateRelationalSubquery(parameterCount > 0, "expected_update_rate", inputRequest.subqueries(i).expected_update_rate_condition(a).relation());
parameterCount++;
}


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
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).source_public_keys_size(); a++)
{ //Handle source public keys restrictions
if(parameterCount > 0)
{
subQueryString += " AND ";
}

subQueryString += "(source_public_key == ?)";
parameterCount++;
}

if(inputRequest.subqueries(i).has_circular_search_region())
{ //Handle requests for basestations within a radius of a particular location 35.779411, -78.648033
//SELECT * from coords WHERE id IN (SELECT id FROM (SELECT id, lat*lat + long*long AS distance FROM coords GROUP BY distance HAVING distance < 7740.0));
if(parameterCount > 0)
{
subQueryString += " AND ";
} //57.2957795130785 -> radian to degrees constant
subQueryString += "(base_station_id IN (SELECT base_station_id FROM (SELECT base_station_id, (6371000*acos(57.2957795130785*(cos(?)*cos(latitude)*cos(longitude-?) + sin(?)*sin(latitude)))) AS distance FROM " + basestationToSQLInterface->messageTableName + " GROUP BY distance HAVING distance <= ?)))";
//params Qlatitude, Qlongitude, Qlatitude, distance constraint value
parameterCount += 4;
}

subQueryString += ")";

if(subQueryString != " OR ()")
{ //Only add subquery if it contains clauses
subQueryStrings.push_back(subQueryString);
}

} //End subquery string generation

std::string queryString = "SELECT " + primaryKeyFieldName + " FROM " + basestationToSQLInterface->messageTableName;

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
bindFieldValueToStatement(*inputStatement, parameterCount+1, (int64_t) inputRequest.subqueries(i).acceptable_classes(a));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

//Handle acceptable_formats subquery
for(int a=0; a<inputRequest.subqueries(i).acceptable_formats_size(); a++)
{
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, (int64_t) inputRequest.subqueries(i).acceptable_formats(a));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}


for(int a=0; a<inputRequest.subqueries(i).latitude_condition_size(); a++)
{ //Handle latitude conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).latitude_condition(a).value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).longitude_condition_size(); a++)
{ //Handle longitude conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).longitude_condition(a).value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

Poco::Timestamp currentTime;
auto timeValue = currentTime.epochMicroseconds();
for(int a=0; a<inputRequest.subqueries(i).uptime_condition_size(); a++)
{ //Handle uptime conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, (int64_t) (timeValue-inputRequest.subqueries(i).uptime_condition(a).value()*1000000.0));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).real_update_rate_condition_size(); a++)
{ //Handle real_update_rate conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).real_update_rate_condition(a).value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).expected_update_rate_condition_size(); a++)
{ //Handle expected_update_rate conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).expected_update_rate_condition(a).value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

if(inputRequest.subqueries(i).has_informal_name_condition())
{ //Add informal name condition

SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).informal_name_condition().value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).base_station_id_condition_size(); a++)
{ //Handle base_station_id conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, (int64_t)  inputRequest.subqueries(i).base_station_id_condition(a).value());
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

for(int a=0; a<inputRequest.subqueries(i).source_public_keys_size(); a++)
{ //Handle source public key conditions
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).source_public_keys(a));
SOM_CATCH("Error binding statement\n")
parameterCount++;
}

if(inputRequest.subqueries(i).has_circular_search_region())
{ //Handle requests for basestations within a radius of a particular 
//subQueryString += "(base_station_id IN (SELECT base_station_id FROM (SELECT base_station_id, (6371000*acos(cos(?)*cos(latitude)*cos(longitude-?) + sin(?)*sin(latitude))) AS distance FROM " + basestationToSQLInterface->primaryTableName + " GROUP BY distance HAVING distance <= ?)))";
//params Qlatitude, Qlongitude, Qlatitude, distance constraint value
SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).circular_search_region().latitude());
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).circular_search_region().longitude());
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).circular_search_region().latitude());
SOM_CATCH("Error binding statement\n")
parameterCount++;

SOM_TRY
bindFieldValueToStatement(*inputStatement, parameterCount+1, inputRequest.subqueries(i).circular_search_region().radius());
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

inputAuthorizedPermissionsBuffer.ParseFromString(inputCredentials.permissions());

if(!inputAuthorizedPermissionsBuffer.IsInitialized())
{ //Authorized permissions are invalid
return std::tuple<bool, bool, bool>(false, isSignedByOfficialEntityKey, isSignedByRegisteredCommunityKey);
}

return std::tuple<bool, bool, bool>(true, isSignedByOfficialEntityKey, isSignedByRegisteredCommunityKey);
}


/**
This function removes a basestation connection from both the maps and the database.
@param inputConnectionID: The connection to remove
@param inputReactor: The reactor that is calling the function

@throws: This function can throw exceptions
*/
void caster::removeConnection(const std::string &inputConnectionID, reactor<caster> &inputReactor)
{

if(authenticatedConnectionIDToConnectionKey.count(inputConnectionID) > 0)
{ //Handle if authenticated
SOM_TRY
removeAuthenticatedConnection(inputConnectionID, inputReactor);
SOM_CATCH("Error, unable to remove authenticated connection\n")
}
else
{
removeUnauthenticatedConnection(inputConnectionID, inputReactor);
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



