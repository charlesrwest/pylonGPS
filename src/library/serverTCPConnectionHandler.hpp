#ifndef  SERVERTCPCONNECTIONHANDLERHPP
#define SERVERTCPCONNECTIONHANDLERHPP

#include<Poco/Net/TCPServerConnection.h>
#include<Poco/Timespan.h>
#include<Poco/URI.h>
#include<Poco/Net/DNS.h>
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

#include "streamSourceTableEntry.hpp"
#include "sourceRegistrationRequestHeader.hpp"


namespace pylongps
{

const static int METADATA_REGISTRATION_TIMEOUT_DURATION = 1000; //Wait up to 1000 milliseconds for server response
const static unsigned int DATA_FORWARDING_BUFFER_SIZE = 512; //Size of buffer in bytes
const static int MAX_NTRIP_REGISTRATION_WAIT = 10; //Maximum wait for CASTER to respond to ntrip registration 

/**
\ingroup Server
This class takes a tcp connection and a caster URI/server metadata passed from the application to register with the caster and forward information received over the connection to the caster to be published.
*/
class serverTCPConnectionHandler : public Poco::Net::TCPServerConnection
{
public:
/**
This constructor passes the given socket to the base class and stores the information to register the associated metadata and connect to the caster.
@param inputConnectionSocket: The socket associated with the connection this object is processing
@param inputCasterURI: The URI (host:port) associated with the caster used to publish the data.
@param inputStreamSourceMetadata: The metadata associated with this source to put in the source table

@throws: This function can throw exceptions
*/
serverTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, const Poco::URI &inputCasterURI, const streamSourceTableEntry &inputStreamSourceMetadata); 


/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.
*/
void run();

Poco::URI casterURI;
streamSourceTableEntry metadata;
};


}
#endif
