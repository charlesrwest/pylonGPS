#ifndef  EVENTHPP
#define EVENTHPP

#include<chrono>
#include "event_message.pb.h"
#include "possible_metadata_timeout_event.pb.h"

/**
\ingroup Events
This class represents an event that is scheduled to happen at a particular point in time.  In an attempt to make it easy to extend the event queue processing to addition event types, the details concerning any given set of events are defined as optional protobuf messages inside a this container.  
*/
class event : public event_message //Can contain optional messages
{
public:
/**
Constructs the object without any submessages, set to occur/expire at the given time
@param inputTime: The time that the event times out or occurs
*/
event(const std::chrono::steady_clock::time_point &inputTime);



std::chrono::steady_clock::time_point time; //Time event is scheduled to occur
};

/**
This function returns left.time > right.time
@param inputLeftEvent: The left side of >
@param inputRightEvent: The right side of >
@return: inputLeftEvent > inputRightEvent
*/
bool operator>(const event &inputLeftEvent, const event &inputRightEvent);








#endif
