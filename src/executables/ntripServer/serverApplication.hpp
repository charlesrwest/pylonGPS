#ifndef  SERVERAPPLICATIONHPP
#define SERVERAPPLICATIONHPP

#include<Poco/Net/TCPServer.h>
#include<Poco/Util/ServerApplication.h>
#include<Poco/Net/TCPServerConnectionFactory.h>
#include<Poco/URI.h>
#include<Poco/Net/DNS.h>
#include "serverTCPServerConnectionFactoryImplementation.hpp"
#include "serverTCPConnectionHandler.hpp"
#include<iostream>
#include<cstdio>
#include<vector>
#include<string>
#include<memory>

namespace pylongps
{


/**
\ingroup Server
This object enables the cross platform creation of long running processes (such as a webserver) by running the "main" member function in a new process with the daemon flags enabled.

//Expects arguments:
//Port number to bind
//URI of the caster to forward the data to
//NTRIP format source metadata
//Optional metadata registration port of caster (if not caster ntrip port number + 1)
*/
class serverApplication : public Poco::Util::ServerApplication
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
