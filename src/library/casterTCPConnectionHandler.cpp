#include "casterTCPConnectionHandler.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

/**
This constructor just passes the given socket to the base class
@param inputConnectionSocket: The socket associated with the connection this object is processing

@throws: This function can throw exceptions
*/
casterTCPConnectionHandler::casterTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket) : Poco::Net::TCPServerConnection(inputConnectionSocket)
{

}

/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.

@throws: This function can throw exceptions
*/
void casterTCPConnectionHandler::run()
{
Poco::Net::StreamSocket &connection = socket();

printf("I live!\n");

connection.setReceiveTimeout(Poco::Timespan(HEADER_RECEIVE_TIMEOUT, 0)); //TODO: Use C++11 clocks to actually have a total time of HEADER_RECEIVE_TIMEOUT before the connection is dropped
int receivedDataSize = 0;
int receivedDataPosition = 0; //Offset to start writing the data in the buffer at
char receivedDataBuffer[HEADER_BUFFER_SIZE];
std::stringstream headerStream;

//Receive until either the maximum character buffer size is reached or the http header has been read
while(true) //TODO: This operation could be made more efficient
{
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
return; //Header connection timed out (TODO: C++11 Clocks)
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

printf("Got this far\n");

//Process header data
headerStream.write(receivedDataBuffer, receivedDataPosition);
Poco::Net::HTTPRequest requestHeader;
try
{
requestHeader.read(headerStream);
}
catch(const std::exception &inputException)
{
return; //Invalid header, so skip
}

//Got a valid header, so send back the required (nonstandard) response
connection.sendBytes(serverResponseString.c_str(), serverResponseString.size());

printf("Body so far (%ld bytes):\n", headerStream.str().size());
Poco::StreamCopier::copyStream(headerStream, std::cout);

} 
