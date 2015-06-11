#include "casterTCPConnectionHandler.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

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
casterTCPConnectionHandler::casterTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, zmq::context_t *inputZMQContext, const std::string &inputServerRegistrationDeregistrationSocketConnectionString, const std::string &inputMountpointDisconnectSocketConnectionString, const std::string &inputSourceTableAccessSocketConnectionString, int inputServerMetadataAdditionSocketPortNumber) : Poco::Net::TCPServerConnection(inputConnectionSocket)
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
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.
*/
void casterTCPConnectionHandler::run()
{
try
{
Poco::Net::StreamSocket &connection = socket();

std::chrono::steady_clock::time_point headerTimeoutTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(HEADER_RECEIVE_TIMEOUT);

//Retrieve header
int receivedDataSize = 0;
int receivedDataPosition = 0; //Offset to start writing the data in the buffer at
char receivedDataBuffer[HEADER_BUFFER_SIZE];


//Receive until either the maximum character buffer size is reached or the http header has been read
bool exitLoopFlag = false;
while(!exitLoopFlag) //TODO: This operation could be made more efficient
{
long int timeToWait = (headerTimeoutTime - std::chrono::steady_clock::now()).count()*1000; //Convert to microseconds to wait
if(timeToWait < 0)
{
return; //Header has timed out
}
connection.setReceiveTimeout(Poco::Timespan(0, timeToWait)); 

SOM_TRY
try
{
receivedDataSize = connection.receiveBytes((void *) (receivedDataBuffer+receivedDataPosition), HEADER_BUFFER_SIZE-receivedDataPosition);
receivedDataPosition += receivedDataSize; //Move write offset in response to getting more data
if(receivedDataSize == 0)
{
return; //Connection closed from client side
}
}
catch(const Poco::TimeoutException &inputException)
{
return; //Header connection timed out
}
SOM_CATCH("Error receiving header data from TCP connection")

if(receivedDataPosition >= HEADER_BUFFER_SIZE)
{
break; //Reached maximum header size, so try processing it
}

//Check if <CR><LF><CR><LF> ("\r\n\r\n") or <LF><LF> "\n\n" has been received
std::string receivedData(receivedDataBuffer, receivedDataPosition);

if(receivedData.find("\r\n\r\n") != std::string::npos || receivedData.find("\r\n\r\n") != std::string::npos)
{
break;
}

}



//SOURCE requests are not compliant with the HTTP standard, so special processing has to be used for them
std::string headerString(receivedDataBuffer, receivedDataPosition);

//Check if it is a SOURCE request or a normal HTTP request
if(headerString.find("SOURCE") == 0) //It's a SOURCE request, so offload it to a dedicated handler
{
SOM_TRY
handleSourceRequest(headerString, connection);
SOM_CATCH("Error handling source request\n")
return;
}

//Process header data
std::stringstream headerStream;
headerStream.write(receivedDataBuffer, receivedDataPosition);

SOM_TRY
handleHTTPRequest(headerStream, connection);
SOM_CATCH("Error handling HTTP request\n")
}
catch(const std::exception &inputException)
{
fprintf(stderr, "%s\n", inputException.what());
}
} 

/**
This function is called to handle serving a SOURCE request.
@param inputHeaderString: The string containing the SOURCE request and possibly some body data
@param inputSocket: The socket to use to get/send data over the associated connection

@throws: This function can throw exceptions
*/
void casterTCPConnectionHandler::handleSourceRequest(const std::string &inputHeaderString, Poco::Net::StreamSocket &inputSocket)
{
std::string illFormedHeaderRejectionString = "400 Bad Request\r\n\r\n";
std::string badPasswordRejectionString = "ERROR - Bad Password\r\n\r\n";
std::string OKMessageString = "ICY 200 OK\r\n\r\na"; 

//Check header. If invalid, send rejection message
sourceRegistrationRequestHeader header;
int numberOfHeaderCharactersUsed = header.parse(inputHeaderString);

if(numberOfHeaderCharactersUsed < 0)
{
inputSocket.sendBytes(illFormedHeaderRejectionString.c_str(), illFormedHeaderRejectionString.size());
return; //Invalid header
}

//Create and bind a inproc publisher address in preparation for registering 
std::unique_ptr<zmq::socket_t> sourcePublisher;
std::string sourcePublisherAddress;
int bindingNumber = rand() % 10000000;
SOM_TRY
sourcePublisher.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error initializing source publisher inproc socket\n")

SOM_TRY
std::tie(sourcePublisherAddress,bindingNumber) = bindZMQSocketWithAutomaticAddressGeneration(*(sourcePublisher), "ntripSourcePublisher", bindingNumber);
SOM_CATCH("Error binding socket for publisher\n")

//Create and connect socket to send request to sourceManager, with max wait of 1 second for reply
std::unique_ptr<zmq::socket_t> requestResponseSocket;
int timeoutDuration = 1000; //1000 milliseconds
SOM_TRY
requestResponseSocket.reset(new zmq::socket_t(*(context), ZMQ_REQ));
SOM_CATCH("Error initializing source registration socket\n")

SOM_TRY
requestResponseSocket->setsockopt(ZMQ_RCVTIMEO, &timeoutDuration, sizeof(timeoutDuration));
SOM_CATCH("Error setting socket timeout\n");

SOM_TRY
requestResponseSocket->connect(serverRegistrationDeregistrationSocketConnectionString.c_str());
SOM_CATCH("Error connecting to source manager\n")

//Create and serialize registration request
std::string serializedRegistrationRequest;
ntrip_server_registration_or_deregistraton_request registrationRequest;
registrationRequest.set_mountpoint(header.mountpoint);
registrationRequest.set_registering(true);
registrationRequest.set_connection_address(sourcePublisherAddress);
registrationRequest.set_password(header.password);
registrationRequest.SerializeToString(&serializedRegistrationRequest);

//Send registration request
SOM_TRY
requestResponseSocket->send(serializedRegistrationRequest.c_str(), serializedRegistrationRequest.size());
SOM_CATCH("Error sending registration request\n")

//Receive reply and wait up to 1 seconds for a reply before throwing an exception
std::unique_ptr<zmq::message_t> messageBuffer;
SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY
requestResponseSocket->recv(messageBuffer.get());
SOM_CATCH("Error getting source manager response\n")

//Deserialize reply
ntrip_server_registration_or_deregistraton_reply registrationReply;
registrationReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!registrationReply.IsInitialized())
{
throw SOMException("Source manager sent invalid reply message\n", UNKNOWN, __FILE__, __LINE__);
}

//Send rejection message if the source manager declined the registration
if(registrationReply.request_succeeded() == false)
{
inputSocket.sendBytes(badPasswordRejectionString.c_str(), badPasswordRejectionString.size());
return; //Assume bad password (could get more sophisticated later)
}
else
{//Send source accepted string
inputSocket.sendBytes(OKMessageString.c_str(), OKMessageString.size());
}

//Send remaining information from string with header, then publish data stream until the connection closes
SOM_TRY
sourcePublisher->send(inputHeaderString.substr(numberOfHeaderCharactersUsed, inputHeaderString.size() - numberOfHeaderCharactersUsed).c_str(), inputHeaderString.size() - numberOfHeaderCharactersUsed);
SOM_CATCH("Error publishing source data\n")

//Make a lambda to send deregistration request
auto sendDeregistrationRequest  = [&] ()
{
//Create and serialize registration request
std::string serializedRegistrationRequest;
ntrip_server_registration_or_deregistraton_request registrationRequest;
registrationRequest.set_mountpoint(header.mountpoint);
registrationRequest.set_registering(false);
registrationRequest.SerializeToString(&serializedRegistrationRequest);

//Send registration request
SOM_TRY
requestResponseSocket->send(serializedRegistrationRequest.c_str(), serializedRegistrationRequest.size());
SOM_CATCH("Error sending deregistration request\n")
};


int receivedDataSize = 0;
char dataBuffer[SOURCE_BUFFER_SIZE];
while(true)
{
try
{
receivedDataSize = inputSocket.receiveBytes((void *) (dataBuffer), SOURCE_BUFFER_SIZE);
}
catch(const std::exception &inputException)
{
SOM_TRY
sendDeregistrationRequest(); //Error occured with socket
SOM_CATCH("Error sending deregistration message\n")
return;
}

if(receivedDataSize == 0)
{
SOM_TRY //Connection closed normally
sendDeregistrationRequest();
SOM_CATCH("Error sending deregistration message\n")
break;
}

//Publish information from source table
SOM_TRY
sourcePublisher->send(dataBuffer, receivedDataSize);
SOM_CATCH("Error publishing source data\n")
}

}

/**
This function is called to handle serving a HTTP request.
@param inputHeaderStream: A stringstream containing the http request and possibly some body data
@param inputSocket: The socket to use to get/send data over the associated connection

@throws: This function can throw exceptions
*/
void casterTCPConnectionHandler::handleHTTPRequest(std::stringstream &inputHeaderStream, Poco::Net::StreamSocket &inputSocket)
{
std::string illFormedHeaderRejectionString = "400 Bad Request\r\n\r\n";
std::string OKMessageString = "ICY 200 OK\r\n\r\n"; 
std::string sourceOKMessageString = "SOURCETABLE 200 OK\r\n";

//Read and verify header
Poco::Net::HTTPRequest requestHeader;
try
{
requestHeader.read(inputHeaderStream);
}
catch(const std::exception &inputException)
{
inputSocket.sendBytes(illFormedHeaderRejectionString.c_str(), illFormedHeaderRejectionString.size());
return; //Invalid header, so send rejection string
}

//Call the right method to deal with the request
if(requestHeader.getMethod().find("GET") != std::string::npos)
{
if(requestHeader.get("User-Agent").find("NTRIP") == std::string::npos)
{
inputSocket.sendBytes(illFormedHeaderRejectionString.c_str(), illFormedHeaderRejectionString.size());
return; //Invalid header, so send rejection string
}

//Create subscriber socket and subscribe to source manager disconnect publisher to avoid race conditions  
std::unique_ptr<zmq::socket_t> disconnectNotificationsSocket;
SOM_TRY
disconnectNotificationsSocket.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error initializing disconnect notification socket\n")

SOM_TRY //Accept starting with the mountpoint
disconnectNotificationsSocket->setsockopt(ZMQ_SUBSCRIBE, requestHeader.getURI().c_str(), requestHeader.getURI().size());
SOM_CATCH("Error setting socket filter\n");

SOM_TRY
disconnectNotificationsSocket->connect(mountpointDisconnectSocketConnectionString.c_str());
SOM_CATCH("Error connecting to source manager\n")


//Create and connect socket to send request to sourceManager with, with max wait of 1 second for reply
std::unique_ptr<zmq::socket_t> requestResponseSocket;
int timeoutDuration = 1000; //1000 milliseconds
SOM_TRY
requestResponseSocket.reset(new zmq::socket_t(*(context), ZMQ_REQ));
SOM_CATCH("Error initializing source request socket\n")

SOM_TRY
requestResponseSocket->setsockopt(ZMQ_RCVTIMEO, &timeoutDuration, sizeof(timeoutDuration));
SOM_CATCH("Error setting socket timeout\n");

SOM_TRY
requestResponseSocket->connect(sourceTableAccessSocketConnectionString.c_str());
SOM_CATCH("Error connecting to source manager\n")

//Create and serialize request
std::string requestSerialized;
ntrip_source_table_request request;
request.set_mountpoint(requestHeader.getURI());
request.SerializeToString(&requestSerialized);

SOM_TRY //Send request to source manager
requestResponseSocket->send(requestSerialized.c_str(), requestSerialized.size());
SOM_CATCH("Error sending request to source manager\n")

std::unique_ptr<zmq::message_t> messageBuffer;
SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Wait up to 1 second to get results
requestResponseSocket->recv(messageBuffer.get());
SOM_CATCH("Error getting source manager response\n")

//Deserialize reply
ntrip_source_table_reply sourceTableReply;
sourceTableReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!sourceTableReply.IsInitialized())
{
throw SOMException("Source manager sent invalid reply message\n", UNKNOWN, __FILE__, __LINE__);
}

if(!sourceTableReply.has_source_table() && !sourceTableReply.has_stream_inproc_address())
{
throw SOMException("Source manager sent invalid reply message\n", UNKNOWN, __FILE__, __LINE__);
}

if(sourceTableReply.has_source_table())
{
//Send OK message and source table
inputSocket.sendBytes(sourceOKMessageString.c_str(), sourceOKMessageString.size());
inputSocket.sendBytes(sourceTableReply.source_table().c_str(), sourceTableReply.source_table().size());

return;
}

if(sourceTableReply.has_stream_inproc_address())
{//TODO: Check for possible race condition here due to inproc connection rules
//Create and connect subscriber for source stream
std::unique_ptr<zmq::socket_t> sourceSocket;
SOM_TRY
sourceSocket.reset(new zmq::socket_t(*(context), ZMQ_SUB));
SOM_CATCH("Error initializing source subscription socket\n")

SOM_TRY //Accept all published messages
sourceSocket->setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
SOM_CATCH("Error setting socket filter\n");

SOM_TRY
sourceSocket->connect(sourceTableReply.stream_inproc_address().c_str());
SOM_CATCH("Error connecting to source publisher\n")

int numberOfPollItems;
std::unique_ptr<zmq::pollitem_t[]> pollItems; //The poll object used to poll the SUB sockets
//Populate the poll object list
SOM_TRY
numberOfPollItems = 2;
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

pollItems[0] = {(void *) (*disconnectNotificationsSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*sourceSocket), 0, ZMQ_POLLIN, 0};

//Send OK message then start forwarding information from source
if(inputSocket.sendBytes(OKMessageString.c_str(), OKMessageString.size()) != OKMessageString.size())
{
return;
}
while(true)
{
SOM_TRY
messageBuffer->rebuild(); //Rebuild so the message buffer can be used again
SOM_CATCH("Error reinitializing message buffer\n")

SOM_TRY //Wait until for updates to send or a disconnect event 
if(zmq::poll(pollItems.get(), numberOfPollItems) == 0)
{
continue; //Poll returned without items to check, so loop again
}
SOM_CATCH("Error occurred listening for activity on ports\n")

if(pollItems[1].revents & ZMQ_POLLIN)
{//Received data from source, so forward over connection
SOM_TRY
sourceSocket->recv(messageBuffer.get());
SOM_CATCH("Error getting information from source publisher\n")

if(inputSocket.sendBytes(messageBuffer->data(), messageBuffer->size()) != messageBuffer->size())
{ //Couldn't forward data over connection, so close it
return;
}
}

if(pollItems[0].revents & ZMQ_POLLIN)
{//Source has disconnected, so close connection
return;
}

}
}

return;
}

//Invalid request
inputSocket.sendBytes(illFormedHeaderRejectionString.c_str(), illFormedHeaderRejectionString.size());
}
