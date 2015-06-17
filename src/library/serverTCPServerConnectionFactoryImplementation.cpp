#include "serverTCPServerConnectionFactoryImplementation.hpp"

using namespace pylongps;

/**
This function initializes the factory and stores all of the passed information so that it can be given to the generated connection objects.
@param inputCasterURI: The URI (host:port) associated with the caster used to publish the data.
@param inputStreamSourceMetadata: The metadata associated with this source to put in the source table

@throws: This function can throw exceptions
*/
serverTCPServerConnectionFactoryImplementation::serverTCPServerConnectionFactoryImplementation(const Poco::URI &inputCasterURI, const streamSourceTableEntry &inputStreamSourceMetadata) : casterURI(inputCasterURI), metadata(inputStreamSourceMetadata)
{
}

/**
This function overrides the base class implementation and is called whenever a new TCPConnection handler needs to be made for the server.  Each connection is passed the information needed to connect to the source manager
@param inputConnectionSocket: The socket associated with the connection that the serverTCPConnectionHandler will handle
@return: Always returns a pointer to a serverTCPConnectionHandler, which handles the given connection

@throws: This function can throw exceptions
*/
Poco::Net::TCPServerConnection *serverTCPServerConnectionFactoryImplementation::createConnection(const Poco::Net::StreamSocket &inputConnectionSocket)
{
SOM_TRY
return new serverTCPConnectionHandler(inputConnectionSocket, casterURI, metadata);
SOM_CATCH("Error initializing serverTCPConnectionHandler object\n")
}
