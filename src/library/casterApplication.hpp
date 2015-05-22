#ifndef  CASTERAPPLICATIONHPP
#define CASTERAPPLICATIONHPP

#include<Poco/Net/HTTPServer.h>
#include<Poco/Util/ServerApplication.h>
#include "casterRequestHandlerFactory.hpp"
#include<iostream>
#include<cstdio>
#include<vector>
#include<string>

/*
This object enables the cross platform creation of long running processes (such as a webserver) by running the "main" member function in a new process with the daemon flags enabled.
*/
class casterApplication : public Poco::Util::ServerApplication
{
protected:
/*
This function is run as the main of a daemon (or windows service), basically replacing the normal main to add crossplatform compatibility for long running processes.
@param inputArguments: The arguments passed to the program (excluding those meant for the ServerApplication base object), stored as strings in a vector
@return: The exit code for the program (should be one of the ExitCode enum values)

@exceptions: This function can throw exceptions
*/
int main(const std::vector<std::string> &inputArguments);


};



#endif 
