#include "casterApplication.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

/**
This function is run as the main of a daemon (or windows service), basically replacing the normal main to add crossplatform compatibility for long running processes.
@param inputArguments: The arguments passed to the program (excluding those meant for the ServerApplication base object), stored as strings in a vector
@return: The exit code for the program (should be one of the ExitCode enum values)

@throws: This function can throw exceptions
*/
int casterApplication::main(const std::vector<std::string> &inputArguments)
{
try
{
//Expect:
//Port number to bind (binds port number and port number+1)
//Maximum number of threads to use (optional)
//Maximum number of connections to queue before rejecting new ones (optional)

//Make sure there is at least a port number
if(inputArguments.size() < 1)
{
std::cerr << "Error: insufficient arguments" << std::endl << "Usage: ntripServer portNumberToBind optionalMaximumNumberOfThreads optionalMaximumNumberOfConnectionsToQueue" << std::endl;
return 1;
}

//Attempt convert to number
std::vector<int> numericArguments;
for(int i=0; i<inputArguments.size() && i < 3; i++)
{
try
{
numericArguments.push_back(stoi(inputArguments[i]));
}
catch(const std::exception &inputException)
{//Conversion failed
std::cerr << "Error, argument " << i << " is invalid."  << std::endl << "Usage: ntripServer portNumberToBind optionalMaximumNumberOfThreads optionalMaximumNumberOfConnectionsToQueue" << std::endl;
return 1;
}

if(numericArguments.back() < 0)
{ //No negative values permitted
std::cerr << "Error, argument " << i << " is invalid."  << std::endl << "Usage: ntripServer portNumberToBind optionalMaximumNumberOfThreads optionalMaximumNumberOfConnectionsToQueue" << std::endl;
return 1;
}

}


Poco::UInt16 port = numericArguments[0];
Poco::Net::TCPServerParams *serverParams = new Poco::Net::TCPServerParams; //TCPServer takes ownership
if(numericArguments.size() >= 2)
{//Determine the size of the thread pool
serverParams->setMaxThreads(numericArguments[1]);
}
else
{
serverParams->setMaxThreads(pylongps::DEFAULT_MAX_NUMBER_OF_SERVER_THREADS);
}

if(numericArguments.size() >= 3)
{ //Determine how many connections can be waiting to be serviced before they are just dropped
serverParams->setMaxQueued(numericArguments[1]);
}
else
{
serverParams->setMaxThreads(pylongps::DEFAULT_MAX_NUMBER_OF_QUEUED_CONNECTIONS);
}



//Create ZMQ context
std::unique_ptr<zmq::context_t> context;
SOM_TRY
context.reset(new zmq::context_t);
SOM_CATCH("Error initializing ZMQ context\n")

//Create source manager
std::unique_ptr<pylongps::sourceManager> manager;
SOM_TRY
manager.reset(new pylongps::sourceManager(context.get(), port+1));
SOM_CATCH("Error initializing source manager\n")

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
auto variable = new TCPServerConnectionFactoryImplementation(context.get(), manager->serverRegistrationDeregistrationSocketConnectionString, manager->mountpointDisconnectSocketConnectionString, manager->sourceTableAccessSocketConnectionString, manager->serverMetadataAdditionSocketPortNumber);
Poco::Net::TCPServer tcpServer(variable, serverSocket, serverParams);

//Start the HTTPServer
tcpServer.start();

//Wait for a kill signal, I think
waitForTerminationRequest(); 

//Stop the HTTPServer/clean up
tcpServer.stop();
}
catch(const std::exception &inputException)
{//Print out exceptions and return gracefully
std::cerr << inputException.what() << std::endl;
return 1;
}
}
