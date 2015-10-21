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
This class takes data published on an inproc ZMQ PUB socket and forwards it to a file stream.
*/
class fileDataSender
{
public:
/**
This function initializes the fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputFilePointer: A file stream pointer to retrieve data from

@throws: This function can throw exceptions
*/
fileDataSender(const std::string &inputSourceConnectionString, zmq::context_t &inputContext, FILE *inputFilePointer);

/**
This function initializes the fileDataSender to send data to the given file.  The object takes ownership of the file and closes the pointer on destruction.
@param inputZMQConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputContext: A reference to the ZMQ context to use
@param inputFilePath: The path to the file to send data to

@throws: This function can throw exceptions
*/
fileDataSender(const std::string &inputZMQConnectionString, zmq::context_t &inputContext, const std::string &inputFilePath);


/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress();

zmq::context_t &context;
std::unique_ptr<FILE, decltype(&fclose)> filePointer;
std::unique_ptr<zmq::socket_t> subscriberSocket;
std::unique_ptr<zmq::socket_t> notificationPublishingSocket;
std::string informationSourceConnectionString; //String used to connect to the data source
std::string notificationConnectionString; //String used to publish status changes (such as unrecoverable disconnects)
std::unique_ptr<reactor<fileDataSender> > senderReactor;

protected:
/**
This function forwards any received messages to the given file descriptor
@param inputReactor: The reactor which called the function
@param inputSocket: The ZMQ socket to read from

@throw: This function can throw exceptions
*/
bool readAndWriteData(reactor<fileDataSender> &inputReactor, zmq::socket_t &inputSocket);

/**
Helper function for the common elements between constructors that delegation doesn't appear to fit well.  Should only be called as part of a constructor
@param inputSourceConnectionString: The connection string to use to subscribe to the ZMQ PUB socket that is providing the data
@param inputFilePointer: A file stream pointer to retrieve data from

@throw: This function can throw exceptions
*/
void subConstructor(const std::string &inputSourceConnectionString, FILE *inputFilePointer);
};

}
