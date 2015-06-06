#ifndef  CASTERTCPCONNECTIONHANDLERHPP
#define CASTERTCPCONNECTIONHANDLERHPP

#include<Poco/Net/TCPServerConnection.h>
#include<Poco/Timespan.h>
#include<Poco/Net/HTTPRequest.h>
#include<string>
#include<cstring>
#include<sstream>  //Stringstream
#include "SOMException.hpp"
#include<cstdio> //For debugging
#include<iostream> //For debugging
#include<Poco/StreamCopier.h>

namespace pylongps
{

const std::string serverResponseString = "ICY 200 OK\r\n";

#define HEADER_RECEIVE_TIMEOUT 1 //How long to wait for header before dropping the connection
#define HEADER_BUFFER_SIZE 512 //How big header can be before we drop the connection

/**
\ingroup Server
*/
class casterTCPConnectionHandler : public Poco::Net::TCPServerConnection
{
public:
/**
This constructor just passes the given socket to the base class
@param inputConnectionSocket: The socket associated with the connection this object is processing

@throws: This function can throw exceptions
*/
casterTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket); 


/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object.

@throws: This function can throw exceptions
*/
void run();
};


}
#endif 
