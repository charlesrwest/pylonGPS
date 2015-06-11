#ifndef  CASTERTCPCONNECTIONHANDLERHPP
#define CASTERTCPCONNECTIONHANDLERHPP

#include<Poco/Net/TCPServerConnection.h>
#include<Poco/Timespan.h>
#include<Poco/Net/HTTPRequest.h>
#include<string>
#include<chrono>
#include<cstring>
#include<cstdlib>
#include<sstream>  //Stringstream
#include<memory>
#include "SOMException.hpp"
#include<cstdio> //For debugging
#include<iostream> //For debugging
#include<Poco/StreamCopier.h>
#include "utilityFunctions.hpp"

#include "zmq.hpp"
#include "ntrip_server_metadata_addition_request.pb.h"
#include "ntrip_server_metadata_addition_reply.pb.h"
#include "ntrip_server_registration_or_deregistraton_request.pb.h"
#include "ntrip_server_registration_or_deregistraton_reply.pb.h"
#include "ntrip_source_table_request.pb.h"
#include "ntrip_source_table_reply.pb.h"

#include "sourceRegistrationRequestHeader.hpp"

namespace pylongps
{

#define HEADER_RECEIVE_TIMEOUT 500 //How long to wait for header before dropping the connection (milliseconds)
#define HEADER_BUFFER_SIZE 512 //How big header can be before we drop the connection
#define SOURCE_BUFFER_SIZE 512 //Size of receiving buffer before forwarding data from a source

/**
\ingroup Server
*/
class casterTCPConnectionHandler : public Poco::Net::TCPServerConnection
{
public:
/**
This constructor passes the given socket to the base class and stores the information required for this object to be able to connect to the ZMQ interfaces of the sourceManager.
@param inputConnectionSocket: The socket associated with the connection this object is processing
@param inputZMQContext: The ZMQ context that the generated connection objects will use
@param inputServerRegistrationDeregistrationSocketConnectionString: The connection string that can be used to connect to the source manager registration/deregistration ZMQ REP socket
@param inputMountpointDisconnectSocketConnectionString: The connection string that can be used to connect to the source manager ZMQ PUB socket which notifies about socket connects/disconnects
@param inputSourceTableAccessSocketConnectionString:  The connection string that can be used to connect to the source manager ZMQ REP socket to get information about sources (either the source table or a data stream address)
@param inputServerMetadataAdditionSocketPortNumber:   The loopback TCP port number of the source manager ZMQ REP port to use to register new metadata


@throws: This function can throw exceptions
*/
casterTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, zmq::context_t *inputZMQContext, const std::string &inputServerRegistrationDeregistrationSocketConnectionString, const std::string &inputMountpointDisconnectSocketConnectionString, const std::string &inputSourceTableAccessSocketConnectionString, int inputServerMetadataAdditionSocketPortNumber); 


/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.
*/
void run();

/**
This function is called to handle serving a SOURCE request.
@param inputHeaderString: The string containing the SOURCE request and possibly some body data
@param inputSocket: The socket to use to get/send data over the associated connection

@throws: This function can throw exceptions
*/
void handleSourceRequest(const std::string &inputHeaderString, Poco::Net::StreamSocket &inputSocket);

/**
This function is called to handle serving a HTTP request.
@param inputHeaderStream: A stringstream containing the http request and possibly some body data
@param inputSocket: The socket to use to get/send data over the associated connection

@throws: This function can throw exceptions
*/
void handleHTTPRequest(std::stringstream &inputHeaderStream, Poco::Net::StreamSocket &inputSocket);

//ZMQ context
zmq::context_t *context;

//Strings to use to connect to the ZMQ interfaces
std::string serverRegistrationDeregistrationSocketConnectionString;
std::string mountpointDisconnectSocketConnectionString;
std::string sourceTableAccessSocketConnectionString;
std::string serverMetadataAdditionSocketConnectionString;

int serverMetadataAdditionSocketPortNumber;
};


}
#endif 
