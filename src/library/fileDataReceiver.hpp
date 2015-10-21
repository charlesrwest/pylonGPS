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
const int FILE_DATA_RECEIVER_DATA_BUFFER_SIZE = 1024;

class fileDataReceiver : public dataReceiver
{
public:
/**
This function initializes the fileDataReceiver to retrieve data from the given file descriptor.  The object takes ownership of the file and closes the pointer on destruction.
@param inputFilePointer: A file stream pointer to retrieve data from
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
fileDataReceiver(FILE *inputFilePointer, zmq::context_t &inputContext);

/**
This function initializes the fileDataReceiver to retrieve data from the file that the given path points to.
@param inputFilePath: The path to the file to retrieve data from
@param inputContext: A reference to the ZMQ context to use

@throws: This function can throw exceptions
*/
fileDataReceiver(const std::string &inputFilePath, zmq::context_t &inputContext);

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
char dataBuffer[sizeof(char)*FILE_DATA_RECEIVER_DATA_BUFFER_SIZE];
std::unique_ptr<reactor<fileDataReceiver> > receiverReactor;

protected:
/**
This function performs a nonblocking read of the given file descriptor and (if there is any data) forwards the received data to the publisher socket.
@param inputReactor: The reactor which called the function
@param inputFileDescriptor: The file descriptor to read from

@throw: This function can throw exceptions
*/
bool readAndPublishData(reactor<fileDataReceiver> &inputReactor, FILE *inputFileDescriptor);

/**
Helper function for the common elements between constructors that delegation doesn't appear to fit well.  Should only be called as part of a constructor
@param inputFilePointer: A file stream pointer to retrieve data from

@throw: This function can throw exceptions
*/
void subConstructor(FILE *inputFilePointer);
};























}
