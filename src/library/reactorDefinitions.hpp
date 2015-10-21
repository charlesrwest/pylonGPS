

/**
This function initializes the reactor with the function that should be called to handle events.
@param inputContext: The ZMQ context that this object should use
@param inputClassInstance: The instance of the class that the given functions should operate on
@param inputEventHandler: The function to call to handle events in the queue (should return negative if there are no outstanding events).  Can be set to nullptr to disable event handling.

@throws: This function can throw exceptions
*/
template <class classType> reactor<classType>::reactor(zmq::context_t *inputContext, classType *inputClassInstance, std::function<Poco::Timestamp (classType*, reactor<classType> &)> inputEventHandler)
{
if(inputContext == nullptr || inputClassInstance == nullptr)
{
throw SOMException("Null pointer given for required field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

context = inputContext;


classInstance = inputClassInstance;

if(inputEventHandler != nullptr)
{
eventHandlerFunction = inputEventHandler;
}
else
{
eventHandlerFunction = [](classType* inputClassPointer, reactor<classType> &inputReactor) {return false;};
}

//Create and bind/connect shutdown sockets
SOM_TRY
shutdownSocket.reset(new zmq::socket_t(*(context), ZMQ_PAIR));
SOM_CATCH("Error intializing shutdownSocket\n")

int extensionStringNumber = 0;
std::string shutdownConnectionString;
SOM_TRY //Bind to an dynamically generated address
std::tie(shutdownConnectionString,extensionStringNumber) = bindZMQSocketWithAutomaticAddressGeneration(*shutdownSocket, "reactorShutdownSocketAddress");
SOM_CATCH("Error binding shutdownPublishingSocket\n")

SOM_TRY
shutdownReceivingSocket.reset(new zmq::socket_t(*(context), ZMQ_PAIR));
SOM_CATCH("Error creating shutdown receiving socket\n")

SOM_TRY
shutdownReceivingSocket->connect(shutdownConnectionString.c_str());
SOM_CATCH("Error, unable to connect receiving socket\n")
}

/**
This (not thread safe) function adds a new socket for the reactor to take ownership of and the member function to call/pass the socket reference to when a message is waiting on that interface.
@param inputSocket: The socket to take ownership of
@param inputMessageHandler: The function to call to handle messages waiting on the interface (returns true if the poll loop should restart rather than continuing and expects a pointer to this object)
@param inputInterfaceName: The (required unique) name to associated with the interface

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::addInterface(std::unique_ptr<zmq::socket_t> &inputSocket, std::function<bool (classType*, reactor<classType> &, zmq::socket_t &)> inputMessageHandler, const std::string &inputInterfaceName)
{
if(inputSocket.get() == nullptr || inputMessageHandler == nullptr)
{
throw SOMException("Null pointer given for required field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(nameToSocket.count(inputInterfaceName) > 0)
{
throw SOMException("Already used name\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Add to maps
auto addressBuffer = inputSocket.get();
interfaces.emplace(addressBuffer, std::move(inputSocket));
socketToHandlerFunction.emplace(addressBuffer, inputMessageHandler);
if(inputInterfaceName != "")
{
nameToSocket[inputInterfaceName] = addressBuffer;
}

SOM_TRY //Regenerate poll items so that this interface is included
regenerateZMQPollArray();
SOM_CATCH("Error, unable to regenerate poll items\n")

}

/**
This (not thread safe) function adds a file descriptor for the reactor to take ownership of and the member function to call/pass the file descriptor to when data is waiting on that interface.
@param inputFileDescriptor: The file descriptor to take ownership of
@param inputStreamHandler: The function to call to handle data waiting on the interface (returns true if the poll loop should restart rather than continuing and expects a pointer to this object)
@param inputInterfaceName: The (required unique) name to associated with the interface

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::addInterface(FILE *inputFileDescriptor, std::function<bool (classType*, reactor<classType> &, FILE *)> inputStreamHandler, const std::string &inputInterfaceName)
{
if(inputFileDescriptor == nullptr || inputStreamHandler == nullptr)
{
throw SOMException("Null pointer given for required field\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

if(nameToFileDescriptor.count(inputInterfaceName) > 0)
{
throw SOMException("Already used name\n", INVALID_FUNCTION_INPUT, __FILE__, __LINE__);
}

//Add to maps
fileInterfaces.emplace(fileno(inputFileDescriptor), inputFileDescriptor);
fileNumberToHandlerFunction.emplace(fileno(inputFileDescriptor), inputStreamHandler);
if(inputInterfaceName != "")
{
nameToFileDescriptor[inputInterfaceName] = inputFileDescriptor;
}

SOM_TRY //Regenerate poll items so that this interface is included
regenerateZMQPollArray();
SOM_CATCH("Error, unable to regenerate poll items\n")

}


/**
This (not threadsafe) function removes an interface from the reactor.
@param inputSocket: The socket interface to remove

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::removeInterface(zmq::socket_t *inputSocket)
{
//Remove from maps
interfaces.erase(inputSocket);
socketToHandlerFunction.erase(inputSocket);
for(auto iter = nameToSocket.begin(); iter != nameToSocket.end(); iter++)
{ //Search to find given socket
if(iter->second == inputSocket)
{
nameToSocket.erase(iter);
break;
}
}


SOM_TRY //Regenerate poll items so that this interface is no longer included
regenerateZMQPollArray();
SOM_CATCH("Error, unable to regenerate poll items\n")
}

/**
This (not threadsafe) function removes an interface from the reactor.
@param inputFileDescriptor: The file interface to remove

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::removeInterface(FILE *inputFileDescriptor)
{
//Remove from maps
fileInterfaces.erase(fileno(inputFileDescriptor));
fileNumberToHandlerFunction.erase(fileno(inputFileDescriptor));
for(auto iter = nameToFileDescriptor.begin(); iter != nameToFileDescriptor.end(); iter++)
{ //Search to find given socket
if(iter->second == inputFileDescriptor)
{
nameToFileDescriptor.erase(iter);
break;
}
}

fclose(inputFileDescriptor);


SOM_TRY //Regenerate poll items so that this interface is no longer included
regenerateZMQPollArray();
SOM_CATCH("Error, unable to regenerate poll items\n")
}

/**
This function starts the reactor so that it begins to process events and messages.
@param inputStartingEvents: The events that should be in the queue when message processing begins

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::start(const std::vector<event> &inputStartingEvents)
{
//Add events to process 
for(int i=0; i<inputStartingEvents.size(); i++)
{
eventQueue.push(inputStartingEvents[i]);
}

//start the thread

SOM_TRY
reactorThread.reset(new std::thread(&reactor<classType>::reactorThreadFunction, this)); 
SOM_CATCH("Error initializing thread\n")
}

/**
This function returns a pointer to the socket for the interface associated with the given name.  If the name is not found, an exception is thrown.
@param inputInterfaceName: The name of the interface with the socket

@throws: This function can throw exceptions
*/
template <class classType> zmq::socket_t * reactor<classType>::getSocket(const std::string &inputInterfaceName)
{
if(nameToSocket.count(inputInterfaceName) == 0)
{
throw SOMException("Expected socket not present in reactor\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

return nameToSocket.at(inputInterfaceName);
}

/**
This function returns a pointer to the file descriptor for the interface associated with the given name.  If the name is not found, an exception is thrown.
@param inputInterfaceName: The name of the interface with the file descriptor

@throws: This function can throw exceptions
*/
template <class classType>  FILE * reactor<classType>::getFileDescriptor(const std::string &inputInterfaceName)
{
if(nameToFileDescriptor.count(inputInterfaceName) == 0)
{
throw SOMException("Expected file descriptor not present in reactor\n", AN_ASSUMPTION_WAS_VIOLATED_ERROR, __FILE__, __LINE__);
}

return nameToFileDescriptor.at(inputInterfaceName);
}

/**
This function sends the termination signal to the reactor's thread and waits for it to shut down. 
*/
template <class classType> reactor<classType>::~reactor()
{
//Publish shutdown signal and wait for the thread
try
{ //Send empty message to signal shutdown
SOM_TRY
shutdownSocket->send(nullptr, 0);
SOM_CATCH("Error sending shutdown signal for reactor thread\n")
}
catch(const std::exception &inputException)
{
fprintf(stderr, "%s", inputException.what());
}

//Wait for threads to finish
reactorThread->join();

for(auto iter = fileInterfaces.begin(); iter != fileInterfaces.end(); iter++)
{
fclose(iter->second); //TODO: replace with custom delete unique pointers
}
}


/**
This function is called by the thread object to perform the message and event processing operations.
*/
template <class classType> void reactor<classType>::reactorThreadFunction()
{
try
{//Run event/message loop

//Determine if an event has timed out (and deal with it if so) and then calculate the time until the next event timeout
Poco::Timestamp nextEventTime;
int64_t timeUntilNextEventInMilliseconds = 0;
while(true)
{
nextEventTime = eventHandlerFunction(classInstance, *this);

if(nextEventTime < 0)
{
timeUntilNextEventInMilliseconds = -1; //No events, so block until a message is received
}
else
{
timeUntilNextEventInMilliseconds = (nextEventTime - Poco::Timestamp())/1000 + 1; //Time in milliseconds till the next event, rounding up
}

//Poll until the next event timeout and resolve any messages that are received
SOM_TRY
if(zmq::poll(pollItems.get(), numberOfPollItems, timeUntilNextEventInMilliseconds) == 0)
{
continue; //Poll returned without indicating any messages have been received, so check events and go back to polling
}
SOM_CATCH("Error polling\n")

//Check if it is time to shutdown
if(pollItems[0].revents & ZMQ_POLLIN)
{
return; //Shutdown message received, so return
}

//Handle all of the messages from sockets
{
auto iter = interfaces.begin();
for(int i=1; i<numberOfPollItems && iter != interfaces.end(); i++)
{
if(pollItems[i].revents & ZMQ_POLLIN)
{ //Call associated message handling function
SOM_TRY
if((socketToHandlerFunction.at(iter->first))(classInstance, *this, *(iter->first)) == true)
{
break; //Function has requested that the polling loop be restarted, so skip the rest of the entries
}
SOM_CATCH("Error with message processing function\n")
}

iter++;
}
}

{
auto iter = fileInterfaces.begin();
for(int i=interfaces.size()+1; i<numberOfPollItems && iter != fileInterfaces.end(); i++)
{
if(pollItems[i].revents & ZMQ_POLLIN)
{ //Call associated message handling function
SOM_TRY
if((fileNumberToHandlerFunction.at(iter->first))(classInstance, *this, iter->second) == true)
{
break; //Function has requested that the polling loop be restarted, so skip the rest of the entries
}
SOM_CATCH("Error with message processing function\n")
}

iter++;
}
}


}//End while loop

}
catch(const std::exception &inputException)
{ //If an exception is thrown, swallow it, send error message and terminate
fprintf(stderr, "ReactorThread: %s\n", inputException.what());
return;
}
}

/**
This function regenerates the pollItems array given the current set of sockets.

@throws: This function can throw exceptions
*/
template <class classType> void reactor<classType>::regenerateZMQPollArray()
{
numberOfPollItems = interfaces.size()+fileInterfaces.size()+1;
SOM_TRY
pollItems.reset(new zmq::pollitem_t[numberOfPollItems]);
SOM_CATCH("Error creating poll items\n")

//First poll item is always the shutdown socket
pollItems[0] = {(void *) (*(shutdownReceivingSocket)), 0, ZMQ_POLLIN, 0};

{
auto iter = interfaces.begin();
for(int i=1; i<numberOfPollItems && iter != interfaces.end(); i++)
{
pollItems[i] = {(void *) (*(iter->second)), 0, ZMQ_POLLIN, 0};
iter++;
}
}

{
auto iter = fileInterfaces.begin();
for(int i=interfaces.size()+1; i<numberOfPollItems && iter != fileInterfaces.end(); i++)
{
pollItems[i] = {nullptr, iter->first, ZMQ_POLLIN, 0};
iter++;
}
}

}


