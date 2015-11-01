#pragma once

#include<cstdint>
#include<memory>
#include<thread>
#include<string>
#include<cstdio>
#include<unistd.h>
#include<set>

#include<Poco/Net/TCPServer.h>
#include<Poco/Net/TCPServerConnection.h>
#include<Poco/Net/TCPServerConnectionFactory.h>

#include "dataSender.hpp"
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "zmq.hpp"
#include "reactor.hpp"
#include "utilityFunctions.hpp"
#include "data_receiver_status_notification.pb.h" 

namespace pylongps
{


/**
This class takes data published on an inproc ZMQ PUB socket and forwards it to anyone that connects to it via a TCP socket. Due to Ubuntu only having ZMQ 4.0 in the repositories (rather than 4.1), this needs to be done using a Poco TCP server, which means notifications aren't really currently supported
*/
class tcpDataSender : public dataSender
{
public:
/**
This function initializes the tcpDataSender to publish data via a bound TCP port to any connecting clients.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputPortNumberToPublishOn: The TCP port number to bind/use for publishing

@throws: This function can throw exceptions
*/
tcpDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, int inputPortNumberToPublishOn);


/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress();

/**
This function shuts down the Poco TCP server.
*/
~tcpDataSender();

zmq::context_t &context;
std::unique_ptr<zmq::socket_t> notificationPublishingSocket;

std::unique_ptr<Poco::Net::ServerSocket> tcpSocket;
std::unique_ptr<Poco::Net::TCPServer> tcpServer;


std::string informationSourceConnectionString; //String used to connect to the data source
std::string notificationConnectionString; //String used to publish status changes (such as unrecoverable disconnects)

};


class tcpDataSenderTCPConnectionHandler : public Poco::Net::TCPServerConnection
{
public:
/**
This function initializes the connection handler so that it can forward data received from the given source connection string.
@param inputConnectionSocket: The socket to forward data to
@param inputSourceConnectionString: The connection string to use for the data source
@param inputContext: The context to use with the source connection string

@throws: This function can throw exception
*/
tcpDataSenderTCPConnectionHandler(const Poco::Net::StreamSocket &inputConnectionSocket, const std::string &inputSourceConnectionString, zmq::context_t &inputContext);

/**
This function is called to handle the TCP connection that is owned by the base TCPServerConnection object and forwards the received data over the connection.
*/
void run();

std::string sourceConnectionString; //String used to connect to the data source
std::unique_ptr<zmq::socket_t> subscriberSocket; //A ZMQ_SUB socket connected to the information source
zmq::context_t &context;
const Poco::Net::StreamSocket &connectionSocket;
};

class tcpDataSenderTCPServerConnectionFactoryImplementation : public Poco::Net::TCPServerConnectionFactory
{
public:
/**
This function initializes the factory with the values to pass to the connection handlers.
@param inputSourceConnectionString: The connection string to connect to a ZMQ_PUB socket which is sending information to forward
@param inputContext: The zmq context to use
*/
tcpDataSenderTCPServerConnectionFactoryImplementation(const std::string &inputSourceConnectionString, zmq::context_t &inputContext);

/**
This function creates a tcpDataSenderTCPConnectionHandler to handle a particular TCP connection.
@param inputConnectionSocket: The connection socket to forward the data over
*/
Poco::Net::TCPServerConnection *createConnection(const Poco::Net::StreamSocket &inputConnectionSocket);

zmq::context_t &context;
std::string sourceConnectionString;
};


}
