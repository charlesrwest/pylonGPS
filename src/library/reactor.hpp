#ifndef REACTORHPP
#define REACTORHPP

#include<functional>
#include<tuple>
#include<memory>
#include<queue>
#include "SOMException.hpp"
#include "SOMScopeGuard.hpp"
#include "event.hpp"
#include "utilityFunctions.hpp"
#include "Poco/Timestamp.h"
#include "zmq.hpp"

namespace pylongps
{

/**
This class starts its own thread, maintains its own event queue and (when activated) processes messages received on its ZMQ socket interfaces/scheduled events.  In general, it is meant to be used as a friend of its given template class.  Sockets are assumed to have already been initialized but the reactor takes ownership of them when they are added (a call to remove destroys the associated socket).  Automatically adds one internal interface to notify the reactor thread when it is time to shut down.
*/
template <class classType> class reactor
{
public:
/**
This function initializes the reactor with the function that should be called to handle events.
@param inputContext: The ZMQ context that this object should use
@param inputClassInstance: The instance of the class that the given functions should operate on
@param inputEventHandler: The function to call to handle events in the queue (should return negative if there are no outstanding events)

@throws: This function can throw exceptions
*/
reactor(zmq::context_t *inputContext, classType *inputClassInstance, std::function<Poco::Timestamp &(classType*, reactor<classType> *)> inputEventHandler);

/**
This (not thread safe) function adds a new socket for the reactor to take ownership of and the member function to call/pass the socket reference to when a message is waiting on that interface.
@param inputSocket: The socket to take ownership of
@param inputMessageHandler: The function to call to handle messages waiting on the interface (returns true if the poll loop should restart rather than continuing and expects a pointer to this object)

@throws: This function can throw exceptions
*/
void addInterface(zmq::socket_t *inputSocket, std::function<bool (classType*, reactor<classType> *, zmq::socket_t &)> inputMessageHandler);

/**
This (not threadsafe) function removes an interface from the reactor.
@param inputSocket: The socket interface to remove

@throws: This function can throw exceptions
*/
void removeInterface(zmq::socket_t *inputSocket);

/**
This function starts the reactor so that it begins to process events and messages.
@param inputStartingEvents: The events that should be in the queue when message processing begins

@throws: This function can throw exceptions
*/
void start(const std::vector<event> &inputStartingEvents);

/**
This function sends the termination signal to the reactor's thread and waits for it to shut down. 
*/
~reactor();

/**
This function is called by the thread object to perform the message and event processing operations.
*/
void reactorThreadFunction();

/**
This function regenerates the pollItems array given the current set of sockets.

@throws: This function can throw exceptions
*/
void regenerateZMQPollArray();




std::priority_queue<pylongps::event> eventQueue;
std::map<zmq::socket_t *, std::unique_ptr<zmq::socket_t> > interfaces;
std::map<zmq::socket_t *, std::function<bool (classType*, reactor<classType> *, zmq::socket_t &)> > socketToHandlerFunction;

private:
std::function<Poco::Timestamp &(classType*, reactor<classType> *)> eventHandlerFunction;

zmq::context_t *context;
classType *classInstance;
std::unique_ptr<zmq::socket_t> shutdownSocket; //This socket is used to tell the reactor thread to shut down and to remove sockets
std::unique_ptr<zmq::socket_t> shutdownReceivingSocket; //This socket receives the shutdown and remove socket signals
std::unique_ptr<std::thread> reactorThread;
std::unique_ptr<zmq::pollitem_t[]> pollItems;
int numberOfPollItems = 0;
};

















#include "reactorDefinitions.hpp"

}
#endif
