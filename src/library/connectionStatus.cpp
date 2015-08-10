#include "connectionStatus.hpp"

using namespace pylongps;

/*
This function sets hasBeenRegistered to false and timeLastMessageWasReceived to 0.
*/
connectionStatus::connectionStatus()
{
hasBeenRegistered = false;
timeLastMessageWasReceived = 0;
requestToTheDatabaseHasBeenSent = false;
baseStationID = 0;
} 
