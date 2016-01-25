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
#include "Poco/ByteOrder.h"
#include "data_receiver_status_notification.pb.h"


namespace pylongps
{

/**
This class reads data from either a local ZMQ publisher or a PylonGPS caster and publishes it at the associated inproc ZMQ publisher address.
*/
class zmqDataReceiver : public dataReceiver
{
public:
/**
This function initializes the zmqDataReceiver to retrieve data from the given ZMQ PUB socket.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
zmqDataReceiver(const std::string &inputIPAddressAndPort, zmq::context_t &inputContext);

/**
This function initializes the zmqDataReceiver to retrieve data from the given given PylonGPS caster PUB socket.
@param inputIPAddressAndPort: A string with the IP address/port in format "IPAddress:portNumber"
@param inputCasterID: The ID of the caster to listen to (host format)
@param inputStreamID: The stream ID associated with the stream to listen to (host format)
@param inputContext: A reference to the ZMQ context to use
@param inputSubscribingToCaster: True if this object is subscribing to a caster and needs to strip the casterID/streamID from the stream before forwarding it 

@throws: This function can throw exceptions
*/
zmqDataReceiver(const std::string &inputIPAddressAndPort, int64_t inputCasterID, int64_t inputStreamID, zmq::context_t &inputContext, bool inputSubscribingToCaster = true);

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
std::unique_ptr<reactor<zmqDataReceiver> > receiverReactor;

bool stripHeader = false; //First sizeof(Poco::Int64)*2 bytes removed from each message when retransmitting if true to get rid of caster header
Poco::Int64 casterID; //The caster ID to listen for, in host format
Poco::Int64 streamID; //The stream ID to listen for, in host format

protected:
/**
This function reads from the ZMQ PUB port and forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputFileDescriptor: The file descriptor to read from
@return: false if the reactor doesn't need to restart its poll cycle

@throw: This function can throw exceptions
*/
bool readAndPublishData(reactor<zmqDataReceiver> &inputReactor, zmq::socket_t &inputSocket);
};























}
