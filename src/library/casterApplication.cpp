#include "casterApplication.hpp"

/*
This function is run as the main of a daemon (or windows service), basically replacing the normal main to add crossplatform compatibility for long running processes.
@param inputArguments: The arguments passed to the program (excluding those meant for the ServerApplication base object), stored as strings in a vector
@return: The exit code for the program (should be one of the ExitCode enum values)
*/
int casterApplication::main(const std::vector<std::string> &inputArguments)
{
Poco::UInt16 port = 9998;
Poco::Net::HTTPServerParams *serverParams = new Poco::Net::HTTPServerParams; //HTTPServer takes ownership
serverParams->setMaxQueued(100);
serverParams->setMaxThreads(16);

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
Poco::Net::HTTPServer httpServer(new casterRequestHandlerFactory(), serverSocket, serverParams);

//Start the HTTPServer
httpServer.start();

//Wait for a kill signal, I think
waitForTerminationRequest(); 

//Stop the HTTPServer/clean up
httpServer.stop();
}
