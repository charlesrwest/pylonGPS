#ifndef  SOURCEMANAGERHPP
#define SOURCEMANAGERHPP

#include<future>
#include<memory>
#include<thread>
#include<chrono>
#include<map>
#include<vector>
#include<queue>
#include<string>
#include<tuple>
#include<functional>
#include<cstdio> //For debugging

#include "zmq.hpp"
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "streamSourceTableEntry.hpp"
#include "casterSourceTableEntry.hpp"
#include "networkSourceTableEntry.hpp"
#include "randomStringGenerator.hpp"
#include "utilityFunctions.hpp"

#include "ntrip_server_metadata_addition_request.pb.h"
#include "ntrip_server_metadata_addition_reply.pb.h"
#include "ntrip_server_registration_or_deregistraton_request.pb.h"
#include "ntrip_server_registration_or_deregistraton_reply.pb.h"
#include "ntrip_source_table_request.pb.h"
#include "ntrip_source_table_reply.pb.h"
#include "event.hpp"

namespace pylongps
{

enum sourceConnectionStatus
{
PENDING = 1, //Metadata received, but source has not yet connected
CONNECTED = 2,
DISCONNECTED = 3,
PERMANENT = 4 //Manually added and should never timeout
};

#define MANAGER_OPERATE_TIMEOUT_TIME_MILLISECONDS 100 //How long the manager thread should block before checking on things it is polling
#define MILLISECONDS_TO_WAIT_BEFORE_DROPPING_METADATA_PENDING 500 //How long to wait before dropping the metadata associated with pending connection
#define MILLISECONDS_TO_WAIT_BEFORE_DROPPING_METADATA_DISCONNECTED 500 //How long to wait before dropping the metadata associated with a connection that has disconnected
#define RANDOM_MOUNTPOINT_SIZE 10
#define RANDOM_PASSWORD_SIZE 10

/**
\ingroup Server
This class starts its own thread and is responsible for serverTCPConnections being able to register/deregister their data streams and clientTCPConnections being able to retrieve the source table and find what address (full connection string) to subscribe to for the stream associated with a particular mount point.

It carries out these roles by establishing 3 ZMQ socket endpoints (with inproc connection strings available as members of the object):

serverRegistrationDeregistrationSocket: a REP type socket which expects a ntrip_server_registration_or_deregistraton_request and replies with a ntrip_server_registration_or_deregistraton_reply.  These transactions allow the underlying source table to be updated and the disappearence of sources to be published.

mountpointDisconnectSocket: a PUB socket which emits a message consisting of a "MOUNTPOINT NAME" string.  These messages indicate that the information source for the stream associated with that mountpoint has disappeared and any client connections for that source should be dropped

sourceTableAccessSocket: a  REP socket which expects a ntrip_source_table_request and responses with a ntrip_source_table_reply.  If the ntrip_source_table_request string is empty or invalid, the reply will contain a string representing the NTRIP 1.0 source table.  If it has a valid mountpoint, it will return the complete string to use to do the ZMQ call to connect to the stream's publisher
*/
class sourceManager
{
public:
/**
This function initializes the object and starts a thread that conducts the operations of the router.  It does not return until all initialization in the thread is completed and all of the public members are safe to read.

@param inputZMQContext: The ZMQ context to use for inproc communications
@param inputPortNumber: The port number to bind for TCP based ZMQ requests to register source metadata with a dynamic port bound if the port number is negative (the resulting port number stored as a public variable).

@throws: This function can throw exceptions
*/
sourceManager(zmq::context_t *inputZMQContext, int inputPortNumber = -1);

//String for shutdown socket
std::string shutdownSocketConnectionString;

//Strings to use to connect to the ZMQ interfaces
std::string serverRegistrationDeregistrationSocketConnectionString;
std::string mountpointDisconnectSocketConnectionString;
std::string sourceTableAccessSocketConnectionString;

int serverMetadataAdditionSocketPortNumber;

/**
This function signals for the thread to shut down and then waits for it to do so.
*/
~sourceManager();

private:
/**
This function is run in a thread to perform the necessary operations to allow the 3 interfaces to process messages as they are suppose to and keep the source table up to date.  It is normally called in a thread created in the object constructor.
*/
void operate();

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
*/
void handlePossibleNtripServerMetadataAdditionRequest();

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
*/
void handlePossibleNtripServerRegistrationOrDeregistratonRequest();

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
*/
void handlePossibleNtripSourceTableRequest();

/**
This function processes any events that are scheduled to have occurred by now and returns when the next event is scheduled to occur.
@return: The time point associated with the soonest event timeout (1 minute from now if no events are waiting)

@throws: This function can throw exceptions
*/
std::chrono::steady_clock::time_point handleEvents();

/**
This function schedules the potential timeout of the associated metadata entry.  If the entry is not in a connected state when the timeout is processed, the metadata will be dropped.

@throws: This function can throw exceptions
*/
void scheduleMetadataTimeout(const std::string &inputMountpoint);

/**
This function generates a string that is the ascii serialization (NTRIP format) of the source table.
@return: The ascii serialized source table
*/
std::string generateSerializedSourceTable();


zmq::context_t *context;

//Thread for operations
std::unique_ptr<std::thread> operationsThread;
std::unique_ptr<zmq::socket_t> shutdownSocket; //This inproc PULL socket expects an empty message, which it takes as a indication that the thread should shut down. 


//Interfaces
std::unique_ptr<zmq::socket_t> serverMetadataAdditionSocket;
std::unique_ptr<zmq::socket_t> serverRegistrationDeregistrationSocket;
std::unique_ptr<zmq::socket_t> mountpointDisconnectSocket;
std::unique_ptr<zmq::socket_t> sourceTableAccessSocket;
std::unique_ptr<zmq::pollitem_t[]> pollItems; //The poll object used to poll the REP sockets
int numberOfPollItems; //Should always be 2

std::priority_queue<event, std::vector<event>, std::greater<event> > eventQueue;  //TODO: Finish implementing event queue functionality


//Metadata, source table and ZMQ information source strings
std::map<std::string, std::tuple<streamSourceTableEntry, sourceConnectionStatus, std::chrono::steady_clock::time_point > > mountpointToStreamSourceMetadata; //Softstate entries, info/connection-status/time of last update
std::map<std::string, std::string> mountpointToStreamSourcePassword; //Password a source must have to be able to start streaming as that mountpoint
std::map<std::string, streamSourceTableEntry> mountpointToStreamSourceTableEntry; //List of currently connected entries
std::map<std::string, std::string> mountpointToZMQConnectionString;
std::vector<casterSourceTableEntry> casterEntries;
std::vector<networkSourceTableEntry> networkEntries;
};



}
#endif 
