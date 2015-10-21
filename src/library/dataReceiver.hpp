#pragma once



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
@return: The connection string required to connect to the given object's information stream
*/
virtual std::string address() = 0;

/**
This function is required to return a ZMQ inproc address which can be used with the context given to the data receiver to access status notifications regarding the object.
@return: The connection string required to connect to the given object's notification stream
*/
virtual std::string notificationAddress() = 0;

/**
This specifies the destructor as virtual so that the derived classes will have their destructors called if a base class pointer to them is deleted.
*/
virtual ~dataReceiver() {}
};
























}  
