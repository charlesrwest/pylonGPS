#include "event.hpp"

using namespace pylongps; //Use pylongps classes without alteration for now

/**
Constructs the object without any submessages, set to occur/expire at the given time
@param inputTime: The time that the event times out or occurs
*/
event::event(const Poco::Timestamp &inputTime)
{
time = inputTime;
} 



/**
This function returns left.time > right.time
@param inputLeftEvent: The left side of >
@param inputRightEvent: The right side of >
@return: inputLeftEvent > inputRightEvent
*/
bool pylongps::operator<(const event &inputLeftEvent, const event &inputRightEvent)
{
return inputLeftEvent.time > inputRightEvent.time;
}
