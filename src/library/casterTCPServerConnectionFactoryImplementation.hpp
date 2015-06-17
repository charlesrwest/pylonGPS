#ifndef  CASTERTCPSERVERCONNECTIONFACTORYIMPLEMENTATIONHPP
#define CASTERTCPSERVERCONNECTIONFACTORYIMPLEMENTATIONHPP

#include<Poco/Net/TCPServerConnectionFactory.h>
#include<string>
#include "casterTCPConnectionHandler.hpp"



namespace pylongps
{

/**
\ingroup Server
This class is  used to create new casterTCPConnectionHandler objects to deal with incoming connections.  It passes all of the information required (inproc connection strings and tcp port numbers) for the connection objects to be able to communicate with the source manager.
*/
class casterTCPServerConnectionFactoryImplementation : public Poco::Net::TCPServerConnectionFactory
{
public:
/**
This function initializes the factory and stores all of the passed information so that it can be given to the generated connection objects.
@param inputZMQContext: The ZMQ context that the generated connection objects will use
@param inputServerRegistrationDeregistrationSocketConnectionString: The connection string that can be used to connect to the source manager registration/deregistration ZMQ REP socket
@param inputMountpointDisconnectSocketConnectionString: The connection string that can be used to connect to the source manager ZMQ PUB socket which notifies about socket connects/disconnects
@param inputSourceTableAccessSocketConnectionString:  The connection string that can be used to connect to the source manager ZMQ REP socket to get information about sources (either the source table or a data stream address)
@param inputServerMetadataAdditionSocketPortNumber:   The loopback TCP port number of the source manager ZMQ REP port to use to register new metadata

@throws: This function can throw exceptions
*/
casterTCPServerConnectionFactoryImplementation(zmq::context_t *inputZMQContext, const std::string &inputServerRegistrationDeregistrationSocketConnectionString, const std::string &inputMountpointDisconnectSocketConnectionString, const std::string &inputSourceTableAccessSocketConnectionString, int inputServerMetadataAdditionSocketPortNumber);

/**
This function overrides the base class implementation and is called whenever a new TCPConnection handler needs to be made for the server.  Each connection is passed the information needed to connect to the source manager
@param inputConnectionSocket: The socket associated with the connection that the casterTCPConnectionHandler will handle
@return: Always returns a pointer to a casterTCPConnectionHandler, which handles the given connection

@throws: This function can throw exceptions
*/
Poco::Net::TCPServerConnection *createConnection(const Poco::Net::StreamSocket &inputConnectionSocket);

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
