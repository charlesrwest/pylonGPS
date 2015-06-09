#include "TCPServerConnectionFactoryImplementation.hpp" 

using namespace pylongps;

/**
This function initializes the factory and stores all of the passed information so that it can be given to the generated connection objects.
@param inputZMQContext: The ZMQ context that the generated connection objects will use
@param inputServerRegistrationDeregistrationSocketConnectionString: The connection string that can be used to connect to the source manager registration/deregistration ZMQ REP socket
@param inputMountpointDisconnectSocketConnectionString: The connection string that can be used to connect to the source manager ZMQ PUB socket which notifies about socket connects/disconnects
@param inputSourceTableAccessSocketConnectionString:  The connection string that can be used to connect to the source manager ZMQ REP socket to get information about sources (either the source table or a data stream address)
@param inputServerMetadataAdditionSocketPortNumber:   The loopback TCP port number of the source manager ZMQ REP port to use to register new metadata

@throws: This function can throw exceptions
*/
TCPServerConnectionFactoryImplementation::TCPServerConnectionFactoryImplementation(zmq::context_t *inputZMQContext, const std::string &inputServerRegistrationDeregistrationSocketConnectionString, const std::string &inputMountpointDisconnectSocketConnectionString, const std::string &inputSourceTableAccessSocketConnectionString, int inputServerMetadataAdditionSocketPortNumber)
{
context = inputZMQContext;
serverRegistrationDeregistrationSocketConnectionString = inputServerRegistrationDeregistrationSocketConnectionString;
mountpointDisconnectSocketConnectionString = inputMountpointDisconnectSocketConnectionString;
sourceTableAccessSocketConnectionString = inputSourceTableAccessSocketConnectionString;
serverMetadataAdditionSocketPortNumber = inputServerMetadataAdditionSocketPortNumber;

if(serverMetadataAdditionSocketPortNumber < 0)
{
throw SOMException("Invalid port number\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

serverMetadataAdditionSocketConnectionString = "tcp://127.0.0.1:" + std::to_string(serverMetadataAdditionSocketPortNumber);

}

/**
This function overrides the base class implementation and is called whenever a new TCPConnection handler needs to be made for the server.  Each connection is passed the information needed to connect to the source manager
@param inputConnectionSocket: The socket associated with the connection that the casterTCPConnectionHandler will handle
@return: Always returns a pointer to a casterTCPConnectionHandler, which handles the given connection

@throws: This function can throw exceptions
*/
Poco::Net::TCPServerConnection *TCPServerConnectionFactoryImplementation::createConnection(const Poco::Net::StreamSocket &inputConnectionSocket)
{
SOM_TRY
return new casterTCPConnectionHandler(inputConnectionSocket, context, serverRegistrationDeregistrationSocketConnectionString, mountpointDisconnectSocketConnectionString, sourceTableAccessSocketConnectionString, serverMetadataAdditionSocketPortNumber);
SOM_CATCH("Error initializing casterTCPConnectionHandler object\n")
}
