#ifndef  SOURCEMANAGERHPP
#define SOURCEMANAGERHPP

#include<future>
#include<memory>
#include<thread>
#include<chrono>
#include<map>
#include<vector>
#include<string>
#include<tuple>
#include<cstdio> //For debugging

#include "zmq.hpp"
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "streamSourceTableEntry.hpp"
#include "casterSourceTableEntry.hpp"
#include "networkSourceTableEntry.hpp"

#include "ntrip_server_registration_or_deregistraton_request.pb.h"
#include "ntrip_server_registration_or_deregistraton_reply.pb.h"
#include "ntrip_source_table_request.pb.h"
#include "ntrip_source_table_reply.pb.h"

/*
This class starts its own thread and is responsible for serverTCPConnections being able to register/deregister their data streams and clientTCPConnections being able to retrieve the source table and find what address (full connection string) to subscribe to for the stream associated with a particular mount point.

It carries out these roles by establishing 3 ZMQ socket endpoints (with inproc connection strings available as members of the object):

serverRegistrationDeregistrationSocket: a REP type socket which expects a ntrip_server_registration_or_deregistraton_request and replies with a ntrip_server_registration_or_deregistraton_reply.  These transactions allow the underlying source table to be updated and the disappearence of sources to be published.

mountpointDisconnectSocket: a PUB socket which emits a message consisting of a "MOUNTPOINT NAME" string.  These messages indicate that the information source for the stream associated with that mountpoint has disappeared and any client connections for that source should be dropped

sourceTableAccessSocket: a  REP socket which expects a ntrip_source_table_request and responses with a ntrip_source_table_reply.  If the ntrip_source_table_request string is empty or invalid, the reply will contain a string representing the NTRIP 1.0 source table.  If it has a valid mountpoint, it will return the complete string to use to do the ZMQ call to connect to the stream's publisher
*/
class sourceManager
{
public:
/*
This function initializes the object and starts a thread that conducts the operations of the router.  It does not return until all initialization in the thread is completed and all of the public members are safe to read.

@param inputZMQContext: The ZMQ context to use for inproc communications

@exceptions: This function can throw exceptions
*/
sourceManager(zmq::context_t *inputZMQContext);

//Strings to use to connect to the ZMQ interfaces
std::string serverRegistrationDeregistrationSocketConnectionString;
std::string mountpointDisconnectSocketConnectionString;
std::string sourceTableAccessSocketConnectionString;

/*
This function signals for the thread to shut down and then waits for it to do so.
*/
~sourceManager();

private:
/*
This function is run in a thread to perform the necessary operations to allow the 3 interfaces to process messages as they are suppose to and keep the source table up to date.  It is normally called in a thread created in the object constructor.
*/
void operate();

/*
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@exceptions: This function can throw exceptions
*/
void handlePossibleNtripServerRegistrationOrDeregistratonRequest();

/*
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@exceptions: This function can throw exceptions
*/
void handlePossibleNtripSourceTableRequest();

/*
This function generates a string that is the ascii serialization (NTRIP format) of the source table.
@return: The ascii serialized source table
*/
std::string generateSerializedSourceTable();

zmq::context_t *context;
bool timeToShutdownFlag;  //Flag to indicate if the thread should return

//Thread for opertations and ZMQ sockets for interfaces
std::unique_ptr<std::thread> operationsThread; 
std::unique_ptr<zmq::socket_t> serverRegistrationDeregistrationSocket;
std::unique_ptr<zmq::socket_t> mountpointDisconnectSocket;
std::unique_ptr<zmq::socket_t> sourceTableAccessSocket;
std::unique_ptr<zmq::pollitem_t[]> pollItems; //The poll object used to poll the REP sockets
int numberOfPollItems; //Should always be 2

//Source table and ZMQ information source strings
std::map<std::string, streamSourceTableEntry> mountpointToStreamSourceTableEntry;
std::map<std::string, std::string> mountpointToZMQConnectionString;
std::vector<casterSourceTableEntry> casterEntries;
std::vector<networkSourceTableEntry> networkEntries;
};

/*
This function compactly allows binding a ZMQ socket to inproc address without needing to specify an exact address.  The function will try binding to addresses in the format: inproc://inputBaseString.inputExtensionNumberAsString and will try repeatedly while incrementing inputExtensionNumber until it succeeds or the maximum number of tries has been exceeded.
@param inputSocket: The ZMQ socket to bind
@param inputBaseString: The base string to use
@param inputExtensionNumber: The extension number to start with
@param inputMaximumNumberOfTries: How many times to try binding before giving up
@return: A tuple of form <connectionString ("inproc://etc"), extensionNumberThatWorked>

@exceptions: This function can throw exceptions if the bind call throws something besides "address taken" or the number of tries are exceeded
*/
std::tuple<std::string, int> bindZMQSocketWithAutomaticAddressGeneration(zmq::socket_t &inputSocket, const std::string &inputBaseString = "", int inputExtensionNumber = 0, unsigned int inputMaximumNumberOfTries = 1000);

#endif 
