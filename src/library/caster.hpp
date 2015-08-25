#ifndef  CASTERHPP
#define CASTERHPP

#include "zmq.hpp"
#include <cstdint>
#include<memory>
#include<thread>
#include<chrono>
#include<queue>
#include<string>
#include<vector>
#include<set>
#include<random>
#include<cmath>
#include<cstring>
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "event.hpp"
#include "utilityFunctions.hpp"
#include "Poco/Timestamp.h"
#include "Poco/ByteOrder.h"
#include "protobufSQLConverter.hpp"
#include "sqlite3.h"
#include "connectionStatus.hpp"
#include <sodium.h>

#include "database_request.pb.h"
#include "database_reply.pb.h"
#include "client_query_request.pb.h"
#include "client_query_reply.pb.h"
#include "transmitter_registration_request.pb.h"
#include "transmitter_registration_reply.pb.h"
#include "possible_base_station_event_timeout.pb.h"
#include "stream_status_update.pb.h"
#include "credentials.pb.h"
#include "authorized_permissions.pb.h"
#include "key_management_request.pb.h"
#include "key_management_reply.pb.h"
#include "key_status_changes.pb.h"
#include "blacklist_key_timeout_event.pb.h"
#include "connection_key_timeout_event.pb.h"
#include "signing_key_timeout_event.pb.h"
#include "update_statistics_event.pb.h"
#include "add_remove_proxy_request.pb.h"
#include "add_remove_proxy_reply.pb.h"

namespace pylongps
{

//How long to wait before a connection is considered to have timed out and is closed
const double SECONDS_BEFORE_CONNECTION_TIMEOUT = 5.0; 

//How many basestation updates rates to update in the database per second
const int UPDATE_RATES_TO_UPDATE_PER_SECOND = 100;

//How long to wait for add proxy related requests
const int ADD_PROXY_MAX_WAIT_TIME = 5000; //5000 milliseconds

/**
This class represents a pylonGPS 2.0 caster.  It opens several ZMQ ports to provide caster services, an in-memory SQLITE database and creates 2 threads to manage its duties.

This class/program is used to distribute information from PylonGPS Transmitters and support proxying of any and all streams via other PylonGPS Casters to enable simple scaling.  Increasing scale can be handle by having all sources point toward a central Caster and then creating other casters as proxies to disseminate the received flows.  The associated star topology should be able to serve very large numbers of receiving clients (assuming the number of clients >> the number of transmitters).  If the number of transmitters gets too large, the star topology can be fragmented so there are separate instances with each taking a fraction of the transmitters.  The in that case, the proxies simply proxy multiple Casters.  Each Caster supports requests for filtered basestation results and returns a stream ID for clients to subscribe to.  The clients can then use the ZMQ subscribe mechanism to receive updates from one or more streams.

Please see the PylonGPS 2.0 documentation and specification for more details.
*/
class caster
{
public:
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
caster(zmq::context_t *inputContext, int64_t inputCasterID, uint32_t inputTransmitterRegistrationAndStreamingPortNumber, uint32_t inputClientRequestPortNumber, uint32_t inputClientStreamPublishingPortNumber, uint32_t inputProxyStreamPublishingPortNumber, uint32_t inputStreamStatusNotificationPortNumber, uint32_t inputKeyRegistrationAndRemovalPortNumber, const std::string &inputCasterPublicKey, const std::string &inputCasterSecretKey, const std::string &inputSigningKeysManagementKey, const std::vector<std::string> &inputOfficialSigningKeys, const std::vector<std::string> &inputRegisteredCommunitySigningKeys, const std::vector<std::string> &inputBlacklistedKeys, const std::string &inputCasterSQLITEConnectionString = "");

/**
This thread safe function adds a new caster to proxy.  The function may have some lag as a query needs to be sent to the caster to proxy and the results returned before this function does.
@param inputClientRequestConnectionString: The ZMQ connection string to use to connect to the client query answering port of the caster to proxy
@param inputBasestationPublishingConnectionString: The ZMQ connection string to use to connect to the interface that publishes the basestation updates
@param inputConnectionDisconnectionNotificationConnectionString: The ZMQ connection string to use to connect to the basestation connect/disconnect notification port on the caster to proxy
@return: The ID of the proxied caster

@throws: This function can throw exceptions
*/
int64_t addProxy(const std::string &inputClientRequestConnectionString, const std::string &inputBasestationPublishingConnectionString, const std::string &inputConnectDisconnectNotificationConnectionString);

/**
This function signals for the threads to shut down and then waits for them to do so.
*/
~caster();

zmq::context_t *context;
int64_t casterID;
uint32_t transmitterRegistrationAndStreamingPortNumber;
uint32_t clientRequestPortNumber;
uint32_t clientStreamPublishingPortNumber;
uint32_t proxyStreamPublishingPortNumber;
uint32_t streamStatusNotificationPortNumber;
std::string databaseConnectionString; //The connection string to use to connect to the associated SQLITE database
std::string casterPublicKey;
std::string addRemoveProxyConnectionString;

private:
std::string shutdownPublishingConnectionString; //string to use for inproc connection for receiving notifications for when the threads associated with this object should shut down
std::string databaseAccessConnectionString; //String to use for inproc connection to send requests to modify the database
std::string casterSecretKey;

//Owned by streamRegistrationAndPublishingThread
std::string signingKeysManagementKey; //The public key of the entity allowed to add/remove sigining keys (registration taken care of in streamRegistrationAndPublishingThread)
std::set<std::string> officialSigningKeys; //A list of acceptable signing keys for "Official" basestations
std::set<std::string> registeredCommunitySigningKeys; //A list of acceptable signing keys for "Registered Community" basestations
std::set<std::string> blacklistedSigningKeys; //A list of all signing keys that have been blacklisted

//See article on key management for how these maps are used
std::multimap<std::string, std::string> signingKeyToConnectionKeys;
std::multimap<std::string, std::string> connectionKeyToSigningKeys;
std::multimap<std::string, std::string> connectionKeyToAuthenticatedConnectionIDs;
std::map<std::string, std::string> authenticatedConnectionIDToConnectionKey;

//Used to keep track of the current status of each basestation connection
int64_t lastAssignedConnectionID = 0; //Incremented to make unique streamIDs
std::map<std::string, connectionStatus> connectionIDToConnectionStatus;

//Owned by statistics gathering thread
int mapUpdateIndex = 0; //The appropriate position to start in the map with the next update cycle.
std::map<int64_t, int64_t> basestationIDToCreationTime; //Resolves when basestation was made (Poco timestamp timevalue)
std::map<int64_t, int64_t> basestationIDToNumberOfSentMessages; //Keeps track of how many messages each basestation has sent

std::map<int64_t, std::map<int64_t, int64_t> > casterIDToMapForForeignBasestationIDToLocalBasestationID;

/**
This function is called in the clientRequestHandlingThread to handle client requests and manage access to the SQLite database.
*/
void clientRequestHandlingThreadFunction();

/**
This function is called in the streamRegistrationAndPublishingThread to handle stream registration and publishing updates.
*/
void streamRegistrationAndPublishingThreadFunction();

/**
This function monitors published updates, collects the associated statistics and periodically reports them to the database.
*/
void statisticsGatheringThreadFunction();

/**
This function processes any events that are scheduled to have occurred by now and returns when the next event is scheduled to occur.  Which thread is calling this function is determined by the type of events in the event queue.
@param inputEventQueue: The event queue to process events from
@return: The time point associated with the soonest event timeout (negative if there are no outstanding events)

@throws: This function can throw exceptions
*/
Poco::Timestamp handleEvents(std::priority_queue<pylongps::event> &inputEventQueue);

//Threads/shutdown socket for operations
std::unique_ptr<zmq::socket_t> shutdownPublishingSocket; //This inproc PUB socket publishes an empty message when it is time for threads to shut down.
//Set of sockets use to listen for the signal from shutdownPublishingSocket
std::unique_ptr<zmq::socket_t> clientRequestHandlingThreadShutdownListeningSocket;
std::unique_ptr<zmq::socket_t> streamRegistrationAndPublishingThreadShutdownListeningSocket;
std::unique_ptr<zmq::socket_t> statisticsGatheringThreadShutdownListeningSocket;

std::unique_ptr<zmq::socket_t> databaseAccessSocket; //A inproc REP socket that handles requests to make changes to the database.  Used by clientRequestHandlingThread.
std::unique_ptr<zmq::socket_t> registrationDatabaseRequestSocket; //A inproc dealer socket used in the streamRegistrationAndPublishingThread to send database requests/get replies
std::unique_ptr<zmq::socket_t> streamStatusNotificationListener; //A TCP SUB socket used in the statisticsGatheringThread to listen to the streamStatusNotificationInterface
std::unique_ptr<zmq::socket_t> proxyStreamListener; //A TCP SUB socket used in the statisticsGatheringThread to listen to the proxyStreamPublishingInterface
std::unique_ptr<zmq::socket_t> statisticsDatabaseRequestSocket; //A inproc dealer socket used in the statisticsGatheringThread to send database requests/get replies

std::unique_ptr<zmq::socket_t> addRemoveProxiesSocket; //A inproc REP socket which accepts add_remove_proxy_requests and responds with a add_remove_proxy_reply.  Used in the streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> proxiesUpdatesListeningSocket; //A TCP SUB socket which subscribes to other casters so that it can republish the updates it received.  Used in the streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> proxiesNotificationsListeningSocket; //A TCP SUB socket which subscribes to other casters so that it can know when they add/remove a basestation. Used in the streamRegistrationAndPublishingThread.


std::unique_ptr<std::thread> clientRequestHandlingThread; //Handles client requests and requests by the stream registration and statistics threads to make changes to the database
std::unique_ptr<std::thread> streamRegistrationAndPublishingThread;
std::unique_ptr<std::thread> statisticsGatheringThread; //This thread analyzes the statistics of the stream messages that are published and periodically updates the associated entries in the database.

std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> databaseConnection; //Pointer to created database connection
std::unique_ptr<protobufSQLConverter<base_station_stream_information> > basestationToSQLInterface; //Allows storage/retrieval of base_station_stream_information objects in the database

//Interfaces
std::unique_ptr<zmq::socket_t> transmitterRegistrationAndStreamingInterface; ///A ZMQ ROUTER socket which expects a transmitter_registration_request to which it responses with a transmitter_registration_reply. If accepted, the request is followed by the data to broadcast.  If the data is athenticated, the data message has a preappended sodium signature of length crypto_sign_BYTES.  Used by streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> keyRegistrationAndRemovalInterface; ///A ZMQ REP socket which expects a key_management_request message and sends back a key_management_reply message.  Used by streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> clientRequestInterface;  ///A ZMQ REP socket which expects a client_query_request and responds with a client_query_reply.  Used by clientRequestHandlingThread.
std::unique_ptr<zmq::socket_t> clientStreamPublishingInterface; ///A ZMQ PUB socket which publishes all data associated with all streams with the caster ID and stream ID preappended for clients to subscribe.  Used by streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> proxyStreamPublishingInterface; ///A ZMQ PUB socket which publishes all data associated with all streams with the caster ID and stream ID preappended for clients to subscribe.  Used by streamRegistrationAndPublishingThread.
std::unique_ptr<zmq::socket_t> streamStatusNotificationInterface; ///A ZMQ PUB socket which publishes stream_status_update messages.  Used by streamRegistrationAndPublishingThread.

//Functions and objects used internally
/**
This function adds a authenticated connection by placing it in the associated maps/sets and the database.  The call is ignored if the connection key is not found in connectionKeyToSigningKeys, so addConnectionKey should have been called first for the connection key.
@param inputConnectionID: The ZMQ connection ID string of the connection to add
@param inputConnectionKey: The ZMQ CURVE key that is being used with this connection (connection key)
@param inputConnectionStatus: The current status of the connection
@param inputBaseStationStreamInfo: The connection's details to register with the database
@param inputEventQueue: The queue to register the timeout associated with this connection (close if idle for X seconds).

@throws: This function can throw exceptions
*/
void addAuthenticatedConnection(const std::string &inputConnectionID,  const std::string &inputConnectionKey, const connectionStatus &inputConnectionStatus, const base_station_stream_information &inputBaseStationStreamInfo, std::priority_queue<event> &inputEventQueue);

/**
This function removes a authenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove

@throws: This function can throw exceptions
*/
void removeAuthenticatedConnection(const std::string &inputConnectionID);

/**
This function adds a connection signing key, updates the associated maps and schedules it to timeout if it has a timeout time.  Only signing keys which are already present will be acknowledged.
@param inputConnectionKey: The connection key to add
@param inputExpirationTime: The timestamp of when this key expires (negative if it does not expire on its own)
@param inputSigningKeys: The keys which have signed this 
@param inputEventQueue: The event queue to add the timeout event to

@return: true if the key had valid signing keys and was added
*/
bool addConnectionKey(const std::string &inputConnectionKey, int64_t inputExpirationTime, const std::vector<std::string> &inputSigningKeys, std::priority_queue<event> &inputEventQueue);

/**
This function removes the given connection key from the maps and removes all associated connections.
@param inputConnectionKey: The connection key to remove

@throws: This function can throw exceptions
*/
void removeConnectionKey(const std::string &inputConnectionKey);

/**
This function adds a new signing key and schedules its timeout.
@param inputSigningKey: The signing key to add
@param inputIsOfficialSigningKey: True if the signing key is an "Official" one and false if it is "Registered Community"
@param inputExpirationTime: When the signing key becomes invalid
@param inputEventQueue: The event queue to add the timeout event to

@return: true if the key was added successfully or is already present
*/
bool addSigningKey(const std::string &inputSigningKey, bool inputIsOfficialSigningKey, int64_t inputExpirationTime, std::priority_queue<event> &inputEventQueue);

/**
This function removes a signing key.  This can cause a cascade where a connection key which is reliant on it is removed, which can in turn cause many connections to be removed.
@param inputSigningKey: The signing key to remove

@throws: This function can throw exceptions
*/
void removeSigningKey(const std::string &inputSigningKey);

/**
Add a key to the blacklist, triggering its removal from "official" and "registered community" and the prevention of it from being reused until the certificate expires.
@param inputBlacklistKey: The key to place on the blacklist
@param inputExpirationTime: When the key expires
@param inputEventQueue: The queue to to ad the expiration event to

@throws: this function can throw exceptions
*/
void addBlacklistKey(const std::string &inputBlacklistKey, int64_t inputExpirationTime, std::priority_queue<event> &inputEventQueue);

/**
This function removes a unauthenticated connection and updates the associated datastructures.
@param inputConnectionID: The connection ID of the authenticated connection to remove

@throws: This function can throw exceptions
*/
void removeUnauthenticatedConnection(const std::string &inputConnectionID);

/**
This function removes a basestation connection from both the maps and the database.
@param inputConnectionID: The connection to remove

@throws: This function can throw exceptions
*/
void removeConnection(const std::string &inputConnectionID);

/**
This function sets up the basestationToSQLInterface and generates the associated tables so that basestations can be stored and returned.  databaseConnection must be setup before this function is called.

@throws: This function can throw exceptions
*/
void setupBaseStationToSQLInterface();

/**
This function handles requests to add or remove a caster proxy.  This generally consists of two part requests to add a proxy or single part requests to remove one.

@throws: This function can throw exceptions
*/
void processAddRemoveProxyRequest();

/**
This function checks if the databaseAccessSocket has received a database_request message and (if so) processes the message and sends a database_reply in response.

@throws: This function can throw exceptions
*/
void processDatabaseRequest();

/**
This function checks if the clientRequestInterface has received a client_query_request message and (if so) processes the message and sends a client_query_reply in response.

@throws: This function can throw exceptions
*/
void processClientQueryRequest();


/**
This function processes messages from the transmitterRegistrationAndStreamingInterface.  A connection is expected to start with a transmitter_registration_request, to which this object replies with a transmitter_registration_reply.  Thereafter, the messages received are forwarded to the associated publisher interfaces until the publisher stops sending for an unacceptably long period (SECONDS_BEFORE_CONNECTION_TIMEOUT), at which point the object erases the associated the associated metadata and publishes that the base station disconnected.  In the authenticated case, the preapended signature is removed and checked.  If authentication fails, packet is dropped (eventually timing out).
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void processAuthenticatedOrUnauthenticatedTransmitterRegistrationAndStreamingMessage(std::priority_queue<event> &inputEventQueue);

/**
This function processes reply messages sent to registrationDatabaseRequestSocket.  It expects database operations to succeed, so it throws an exception upon receiving a failure message.

@throws: This function can throw exceptions
*/
void processTransmitterRegistrationAndStreamingDatabaseReply();

/**
This function processes key_management_request messages and accordingly modifies the list of accepted signing keys.  If a connection is reliant on a dropped signing key (has no other valid signing keys), then it will be dropped when the signing key is taken out of circulation.
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void processKeyManagementRequest(std::priority_queue<event> &inputEventQueue);

/**
This function processes stream status notification messages from the streamStatusNotificationListener socket in the statistics gathering thread.
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void statisticsProcessStreamStatusNotification(std::priority_queue<event> &inputEventQueue);

/**
This function processes stream messages from the proxyStreamListener socket in the statistics gathering thread.
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void statisticsProcessStreamMessage(std::priority_queue<event> &inputEventQueue);

/**
This function processes database_reply messages from the statisticsDatabaseRequestSocket socket in the statistics gathering thread.
@param inputEventQueue: The event queue to register events to

@throws: This function can throw exceptions
*/
void statisticsProcessDatabaseReply(std::priority_queue<event> &inputEventQueue);

/**
This function generates the complete query string required to get all of the ids of the stations that meet the query's requirements.
@param inputRequest: This is the request to generate the query string for
@param inputParameterCountBuffer: This is set to the total number of bound variables
@return: The query string

@throws: This function can throw exceptions
*/
std::string generateClientQueryRequestSQLString(const client_query_request &inputRequest, int &inputParameterCountBuffer);

/**
This function binds the fields from the client_query_request to the associated prepared statement.
@param inputStatement: The statement to bind the fields for
@param inputRequest: The request to bind the fields with
@return: The number of bound parameters

@throws: This function can throw exceptions
*/
int bindClientQueryRequestFields(std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> &inputStatement, const client_query_request &inputRequest);

/**
This (non-threadsafe) function generates new unique (sequential) connection ids.
@return: A new connection ID to use
*/
int64_t getNewStreamID();

/**
This function checks whether the permissions in a credentials message are considered valid according to the current lists of trusted keys.  
@param inputCredentials: The credentials giving the permissions/signing keys
@param inputAuthorizedPermissionsBuffer: The object to return the authorized_permissions with
@return: A tuple of <messageIsValid, isSignedByOfficialEntityKey, isSignedByRegisteredCommunityKey>
*/
std::tuple<bool, bool, bool> checkCredentials(credentials &inputCredentials, authorized_permissions &inputAuthorizedPermissionsBuffer);
};

/**
This function makes it easier to process messages from a ZMQ router socket.  It retrieves the messages associated with a single router message (all parts of the multipart message) and stores them in the vector and returns true if the expected format was followed (3 part: addess, empty, message)
@param inputSocket: The socket to process messages from
@param inputMessageBuffer: The buffer to store messages in
@return: true if the message followed the expected format

@throws: This function can throw exceptions
*/
bool retrieveRouterMessage(zmq::socket_t &inputSocket, std::vector<std::string> &inputMessageBuffer);

/**
This function helps with creating SQL query strings for client requests.
@param inputRelation: The relation to resolve into a SQL string part (such as "<= ?"
@return: The associated SQL string component 
*/
std::string sqlRelationalOperatorToSQLString(sql_relational_operator inputRelation);


/**
This function helps with creating SQL query strings for client requests.
@param inputStringRelation: The relation to resolve into a SQL string part (such as "LIKE ?"
@return: The associated SQL string component 
*/
std::string sqlStringRelationalOperatorToSQLString(sql_string_relational_operator inputStringRelation);

/**
This function generates a sql query subpart of the form (fieldName IN (?, ?, etc))
@param inputPreappendAND: True if an " AND " should be added before the subquery part
@param inputFieldName: The field name to have conditions on
@param inputNumberOfFields: The number of entries in the "IN" set
@return: The query subpart
*/
std::string generateInSubquery(bool inputPreappendAND, const std::string &inputFieldName, int inputNumberOfFields);

/**
This function generates a sql query subpart of the form (fieldName IN (?, ?, etc))
@param inputPreappendAND: True if an " AND " should be added before the subquery part
@param inputFieldName: The field name to have conditions on
@param inputRelationalOperator: The relational operator to impose using the value
@return: The query subpart
*/
std::string generateRelationalSubquery(bool inputPreappendAND, const std::string &inputFieldName, sql_relational_operator inputRelationalOperator);

//SQLITE functions to add for support of great circle calculations
const double PI = 3.14159265359;
const double DEGREES_TO_RADIANS_CONSTANT = PI/180.0;

/**
This function flips the sign of the given relational operator.  Give >=, it will return <=, etc.
@param inputSQLRelationalOperator: The operator to flip
@return: The flipped operator
*/
sql_relational_operator flipOperator(sql_relational_operator inputSQLRelationalOperator);

/**
This function can be used to add the "sin" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void SQLiteSinFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues);

/**
This function can be used to add the "cos" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void SQLiteCosFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues);

/**
This function can be used to add the "acos" function to the current SQLite connection.
@param inputContext: The current SQLite context
@param inputArraySize: The number of values in the array
@param inputValues: The array values
*/
void SQLiteAcosFunctionDegrees(sqlite3_context *inputContext, int inputArraySize, sqlite3_value **inputValues);

/**
This function removes all entries of the map with the specific key/value.
@param inputMultimap: the multimap to delete from
@param inputKey: the key of the key/value pair
@param inputValue: The value of the key/value pair
*/
void removeKeyValuePairFromStringMultimap(std::multimap<std::string, std::string> &inputMultimap, const std::string &inputKey, const std::string &inputValue);

/**
This function calculates the signature for the given string/private signing key and preappends it to the message.
@param inputMessage: The message to sign
@param inputSigningSecretKey: The secret key to sign with (must be crypto_sign_SECRETKEYBYTES bytes)

@throws: This function can throw exceptions
*/
std::string calculateAndPreappendSignature(const std::string &inputMessage, const std::string &inputSigningSecretKey);

}
#endif
