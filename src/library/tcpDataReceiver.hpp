#pragma once

#include<cstdint>
#include<memory>
#include<thread>
#include<string>
#include<cstdio>
#include<unistd.h>

#include "dataReceiver.hpp"
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "zmq.hpp"
#include "reactor.hpp"
#include "utilityFunctions.hpp"
#include "data_receiver_status_notification.pb.h"


namespace pylongps
{

/**
This class reads data from a TCP server and publishes it at the associated inproc ZMQ publisher address.  It will automatically attempt to reconnect if the connection is closed.
*/
class tcpDataReceiver : public dataReceiver
{
public:
/**
This function initializes the tcpDataReceiver to retrieve data from the given TCP address/port.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
tcpDataReceiver(const std::string &inputIPAddressAndPort, zmq::context_t &inputContext);

/**
This function returns a string containing the ZMQ connection string required to connect this object's publisher (which forwards data from the associated file).
@return: The connection string to use to connect to this data source
*/
virtual std::string address();

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress();

zmq::context_t &context;
std::unique_ptr<zmq::socket_t> publishingSocket;
std::unique_ptr<zmq::socket_t> notificationPublishingSocket;
std::string publisherConnectionString; //String used to connect to this object's publisher
std::string notificationConnectionString; //String used to publish status changes (such as unrecoverable disconnects)
std::unique_ptr<reactor<tcpDataReceiver> > receiverReactor;

protected:
/**
This function reads from the TCP port and forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from
@return: false if the reactor doesn't need to restart its poll cycle

@throw: This function can throw exceptions
*/
bool readAndPublishData(reactor<tcpDataReceiver> &inputReactor, zmq::socket_t &inputSocket);
};























}
