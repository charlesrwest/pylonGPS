#include "serverApplication.hpp"

using namespace pylongps;

/**
This function is run as the main of a daemon (or windows service), basically replacing the normal main to add crossplatform compatibility for long running processes.
@param inputArguments: The arguments passed to the program (excluding those meant for the ServerApplication base object), stored as strings in a vector
@return: The exit code for the program (should be one of the ExitCode enum values)

@throws: This function can throw exceptions
*/
int serverApplication::main(const std::vector<std::string> &inputArguments)
{
static const std::string usage = "Usage: ntripServer portNumberToBind casterURI ntripFormatSourceMetadata";

try
{
//Expect:
//Port number to received data on
//URI of caster to send to
//source metadata to register, in NTRIP format

//Make sure there are sufficient arguments
if(inputArguments.size() < 3)
{
std::cerr << "Error: insufficient arguments" << std::endl << usage << std::endl;
return 1;
}

//Attempt deserialize the port number
int portNumberArgument = -1;
try
{
portNumberArgument = stoi(inputArguments[0]);
}
catch(const std::exception &inputException)
{//Conversion failed
std::cerr << "Error, port number is invalid."  << std::endl << usage << std::endl;
return 1;
}

if(portNumberArgument < 0)
{ //No negative values permitted
std::cerr << "Error, port number is invalid."  << std::endl << usage << std::endl;
return 1;
}

//Attempt to deserialize the URI
std::unique_ptr<Poco::URI> uriArgument;
try
{
uriArgument.reset(new Poco::URI(inputArguments[1]));
}
catch(const std::exception &inputException)
{
std::cerr << "Error, unable to parse caster URI" << std::endl << usage << std::endl;
return 1;
}

if(uriArgument->getPort() == 0)
{
std::cerr << "Error, caster port invalid" << std::endl << usage << std::endl;
return 1;
}

if(uriArgument->getHost().size() == 0)
{
std::cerr << "Error, caster host field invalid" << std::endl << usage << std::endl;
return 1;
}

//Parse metadata
streamSourceTableEntry metadataArgument;
if(metadataArgument.parse(inputArguments[2], false) < 0)
{
std::cerr << "Error, server metadata argument invalid" << std::endl << usage << std::endl;
return 1;
}

Poco::UInt16 port = portNumberArgument;
Poco::Net::TCPServerParams *serverParams = new Poco::Net::TCPServerParams; //TCPServer takes ownership
serverParams->setMaxThreads(1); //One connection at a time
serverParams->setMaxQueued(1);

Poco::Net::ServerSocket serverSocket(port); //Create a server socket
auto variable = new serverTCPServerConnectionFactoryImplementation(*uriArgument, metadataArgument);
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
