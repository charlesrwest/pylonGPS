#pragma once

#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "zmq.hpp"
#include<cstdint>
#include<memory>
#include<thread>
#include<string>

namespace pylongps
{

/**
This class is an abstract base class that defines the interfaces for a "dataReceiver" class which forwards data from a local source over a zmq inproc publisher socket.
*/
class dataReceiver
{
public:

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access the information published by this object.
*/
virtual std::string address() = 0;
};
























}  
