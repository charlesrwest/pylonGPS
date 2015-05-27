#include "sourceManager.hpp"

/*
This function initializes the object and starts a thread that conducts the operations of the router.  It does not return until all initialization in the thread is completed and all of the public members are safe to read.

@param inputZMQContext: The ZMQ context to use for inproc communications

@exceptions: This function can throw exceptions
*/
sourceManager::sourceManager(zmq::context_t *inputZMQContext)
{
if(inputZMQContext == NULL)
{
throw SOMException(std::string("NULL value in received pointer to ZMQ context\n"), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
context = inputZMQContext;

timeToShutdownFlag = false;

//Initialize each of the sockets and determine the socket address strings
int extensionStringNumber = 0; 

SOM_TRY
serverRegistrationDeregistrationSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error initializing serverRegistrationDeregistrationSocket inproc socket\n")




SOM_TRY //Bind to an dynamically generated address
std::tie(serverRegistrationDeregistrationSocketConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*serverRegistrationDeregistrationSocket, "serverRegistrationDeregistrationSocketAddress");
SOM_CATCH("Error binding serverRegistrationDeregistrationSocket\n")

SOM_TRY
mountpointDisconnectSocket.reset(new zmq::socket_t(*(context), ZMQ_PUB));
SOM_CATCH("Error initializing mountpointDisconnectSocket inproc socket\n")
 
SOM_TRY //Bind to an dynamically generated address
std::tie(mountpointDisconnectSocketConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*mountpointDisconnectSocket, "mountpointDisconnectSocketAddress");
SOM_CATCH("Error binding mountpointDisconnectSocket\n")

SOM_TRY
sourceTableAccessSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error initializing mountpointDisconnectSocket inproc socket\n")
 
SOM_TRY //Bind to an dynamically generated address
std::tie(sourceTableAccessSocketConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*sourceTableAccessSocket, "sourceTableAccessSocketAddress");
SOM_CATCH("Error binding sourceTableAccessSocket\n")

//Populate the poll object list
SOM_TRY
numberOfPollItems = 2;
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

pollItems[0] = {(void *) (*serverRegistrationDeregistrationSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*sourceTableAccessSocket), 0, ZMQ_POLLIN, 0};

//Start the thread
SOM_TRY
operationsThread.reset(new std::thread(&sourceManager::operate, this)); 
SOM_CATCH("Error initializing thread\n")
}

/*
This function signals for the thread to shut down and then waits for it to do so.
*/
sourceManager::~sourceManager()
{
timeToShutdownFlag = true;
operationsThread->join();
}

/*
This function is run in a thread to perform the necessary operations to allow the 3 interfaces to process messages as they are suppose to and keep the source table up to date.  It is normally called in a thread created in the object constructor.
*/
void sourceManager::operate()
{
while(!timeToShutdownFlag)
{
//See if any requests have been received, polling with a 100 millisecond timeout so that we can periodically check if the thread should shut down (consider adding another socket for shutdown so flag checking wouldn't be required)
SOM_TRY
if(zmq::poll(pollItems.get(), numberOfPollItems, 100) == 0)
{
continue; //Poll returned without items to check, so loop again (checking flag)
}
SOM_CATCH("Error occurred listening for activity on ports\n")

//Process possible ntrip_server_registration_or_deregistraton_request
if(pollItems[0].revents & ZMQ_POLLIN)
{
SOM_TRY
handlePossibleNtripServerRegistrationOrDeregistratonRequest();
SOM_CATCH("Error handling server registration or deregistration\n");
}

//Process possible ntrip_source_table_request
if(pollItems[1].revents & ZMQ_POLLIN)
{
SOM_TRY
handlePossibleNtripSourceTableRequest();
SOM_CATCH("Error handing source table request\n")
}
}
}

/*
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@exceptions: This function can throw exceptions
*/
void sourceManager::handlePossibleNtripServerRegistrationOrDeregistratonRequest()
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(serverRegistrationDeregistrationSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving server registration/deregistration message")

//Create lambda to make it easy to send request succeeded or request failed replies
auto sendReplyLambda = [&] (bool inputReplyValue)
{
ntrip_server_registration_or_deregistraton_reply reply;
std::string serializedReply;

reply.set_request_succeeded(inputReplyValue);
reply.SerializeToString(&serializedReply);

SOM_TRY
serverRegistrationDeregistrationSocket->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Deserialize message
ntrip_server_registration_or_deregistraton_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

if(request.registering()) //Deal with registration
{
if(mountpointToStreamSourceTableEntry.count(request.mountpoint()) == 1 || !request.has_connection_address() || !request.has_server_source_table_string()) //Mountpoint taken or request had issues
{
SOM_TRY
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

//Attempt to read source table entry from string
streamSourceTableEntry tableEntry;
if(tableEntry.parse(request.server_source_table_string()) < 0)
{
printf("Couldn't read table entry\n");
SOM_TRY //Couldn't read table entry
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

//Add server to source table
mountpointToStreamSourceTableEntry[request.mountpoint()] = tableEntry;
mountpointToZMQConnectionString[request.mountpoint()] = request.connection_address();

SOM_TRY //Source table update, request succeeded
sendReplyLambda(true);
return;
SOM_CATCH("Error sending reply");
}

//Attempting to deregister
if(mountpointToStreamSourceTableEntry.count(request.mountpoint()) != 1)
{
SOM_TRY //Can't remove what isn't there
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

//Erase entry
mountpointToStreamSourceTableEntry.erase(request.mountpoint());
mountpointToZMQConnectionString.erase(request.mountpoint());

//Emit signal that entry has been removed
SOM_TRY
mountpointDisconnectSocket->send(request.mountpoint().c_str(), request.mountpoint().size());
SOM_CATCH("Error sending mountpoint removed message\n")

SOM_TRY //Entry erased, so request succeeded
sendReplyLambda(true);
return;
SOM_CATCH("Error sending reply");
}

/*
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@exceptions: This function can throw exceptions
*/
void sourceManager::handlePossibleNtripSourceTableRequest()
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(sourceTableAccessSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving source table request message\n")

//Create lambda to make it easy to send request succeeded or request failed replies
auto sendReplyLambda = [&] (const std::string &inputMountpoint)
{ //Sends back source table if mountpoint is empty or not found
std::string serializedReply;
ntrip_source_table_reply reply;

if(inputMountpoint.size() == 0 || mountpointToStreamSourceTableEntry.count(inputMountpoint) == 0)
{
reply.set_source_table(generateSerializedSourceTable());
}
else
{
reply.set_stream_inproc_address(mountpointToZMQConnectionString[inputMountpoint]);
}

reply.SerializeToString(&serializedReply);

SOM_TRY
sourceTableAccessSocket->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Deserialize message
ntrip_source_table_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
//Send full source table since invalid request received
SOM_TRY
sendReplyLambda(""); 
return;
SOM_CATCH("Error sending reply");
}

//Send reply (source table if mountpoint not found, inproc address if it is)
SOM_TRY
sendReplyLambda(request.mountpoint()); 
return;
SOM_CATCH("Error sending reply");
}

/*
This function generates a string that is the ascii serialization (NTRIP format) of the source table.
@return: The ascii serialized source table
*/
std::string sourceManager::generateSerializedSourceTable()
{
std::string stringToReturn;
for(int i=0; i<casterEntries.size(); i++)
{ //Get all the casters
stringToReturn += casterEntries[i].serialize();
}

for(int i=0; i<networkEntries.size(); i++)
{ //Get all the casters
stringToReturn += networkEntries[i].serialize();
}

for(auto iter = mountpointToStreamSourceTableEntry.begin(); iter != mountpointToStreamSourceTableEntry.end(); iter++)
{
stringToReturn += iter->second.serialize();
}

stringToReturn += "ENDSOURCETABLE\r\n";

return stringToReturn;
}


/*
This function compactly allows binding a ZMQ socket to inproc address without needing to specify an exact address.  The function will try binding to addresses in the format: inproc://inputBaseString.inputExtensionNumberAsString and will try repeatedly while incrementing inputExtensionNumber until it succeeds or the maximum number of tries has been exceeded.
@param inputSocket: The ZMQ socket to bind
@param inputBaseString: The base string to use
@param inputExtensionNumber: The extension number to start with
@param inputMaximumNumberOfTries: How many times to try binding before giving up
@return: A tuple of form <connectionString ("inproc://etc"), extensionNumberThatWorked>

@exceptions: This function can throw exceptions if the bind call throws something besides "address taken" or the number of tries are exceeded
*/
std::tuple<std::string, int> bindZMQSocketWithAutomaticAddressGeneration(zmq::socket_t &inputSocket, const std::string &inputBaseString, int inputExtensionNumber, unsigned int inputMaximumNumberOfTries)
{
bool socketBindSuccessful = false;
std::string connectionString;
int extensionNumber = inputExtensionNumber;

for(int i=0; i<inputMaximumNumberOfTries; i++)
{
try //Attempt to bind the socket
{
connectionString = std::string("inproc://") + inputBaseString + std::string(".") + std::to_string(extensionNumber);
inputSocket.bind(connectionString.c_str());
socketBindSuccessful = true; //Got this far without throwing
break;
}
catch(const zmq::error_t &inputZMQError)
{
if(inputZMQError.num() == EADDRINUSE)
{
extensionNumber++; //Increment so the next attempted address won't conflict
}
else
{
throw SOMException(std::string("Error binding socket") + connectionString + std::string("\n"), ZMQ_ERROR, __FILE__, __LINE__);
}
}

}

if(!socketBindSuccessful)
{
throw SOMException(std::string("Socket bind did not succeed in ") + std::to_string(inputMaximumNumberOfTries) + std::string(" attempts\n"), UNKNOWN, __FILE__, __LINE__);
}

return make_tuple(connectionString, extensionNumber);
}

