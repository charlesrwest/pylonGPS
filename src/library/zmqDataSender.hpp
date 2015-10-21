#pragma once

#include<cstdint>
#include<memory>
#include<thread>
#include<string>
#include<cstdio>
#include<unistd.h>

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
This class takes data published on an inproc ZMQ PUB socket and forwards it to a tcp ZMQ PUB socket. 
*/
class zmqDataSender
{
public:
/**
This function initializes the zmqDataSender to publish data using a ZMQ PUB socket on the given port.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputPortNumberToPublishOn: The TCP port number to bind/use for publishing

@throws: This function can throw exceptions
*/
zmqDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, int inputPortNumberToPublishOn);



/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress();

zmq::context_t &context;
std::unique_ptr<zmq::socket_t> subscriberSocket;
std::unique_ptr<zmq::socket_t> publishingSocket;
std::unique_ptr<zmq::socket_t> notificationPublishingSocket;
std::string informationSourceConnectionString; //String used to connect to the data source
std::string notificationConnectionString; //String used to publish status changes (such as unrecoverable disconnects)
std::unique_ptr<reactor<zmqDataSender> > senderReactor;

protected:
/**
This function forwards any received messages to the publisher socket
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool readAndWriteData(reactor<zmqDataSender> &inputReactor, zmq::socket_t &inputSocket);
};

}
