#ifndef  SERVERTCPSERVERCONNECTIONFACTORYIMPLEMENTATIONHPP
#define SERVERTCPSERVERCONNECTIONFACTORYIMPLEMENTATIONHPP

#include<Poco/Net/TCPServerConnectionFactory.h>
#include<string>
#include "serverTCPConnectionHandler.hpp"

namespace pylongps
{

/**
\ingroup Server
This class is  used to create new serverTCPConnectionHandler objects to deal with incoming connections.  It passes all of the information required (caster URI and source metadata) for the connection objects to be able register and relay the information from the connection to the ntrip caster.
*/
class serverTCPServerConnectionFactoryImplementation : public Poco::Net::TCPServerConnectionFactory
{
public:
/**
This function initializes the factory and stores all of the passed information so that it can be given to the generated connection objects.
@param inputCasterURI: The URI (host:port) associated with the caster used to publish the data.
@param inputStreamSourceMetadata: The metadata associated with this source to put in the source table

@throws: This function can throw exceptions
*/
serverTCPServerConnectionFactoryImplementation(const Poco::URI &inputCasterURI, const streamSourceTableEntry &inputStreamSourceMetadata);

/**
This function overrides the base class implementation and is called whenever a new TCPConnection handler needs to be made for the server.  Each connection is passed the information needed to connect to the source manager
@param inputConnectionSocket: The socket associated with the connection that the serverTCPConnectionHandler will handle
@return: Always returns a pointer to a serverTCPConnectionHandler, which handles the given connection

@throws: This function can throw exceptions
*/
Poco::Net::TCPServerConnection *createConnection(const Poco::Net::StreamSocket &inputConnectionSocket);

Poco::URI casterURI;
streamSourceTableEntry metadata;
};
















}
#endif
