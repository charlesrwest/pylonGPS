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
Poco::UInt16 port = 9998;
Poco::Net::TCPServerParams *serverParams = new Poco::Net::TCPServerParams; //TCPServer takes ownership
serverParams->setMaxQueued(100);
serverParams->setMaxThreads(16);

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
auto variable = new Poco::Net::TCPServerConnectionFactoryImpl<casterTCPConnectionHandler>();
Poco::Net::TCPServer tcpServer(variable, serverSocket, serverParams);

//Start the HTTPServer
tcpServer.start();

//Wait for a kill signal, I think
waitForTerminationRequest(); 

//Stop the HTTPServer/clean up
tcpServer.stop();
}
