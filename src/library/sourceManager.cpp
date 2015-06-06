#include "sourceManager.hpp"

/**
This function initializes the object and starts a thread that conducts the operations of the router.  It does not return until all initialization in the thread is completed and all of the public members are safe to read.

@param inputZMQContext: The ZMQ context to use for inproc communications
@param inputPortNumber: The port number to bind for TCP based ZMQ requests to register source metadata with a dynamic port bound if the port number is negative (the resulting port number stored as a public variable).

@throws: This function can throw exceptions
*/
sourceManager::sourceManager(zmq::context_t *inputZMQContext, int inputPortNumber)
{
if(inputZMQContext == NULL)
{
throw SOMException(std::string("NULL value in received pointer to ZMQ context\n"), INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}
context = inputZMQContext;


SOM_TRY
shutdownSocket.reset(new zmq::socket_t(*(context), ZMQ_PULL));
SOM_CATCH("Error initializing shutdownSocket inproc socket\n")

int extensionStringNumber = 0; 

SOM_TRY //Bind to an dynamically generated address
std::tie(shutdownSocketConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*shutdownSocket, "shutdownSocketAddress");
SOM_CATCH("Error binding shutdownSocket\n")

SOM_TRY
serverMetadataAdditionSocket.reset(new zmq::socket_t(*(context), ZMQ_REP));
SOM_CATCH("Error initializing serverMetadataAdditionSocket tcp socket\n")

char endpointString[256];
size_t endpointStringSize = 256;

SOM_TRY
if(inputPortNumber < 0)
{
serverMetadataAdditionSocket->bind("tcp://**:*");
serverMetadataAdditionSocket->getsockopt(ZMQ_LAST_ENDPOINT, (void *) endpointString, &endpointStringSize);
std::string stdEndpointString(endpointString);
auto portDelimiterPosition = stdEndpointString.rfind(":");
if(portDelimiterPosition == std::string::npos || (portDelimiterPosition + 1) >= stdEndpointString.size())
{
throw SOMException(std::string("Error binding socket ") + endpointString + std::string("\n"), ZMQ_ERROR, __FILE__, __LINE__);
}

serverMetadataAdditionSocketPortNumber = std::stoi(stdEndpointString.substr(portDelimiterPosition+1));
}
else
{
serverMetadataAdditionSocketPortNumber = inputPortNumber;
serverMetadataAdditionSocket->bind((std::string("tcp://**:")+std::to_string(serverMetadataAdditionSocketPortNumber)).c_str());
}
SOM_CATCH("Error binding serverMetadataAdditionSocket tcp socket\n")

//Initialize each of the sockets and determine the socket address strings


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
numberOfPollItems = 4;
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

pollItems[0] = {(void *) (*serverRegistrationDeregistrationSocket), 0, ZMQ_POLLIN, 0};
pollItems[1] = {(void *) (*sourceTableAccessSocket), 0, ZMQ_POLLIN, 0};
pollItems[2] = {(void *) (*serverMetadataAdditionSocket), 0, ZMQ_POLLIN, 0};
pollItems[3] = {(void *) (*shutdownSocket), 0, ZMQ_POLLIN, 0};

//Start the thread
SOM_TRY
operationsThread.reset(new std::thread(&sourceManager::operate, this)); 
SOM_CATCH("Error initializing thread\n")
}

/**
This function signals for the thread to shut down and then waits for it to do so.
*/
sourceManager::~sourceManager()
{
//Send message to try to shut down thread
try
{
zmq::socket_t shutdownAlertSocket(*(context), ZMQ_PUSH);
shutdownAlertSocket.connect(shutdownSocketConnectionString.c_str());
shutdownAlertSocket.send(nullptr, 0);
}
catch(const std::exception &inputException)
{
}

operationsThread->join();
}

/**
This function is run in a thread to perform the necessary operations to allow the 3 interfaces to process messages as they are suppose to and keep the source table up to date.  It is normally called in a thread created in the object constructor.
*/
void sourceManager::operate()
{ //TODO: Deal with metadata timeouts
while(true)
{
//Process any events that have timed out (time to occur has passed) and determine time to next event timeout
std::chrono::steady_clock::time_point nextEventTimeout;

SOM_TRY
nextEventTimeout = handleEvents();
SOM_CATCH("Error handling events\n");

std::chrono::milliseconds pollWaitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(nextEventTimeout - std::chrono::steady_clock::now());
long int pollWaitTime = pollWaitDuration.count();
if(pollWaitTime < 0)
{
pollWaitTime = 0; //Make sure that we don't have negative wait
}

SOM_TRY //Wait until either a event is scheduled to occur or a message is received 
if(zmq::poll(pollItems.get(), numberOfPollItems, pollWaitTime) == 0)
{
continue; //Poll returned without items to check, so loop again (checking flag)
}
SOM_CATCH("Error occurred listening for activity on ports\n")

//See if it is time to shutdown
if(pollItems[3].revents & ZMQ_POLLIN)
{
return; //Don't bother reading the message, just shut down
}

//Process possible ntrip_server_metadata_addition_request
if(pollItems[2].revents & ZMQ_POLLIN)
{
SOM_TRY
handlePossibleNtripServerMetadataAdditionRequest();
SOM_CATCH("Error handling server metadata addition\n")
}

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

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
*/
void sourceManager::handlePossibleNtripServerMetadataAdditionRequest()
{
std::unique_ptr<zmq::message_t> messageBuffer;

SOM_TRY
messageBuffer.reset(new zmq::message_t);
SOM_CATCH("Error initializing ZMQ message")

SOM_TRY //Receive message
if(serverMetadataAdditionSocket->recv(messageBuffer.get(), ZMQ_DONTWAIT) != true)
{
return; //No message to be had
}
SOM_CATCH("Error receiving server registration/deregistration message")

//Create lambda to make it easy to send request failed replies
auto sendRequestFailedMessageLambda = [&] (enum reasonForRequestFailure inputReason)
{
ntrip_server_metadata_addition_reply reply;
std::string serializedReply;

reply.set_request_failure_reason(inputReason);
reply.SerializeToString(&serializedReply);

SOM_TRY
serverMetadataAdditionSocket->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Create lambda to make it easy to send request succeeded replies
auto sendRequestSucceededMessageLambda = [&] (const std::string &inputPassword, const std::string &inputMountpoint)
{
ntrip_server_metadata_addition_reply reply;
std::string serializedReply;

reply.set_password(inputPassword);
reply.set_mountpoint(inputMountpoint);
reply.SerializeToString(&serializedReply);

SOM_TRY
serverMetadataAdditionSocket->send(serializedReply.c_str(), serializedReply.size());
SOM_CATCH("Error sending reply message\n")
};

//Deserialize message
ntrip_server_metadata_addition_request request;
request.ParseFromArray(messageBuffer->data(), messageBuffer->size());

if(!request.IsInitialized())
{
//Message header serialization failed, so send back message saying request failed
SOM_TRY
sendRequestFailedMessageLambda(MESSAGE_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}

if(mountpointToStreamSourceMetadata.count(request.mountpoint()) > 0)
{
SOM_TRY //Already have metadata info for that mountpoint
sendRequestFailedMessageLambda(MOUNTPOINT_ALREADY_TAKEN);
return;
SOM_CATCH("Error sending reply");
}

streamSourceTableEntry entry;
if(entry.parse(request.server_source_table_entry()) < 0)
{
SOM_TRY //Already have metadata info for that mountpoint
sendRequestFailedMessageLambda(SOURCE_TABLE_ENTRY_DESERIALIZATION_FAILED);
return;
SOM_CATCH("Error sending reply");
}

if(request.mountpoint() == "")
{
//Dynamically generated mountpoint requested
while(true)
{
entry.mountpoint = generateRandomString(RANDOM_MOUNTPOINT_SIZE);

if(mountpointToStreamSourceMetadata.count(entry.mountpoint) == 0)
{
break; //Found mountpoint that works
}
}

}
std::string generatedPassword = generateRandomString(RANDOM_PASSWORD_SIZE); //Generate new password for stream


//Register new mountpoint metadata
auto metaDataRegistrationTime = std::chrono::steady_clock::now();
mountpointToStreamSourceMetadata[entry.mountpoint] = make_tuple(entry, (sourceConnectionStatus) PENDING, metaDataRegistrationTime);
mountpointToStreamSourcePassword[entry.mountpoint] = generatedPassword;

//Schedule timeout event 
SOM_TRY
scheduleMetadataTimeout(entry.mountpoint);
SOM_CATCH("Error scheduling timeout\n")

//Send back reply indicating success and the password
SOM_TRY
sendRequestSucceededMessageLambda(generatedPassword, entry.mountpoint);
SOM_CATCH("Error sending request succeeded message\n")
}

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
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
if(mountpointToStreamSourceTableEntry.count(request.mountpoint()) == 1 || !request.has_connection_address() || !request.has_password()) //Mountpoint taken or request had issues
{
SOM_TRY
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

//Check if we have the metadata for this mountpoint and the password matches
if(mountpointToStreamSourceMetadata.count(request.mountpoint()) != 1)
{
SOM_TRY //Don't have metadata for the source, so reject it
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

if(mountpointToStreamSourcePassword[request.mountpoint()] != request.password())
{
SOM_TRY //Password doesn't match, so reject it
sendReplyLambda(false);
return;
SOM_CATCH("Error sending reply");
}

//Add server to source table
mountpointToStreamSourceTableEntry[request.mountpoint()] = std::get<0>(mountpointToStreamSourceMetadata[request.mountpoint()]);
mountpointToZMQConnectionString[request.mountpoint()] = request.connection_address();

//Update metadata entry with update time and connection status
if(std::get<1>(mountpointToStreamSourceMetadata[request.mountpoint()]) != PERMANENT)
{ //Can't change if it is permanent
std::get<1>(mountpointToStreamSourceMetadata[request.mountpoint()]) = CONNECTED;
}
std::get<2>(mountpointToStreamSourceMetadata[request.mountpoint()]) = std::chrono::steady_clock::now();

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

//Mark disconnected if it isn't permanent
if(std::get<1>(mountpointToStreamSourceMetadata[request.mountpoint()]) != PERMANENT)
{
std::get<1>(mountpointToStreamSourceMetadata[request.mountpoint()]) = DISCONNECTED;
}
std::get<2>(mountpointToStreamSourceMetadata[request.mountpoint()]) = std::chrono::steady_clock::now();

//Erase entry
mountpointToStreamSourceTableEntry.erase(request.mountpoint());
mountpointToZMQConnectionString.erase(request.mountpoint());

//Schedule deletion of the metadata if the source doesn't reconnect before the timeout
SOM_TRY
scheduleMetadataTimeout(request.mountpoint());
SOM_CATCH("Error scheduling timeout\n")

//Emit signal that entry has been removed
SOM_TRY
mountpointDisconnectSocket->send(request.mountpoint().c_str(), request.mountpoint().size());
SOM_CATCH("Error sending mountpoint removed message\n")

SOM_TRY //Entry erased, so request succeeded
sendReplyLambda(true);
return;
SOM_CATCH("Error sending reply");
}

/**
This function (typically called by operate), checks to see if there is a request and takes the appropriate action (such as generating and sending a response), if there is.

@throws: This function can throw exceptions
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

/**
This function processes any events that are scheduled to have occurred by now and returns when the next event is scheduled to occur.
@return: The time point associated with the soonest event timeout (1 minute from now if no events are waiting)

@throws: This function can throw exceptions
*/
std::chrono::steady_clock::time_point sourceManager::handleEvents()
{
//Define lambdas to handle different event types

//possible_metadata_timeout_event
auto handlePossibleMetadataTimeout = [&] ()
{ //Delete metadata if the timeout time has not been extended or the status is not connected or permanent
//Get event data
std::string mountpoint = eventQueue.top().GetExtension(possible_metadata_timeout_event::instance).mountpoint();
//Remove the event from the queue
eventQueue.pop();


if(mountpointToStreamSourceMetadata.count(mountpoint) == 0 || mountpointToStreamSourcePassword.count(mountpoint) == 0)
{//Seems to be erased already, so do nothing
return;
}

if( std::get<1>(mountpointToStreamSourceMetadata[mountpoint]) == CONNECTED || std::get<1>(mountpointToStreamSourceMetadata[mountpoint]) == PERMANENT)
{ //It's either connected or marked so it shouldn't be deleted, so don't do anything
return;
}

if(std::get<2>(mountpointToStreamSourceMetadata[mountpoint]) > std::chrono::steady_clock::now() )
{ //Its time has been extended (presumably with another timeout scheduled), so don't do anything
return;
}

//Erase the metadata
mountpointToStreamSourceMetadata.erase(mountpoint);
mountpointToStreamSourcePassword.erase(mountpoint);
};

if(eventQueue.size() == 0)
{
return std::chrono::steady_clock::now() + std::chrono::minutes(1); //No events to process
}

//Process events with time points less than the current time
while(eventQueue.top().time < std::chrono::steady_clock::now())
{
if(eventQueue.top().HasExtension(possible_metadata_timeout_event::instance))
{
handlePossibleMetadataTimeout();
}

if(eventQueue.size() == 0)
{
std::chrono::steady_clock::now() + std::chrono::minutes(1); //No events to process
}
}

return eventQueue.top().time;
}

/**
This function schedules the potential timeout of the associated metadata entry.  If the entry is not in a connected state when the timeout is processed, the metadata will be dropped.

@throws: This function can throw exceptions
*/
void sourceManager::scheduleMetadataTimeout(const std::string &inputMountpoint)
{
if(mountpointToStreamSourceMetadata.count(inputMountpoint) == 0 || 
mountpointToStreamSourcePassword.count(inputMountpoint) == 0)
{
throw SOMException("Metadata associated with mountpoint could not be found\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(std::get<1>(mountpointToStreamSourceMetadata[inputMountpoint]) != PENDING && std::get<1>(mountpointToStreamSourceMetadata[inputMountpoint]) == DISCONNECTED )
{
return; //Not going to timeout, so don't add an event
}

std::chrono::milliseconds delay;

if(std::get<1>(mountpointToStreamSourceMetadata[inputMountpoint]) == PENDING )
{
delay = std::chrono::milliseconds(MILLISECONDS_TO_WAIT_BEFORE_DROPPING_METADATA_PENDING); //Pending wait time
}
else
{
delay = std::chrono::milliseconds(MILLISECONDS_TO_WAIT_BEFORE_DROPPING_METADATA_DISCONNECTED); //Disconnected wait time
}

//Create event and add it to the queue
event timeoutEvent(std::get<2>(mountpointToStreamSourceMetadata[inputMountpoint])+delay);
timeoutEvent.MutableExtension(possible_metadata_timeout_event::instance)->set_mountpoint(inputMountpoint);
eventQueue.push(timeoutEvent);
}

/**
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


/**
This function compactly allows binding a ZMQ socket to inproc address without needing to specify an exact address.  The function will try binding to addresses in the format: inproc://inputBaseString.inputExtensionNumberAsString and will try repeatedly while incrementing inputExtensionNumber until it succeeds or the maximum number of tries has been exceeded.
@param inputSocket: The ZMQ socket to bind
@param inputBaseString: The base string to use
@param inputExtensionNumber: The extension number to start with
@param inputMaximumNumberOfTries: How many times to try binding before giving up
@return: A tuple of form <connectionString ("inproc://etc"), extensionNumberThatWorked>

@throws: This function can throw exceptions if the bind call throws something besides "address taken" or the number of tries are exceeded
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

