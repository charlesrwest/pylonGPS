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
This class is an abstract base class that defines the interfaces for a "dataTransmitter" class which forwards data from a local source over a zmq inproc publisher socket.
*/
class dataTransmitter
{
public:
/**
This function has the data subscriber remove any previous connections it held and subscribe to the address given here.
@param inputConnectionString: The ZMQ connection string to use

@throw: This function can throw exceptions
*/
virtual void connect(const std::string &inputConnectionString);
};























} 
