#pragma once

#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "zmq.hpp"
#include<cstdint>
#include<memory>
#include<thread>

namespace pylongps
{



/**
This class is an abstract base class that defines the interfaces for a "dataTransmitter" class which forwards data from a local source over a zmq inproc publisher socket.  If an error occurs, it is published to a ZMQ PUB socket claiming the notificationAddress.
*/
class dataSender
{
public:

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress() = 0;

/**
This specifies the destructor as virtual so that the derived classes will have their destructors called if a base class pointer to them is deleted.
*/
virtual ~dataSender() {}
};























} 
