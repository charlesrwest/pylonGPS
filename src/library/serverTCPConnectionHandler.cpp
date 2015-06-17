 #include "serverTCPConnectionHandler.hpp"

using namespace pylongps;

/**
This constructor passes the given socket to the base class and stores the information to register the associated metadata and connect to the caster.
@param inputConnectionSocket: The socket associated with the connection this object is processing
@param inputCasterURI: The URI (host:port) associated with the caster used to publish the data.
@param inputStreamSourceMetadata: The metadata associated with this source to put in the source table

@throws: This function can throw exceptions
*/
serverTCPConnectionHandler::serverTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, const Poco::URI &inputCasterURI, const streamSourceTableEntry &inputStreamSourceMetadata) : Poco::Net::TCPServerConnection(inputConnectionSocket), casterURI(inputCasterURI), metadata(inputStreamSourceMetadata)
{
}


/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.
*/
void serverTCPConnectionHandler::run()
{
try
{
//URI and metadata have been checked but not resolved
Poco::Net::StreamSocket &sourceConnection = socket();

//Resolve IP address associated with URI
Poco::Net::IPAddress casterAddress;
try
{
casterAddress = Poco::Net::DNS::resolveOne(casterURI.getHost());
}
catch(const std::exception &inputException)
{
std::cerr << "Error, unable to resolve caster IP address" << std::endl;
return;
}

std::unique_ptr<zmq::context_t> context;
SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error creating ZMQ context\n")

//Attempt to register source metadata
//Make socket
std::unique_ptr<zmq::socket_t> metaDataRequestSocket;
SOM_TRY
metaDataRequestSocket.reset(new zmq::socket_t(*context, ZMQ_REQ));
SOM_CATCH("Error initializing metadata request socket\n")

//Set receive timeout 
SOM_TRY
int timeoutDuration = METADATA_REGISTRATION_TIMEOUT_DURATION;
metaDataRequestSocket->setsockopt(ZMQ_RCVTIMEO, &timeoutDuration, sizeof(timeoutDuration));
SOM_CATCH("Error setting ZMQ socket option\n")

//Connect to caster tcp port
SOM_TRY
std::string connectionString = "tcp://"+casterAddress.toString()+":" + std::to_string(casterURI.getPort()+1);
metaDataRequestSocket->connect(connectionString.c_str());
SOM_CATCH("Error connecting to socket\n")

//Create request message
std::string serializedMetadataRegistrationRequest;
pylongps::ntrip_server_metadata_addition_request metadataRegistrationRequest;

metadataRegistrationRequest.set_mountpoint(metadata.mountpoint);
metadataRegistrationRequest.set_server_source_table_entry(metadata.serialize());

//Serialize request
metadataRegistrationRequest.SerializeToString(&serializedMetadataRegistrationRequest);

//Send request to register metadata for source (and get generated password)
SOM_TRY
metaDataRequestSocket->send(serializedMetadataRegistrationRequest.c_str(), serializedMetadataRegistrationRequest.size());
SOM_CATCH("Error sending metadata registration request message\n")



std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")


SOM_TRY
if(metaDataRequestSocket->recv(messageBuffer.get()) != true)
{
std::cerr << "Metadata request attempt timed out" << std::endl;
return;
}
SOM_CATCH("Error recieving metadata registration message from caster\n")



pylongps::ntrip_server_metadata_addition_reply metadataReply;

metadataReply.ParseFromArray(messageBuffer->data(), messageBuffer->size());
if(metadataReply.IsInitialized() != true)
{
std::cerr << "Metadata registration reply from server is invalid" << std::endl;
return;
}

//Check if metadata request succeeded
if(metadataReply.has_request_failure_reason())
{
std::string failureReasonString;
if(metadataReply.request_failure_reason() == MESSAGE_DESERIALIZATION_FAILED)
{
failureReasonString = "Caster could not deserialize our registration message"; 
}
else if(metadataReply.request_failure_reason() == SOURCE_TABLE_ENTRY_DESERIALIZATION_FAILED)
{
failureReasonString = "Caster found our source table entry invalid"; 
}
else if(metadataReply.request_failure_reason() == MOUNTPOINT_ALREADY_TAKEN)
{
failureReasonString = "Mountpoint has already been registered"; 
}

std::cerr << "Caster rejected metadata registration request: " << failureReasonString << std::endl;
return;
}

if(metadataReply.has_password() != true)
{
std::cerr << "Caster metadata registration reply did not have the password to use" << std::endl;
return;
}

if(metadataReply.has_mountpoint() != true)
{
std::cerr << "Caster metadata registration reply invalid (no mountpoint entry)" << std::endl;
return;
}

if(metadataReply.mountpoint() != metadata.mountpoint)
{
std::cerr << "Caster metadata registration reply invalid (returned different mountpoint)" << std::endl;
return;
}

//Metadata has been registered, so forward information from the connection until one or the other connections have been broken
std::unique_ptr<Poco::Net::StreamSocket> casterConnection;

SOM_TRY
casterConnection.reset(new Poco::Net::StreamSocket(Poco::Net::SocketAddress(casterAddress, casterURI.getPort())));
SOM_CATCH("Error, unable to create/connect socket\n")

//Register with the provided password
std::string sourceHeaderMessage;
sourceHeaderMessage += "SOURCE " + metadataReply.password() + " " + metadataReply.mountpoint() + "\r\n";
sourceHeaderMessage += "Source-Agent: NTRIP product|comment\r\n\r\n";

int bytesReceived = 0;
int bytesSent = 0;
int bytesRemainingSize = sourceHeaderMessage.size();
int headerBytesSent = 0;

//Send registration header
while(bytesRemainingSize > 0)
{
try
{//Offset the data pointer by the number of bytes that have been sent
bytesSent = casterConnection->sendBytes((void *) (sourceHeaderMessage.c_str()+sourceHeaderMessage.size() - bytesRemainingSize), bytesRemainingSize);
}
catch(const std::exception &inputException)
{ //Socket closed or other error occurred
return;
}

if(bytesSent < 0)
{//Error occurred
return;
}
bytesRemainingSize = bytesRemainingSize - bytesSent;
}


//Set maximum time to wait for reply
casterConnection->setReceiveTimeout(Poco::Timespan(MAX_NTRIP_REGISTRATION_WAIT/5,0)); //Max wait

bytesReceived = 0;
bytesSent = 0;
bytesRemainingSize = 0;
char forwardingBuffer[DATA_FORWARDING_BUFFER_SIZE];
std::string receivedReply;

for(int i=0; i<5; i++)//Up to 5 message received
{
SOM_TRY
try
{
bytesReceived = casterConnection->receiveBytes(forwardingBuffer, DATA_FORWARDING_BUFFER_SIZE);
}
catch(const Poco::TimeoutException &inputException)
{//Don't do anything
}
SOM_CATCH("Error getting reply from server\n")

receivedReply+= std::string(forwardingBuffer, bytesReceived);
if(receivedReply.find("\r\n\r\n") != std::string::npos)
{
break;
}
}



if(receivedReply.find("\r\n\r\n") == std::string::npos || receivedReply.find("ICY 200 OK\r\n\r\n") != 0)
{
std::cerr << "Caster ntrip registration reply invalid" << std::endl;
return;
} 

bytesReceived = 0;
bytesSent = 0;
bytesRemainingSize = 0;
while(true)
{//Forward data until one of the connections close
try
{//Get data from source
bytesReceived = sourceConnection.receiveBytes((void *) forwardingBuffer, DATA_FORWARDING_BUFFER_SIZE);
}
catch(const std::exception &inputException)
{
return;
}
if(bytesReceived == 0)
{
return;
}
bytesRemainingSize = bytesReceived;

while(bytesRemainingSize > 0)
{
try
{//Offset the data pointer by the number of bytes that have been sent
bytesSent = casterConnection->sendBytes((void *) (forwardingBuffer+bytesReceived - bytesRemainingSize), bytesRemainingSize);
}
catch(const std::exception &inputException)
{ //Socket closed or other error occurred
return;
}

if(bytesSent < 0)
{//Error occurred
return;
}
bytesRemainingSize = bytesRemainingSize - bytesSent;
}

}

}
catch(const std::exception &inputException)
{
std::cerr << inputException.what() << std::endl;
return;
}
}
