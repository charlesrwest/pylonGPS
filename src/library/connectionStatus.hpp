#ifndef CONNECTIONSTATUSHPP
#define CONNECTIONSTATUSHPP

#include "Poco/Timestamp.h"


namespace pylongps
{
 

/**
This class is used in a map to keep track of the current details associated with a given connection.
*/
class connectionStatus
{
public:
/*
This function sets hasBeenRegistered to false and timeLastMessageWasReceived to 0.
*/
connectionStatus();

bool hasBeenRegistered;
bool requestToTheDatabaseHasBeenSent;
int64_t baseStationID;
Poco::Timestamp timeLastMessageWasReceived;
};



















}
#endif
