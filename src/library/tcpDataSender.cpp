#include "tcpDataSender.hpp"

using namespace pylongps;

/**
This function initializes the tcpDataSender to publish data via a bound TCP port to any connecting clients.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputPortNumberToPublishOn: The TCP port number to bind/use for publishing

@throws: This function can throw exceptions
*/
tcpDataSender::tcpDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, int inputPortNumberToPublishOn) : context(inputContext)
{
if(inputPortNumberToPublishOn < 0)
{
throw SOMException("Received negative port number\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

informationSourceConnectionString = inputSourceConnectionString;
SOM_TRY //Make TCP socket for server to use
tcpSocket.reset(new Poco::Net::ServerSocket(inputPortNumberToPublishOn));
SOM_CATCH("Error, unable to create Poco server socket\n")

printf("Yoda %s\n", informationSourceConnectionString.c_str());

tcpDataSenderTCPServerConnectionFactoryImplementation *connectionFactory = nullptr;
SOM_TRY //Make connection factory for server to use
connectionFactory = new tcpDataSenderTCPServerConnectionFactoryImplementation(informationSourceConnectionString, context);
SOM_CATCH("Error, unable to make Poco connection factory\n")

Poco::Net::TCPServerParams *serverParameters = new Poco::Net::TCPServerParams; //TCPServer takes ownership

Poco::Net::ServerSocket *serverSocket = tcpSocket.get();

printf("Connection factory: %p\n", connectionFactory);
printf("Pointer serverSocket: %p\n", serverSocket);
printf("Pointer TCPServerParams: %p\n", serverParameters);

tcpSocket.release(); //Release ownership

printf("Yowsers\n");

SOM_TRY //tcpServer takes ownership of connection factory
tcpServer.reset(new Poco::Net::TCPServer(connectionFactory, *serverSocket, serverParameters));
SOM_CATCH("Error initializing Poco tcp server\n")

printf("Tau %p\n", tcpServer.get());

SOM_TRY
tcpServer->start();
SOM_CATCH("Error starting tcp server\n")
}


/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
std::string tcpDataSender::notificationAddress()
{
return notificationConnectionString;
}

/**
This function shuts down the Poco TCP server.
*/
tcpDataSender::~tcpDataSender()
{
printf("Nala\n");
tcpServer->stop();
}

/**
This function initializes the connection handler so that it can forward data received from the given source connection string.
@param inputConnectionSocket: The socket to forward data to
@param inputSourceConnectionString: The connection string to use for the data source
@param inputContext: The context to use with the source connection string

@throws: This function can throw exception
*/
tcpDataSenderTCPConnectionHandler::tcpDataSenderTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, const std::string &inputSourceConnectionString, zmq::context_t &inputContext) : Poco::Net::TCPServerConnection(inputConnectionSocket), context(inputContext), connectionSocket(inputConnectionSocket)
{
printf("Pala\n");
sourceConnectionString = inputSourceConnectionString;

SOM_TRY //Init socket
subscriberSocket.reset(new zmq::socket_t(context, ZMQ_SUB));
SOM_CATCH("Error making socket\n")

SOM_TRY //Set filter to allow any published messages to be received
subscriberSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting subscription for socket\n")

SOM_TRY //Connect to caster
subscriberSocket->connect(sourceConnectionString.c_str());
SOM_CATCH("Error connecting socket with info source\n")
}

/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object and forwards the received data over the connection.
*/
void tcpDataSenderTCPConnectionHandler::run()
{
printf("Makala\n");
std::unique_ptr<zmq::message_t> messageBuffer;
Poco::Net::StreamSocket &connection = socket();

while(true)
{
SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error making zmq message buffer\n")

SOM_TRY
if(subscriberSocket->recv(messageBuffer.get()) == false)
{
continue; //Nothing received
}
SOM_CATCH("Error, unable to forward data\n")

SOM_TRY
connection.sendBytes(messageBuffer->data(), messageBuffer->size());
SOM_CATCH("Error, couldn't send via Poco socket\n")
}
}

/**
This function initializes the factory with the values to pass to the connection handlers.
@param inputSourceConnectionString: The connection string to connect to a ZMQ_PUB socket which is sending information to forward
@param inputContext: The zmq context to use
*/
tcpDataSenderTCPServerConnectionFactoryImplementation::tcpDataSenderTCPServerConnectionFactoryImplementation(const std::string &inputSourceConnectionString, zmq::context_t &inputContext) : context(inputContext)
{
printf("Zumba\n");
sourceConnectionString = inputSourceConnectionString;
}

/**
This function creates a tcpDataSenderTCPConnectionHandler to handle a particular TCP connection.
@param inputConnectionSocket: The connection socket to forward the data over
*/
Poco::Net::TCPServerConnection *tcpDataSenderTCPServerConnectionFactoryImplementation::createConnection(const Poco::Net::StreamSocket &inputConnectionSocket)
{
printf("Plunta\n");
SOM_TRY
return new tcpDataSenderTCPConnectionHandler(inputConnectionSocket, sourceConnectionString, context);
SOM_CATCH("Error, unable to create connection handler\n")
}
