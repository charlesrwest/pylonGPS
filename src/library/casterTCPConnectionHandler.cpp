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

@throws: This function can throw exceptions
*/
void casterTCPConnectionHandler::run()
{
Poco::Net::StreamSocket &connection = socket();

std::chrono::steady_clock::time_point headerTimeoutTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(HEADER_RECEIVE_TIMEOUT);

//Retrieve header
int receivedDataSize = 0;
int receivedDataPosition = 0; //Offset to start writing the data in the buffer at
char receivedDataBuffer[HEADER_BUFFER_SIZE];


//Receive until either the maximum character buffer size is reached or the http header has been read
while(true) //TODO: This operation could be made more efficient
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
printf("Received %d bytes data\n",  receivedDataPosition);
if(receivedDataSize == 0)
{
return; //Connection closed from client side
}
}
catch(const Poco::TimeoutException &inputException)
{
printf("Connection timed out\n");
return; //Header connection timed out
}
SOM_CATCH("Error receiving header data from TCP connection")

if(receivedDataPosition >= HEADER_BUFFER_SIZE)
{
break; //Reached maximum header size, so try processing it
}

//Check if <CR><LF><CR><LF> ("\r\n\r\n") or <LF><LF> "\n\n" has been received
for(int i=0; i<(receivedDataPosition-2); i++)
{
if(receivedDataBuffer[i] == '\n' && receivedDataBuffer[i+1] == '\n')
{
break; //Header delimiter found
}
}

for(int i=0; i<(receivedDataPosition-4); i++)
{
if(receivedDataBuffer[i] == '\r' && receivedDataBuffer[i+1] == '\n' && receivedDataBuffer[i+2] == '\r' && receivedDataBuffer[i+3] == '\n')
{
break; //Header delimiter found
}
}

std::string receivedData(receivedDataBuffer, receivedDataPosition);

printf("Delimiters not found\n");
printf("Received: %s\n", receivedData.c_str());
}

//SOURCE requests are not compliant with the HTTP standard, so special processing has to be used for them
std::string headerString(receivedDataBuffer, receivedDataPosition);

//Check if it is a SOURCE request or a normal HTTP request
if(headerString.find("SOURCE") == 0) //It's a SOURCE request, so offload it to a dedicated handler
{
//TODO: Finish SOURCE request handler
}

//Process header data
std::stringstream headerStream;
headerStream.write(receivedDataBuffer, receivedDataPosition);
Poco::Net::HTTPRequest requestHeader;
try
{
requestHeader.read(headerStream);
}
catch(const std::exception &inputException)
{
printf("Invalid header\n");
return; //Invalid header, so skip
}

//Call the right method to deal with the request
if(requestHeader.getMethod().find("GET") != std::string::npos)
{
printf("Received GET request\n");
}
else
{//Invalid request
printf("I got a bad request: \nMethod: %s\n", requestHeader.getMethod().c_str());
}

//Got a valid header, so send back the required (nonstandard) response
connection.sendBytes(serverResponseString.c_str(), serverResponseString.size());

printf("Body so far (%ld bytes):\n", headerStream.str().size());
Poco::StreamCopier::copyStream(headerStream, std::cout);

} 

/**
This function is called to handle serving a SOURCE request.
@param inputHeaderString: The string containing the SOURCE request and possibly some body data
@param inputSocket: The socket to use to get/send data over the associated connection

@throws: This function can throw exceptions
*/
void handleSourceRequest(const std::string &inputHeaderString, Poco::Net::StreamSocket &inputSocket)
{
//Check header. If invalid, send rejection message
//Tokenize
auto secondLineEndingPosition = inputHeaderString.find("\r\n\r\n");
if(secondLineEndingPosition == std::string::npos)
{
return;
}

//Get the header without the body or end \r\n\r\n
std::string headerStringTruncated = inputHeaderString.substr(0, secondLineEndingPosition);

auto firstLineEndingPosition = headerStringTruncated.find("\r\n");
if(firstLineEndingPosition == std::string::npos)
{
return; //TODO: Should probably make another serialization/deserialization class for this request format
}
//std::string firstLine = 

}
