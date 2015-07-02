#ifndef  CASTERAPPLICATIONHPP
#define CASTERAPPLICATIONHPP

#include<Poco/Net/TCPServer.h>
#include<Poco/Util/ServerApplication.h>
#include<Poco/Net/TCPServerConnectionFactory.h>
#include "casterTCPServerConnectionFactoryImplementation.hpp"
#include "casterTCPConnectionHandler.hpp"
#include "sourceManager.hpp"
#include<iostream>
#include<cstdio>
#include<vector>
#include<string>
#include<memory>

namespace pylongps
{

const int CASTER_DEFAULT_MAX_NUMBER_OF_SERVER_THREADS = 1000;
const int CASTER_DEFAULT_MAX_NUMBER_OF_QUEUED_CONNECTIONS = 100;

/**
\ingroup Server
This object enables the cross platform creation of long running processes (such as a webserver) by running the "main" member function in a new process with the daemon flags enabled.

//Expects arguments:
//Port number to bind (binds port number and port number+1 by default)
//Maximum number of threads to use (optional)
//Maximum number of connections to queue before rejecting new ones (optional)
//Optional port to use for meta
*/
class casterApplication : public Poco::Util::ServerApplication
{
protected:
/**
This function is run as the main of a daemon (or windows service), basically replacing the normal main to add crossplatform compatibility for long running processes.
@param inputArguments: The arguments passed to the program (excluding those meant for the ServerApplication base object), stored as strings in a vector
@return: The exit code for the program (should be one of the ExitCode enum values)

@throws: This function can throw exceptions
*/
int main(const std::vector<std::string> &inputArguments);

};


}
#endif 
