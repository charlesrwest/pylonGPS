#include "basestationListRetriever.hpp"

using namespace pylongps; 

/**
This function initializes the object and gives it the GUI for it to update.
@param inputGUI: The GUI to update
@param inputQueryRangeBoundingBox: The west, north, east, south boundaries to consider for the restrictions on the basestations to consider (in radians)
@param inputConnectionString: The ZMQ connection string to use for connecting to the caster

throw: This function can throw exceptions
*/
basestationListRetriever::basestationListRetriever(pylongps::transceiverGUI &inputGUI, std::array<double, 4> inputBoundArray, const std::string &inputConnectionString) : guiReference(inputGUI)
{

//Make a ZMQ socket to send request to caster
SOM_TRY
casterRequestSocket.reset(new zmq::socket_t(*(guiReference.context), ZMQ_REQ));
SOM_CATCH("Error intializing casterRequestSocket\n")

SOM_TRY
casterRequestSocket->setsockopt(ZMQ_RCVTIMEO, (void *) &BASESTATION_LIST_RECEIVER_TIMEOUT_DURATION_IN_MILLISECONDS, sizeof(BASESTATION_LIST_RECEIVER_TIMEOUT_DURATION_IN_MILLISECONDS));
SOM_CATCH("Error setting timeout time\n")

SOM_TRY
casterRequestSocket->connect(inputConnectionString.c_str());
SOM_CATCH("Error connecting casterRequestSocket\n")

boundArray = inputBoundArray;

connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
start();
}

/**
This function performs the retrieval operation the object was created for.
*/
void basestationListRetriever::run()
{
try
{
printf("Called\n");

//Compose query
client_query_request request;
client_subquery requestSubquery;

sql_double_condition westCondition;
westCondition.set_relation(GREATER_THAN_EQUAL_TO);
westCondition.set_value(fmin(boundArray[0],boundArray[2])*180.0/pylongps::PI);

sql_double_condition northCondition;
northCondition.set_relation(LESS_THAN_EQUAL_TO);
northCondition.set_value(fmax(boundArray[1],boundArray[3])*180.0/pylongps::PI);

sql_double_condition eastCondition;
eastCondition.set_relation(LESS_THAN_EQUAL_TO);
eastCondition.set_value(fmax(boundArray[0],boundArray[2])*180.0/pylongps::PI); 

sql_double_condition southCondition;
southCondition.set_relation(GREATER_THAN_EQUAL_TO); 
southCondition.set_value(fmin(boundArray[1],boundArray[3])*180.0/pylongps::PI);

sql_double_condition *doubleConditionBuffer = nullptr;

doubleConditionBuffer = requestSubquery.add_latitude_condition(); 
(*doubleConditionBuffer) = northCondition;

doubleConditionBuffer = requestSubquery.add_latitude_condition(); 
(*doubleConditionBuffer) = southCondition;

doubleConditionBuffer = requestSubquery.add_longitude_condition(); 
(*doubleConditionBuffer) = westCondition;

doubleConditionBuffer = requestSubquery.add_longitude_condition(); 
(*doubleConditionBuffer) = eastCondition;

(*request.add_subqueries()) = requestSubquery;

bool messageReceived = false;
bool messageDeserialized = false;
client_query_reply reply;

{
//Lock mutex
std::lock_guard<std::mutex> lockGuard(guiReference.visibleBasestationMutex);
guiReference.timeOfLastSentQuery = Poco::Timestamp();
}

SOM_TRY
std::tie(messageReceived, messageDeserialized) = remoteProcedureCall(*casterRequestSocket, request, reply);
SOM_CATCH("Error, unable to do RPC\n")

if(!messageReceived || !messageDeserialized)
{
return;
}

if(!reply.has_caster_id() || reply.has_failure_reason())
{
return;
}

//Lock mutex
std::lock_guard<std::mutex> lockGuard(guiReference.visibleBasestationMutex);

//Update cache
guiReference.potentiallyVisibleBasestationList.clear();
guiReference.latitudeToBasestationKey.clear();
guiReference.longitudeToBasestationKey.clear();
for(int i=0; i<reply.base_stations_size(); i++)
{
std::pair<int64_t, int64_t> basestationIDPair(reply.caster_id(), reply.base_stations(i).base_station_id());
guiReference.potentiallyVisibleBasestationList.emplace(basestationIDPair, reply.base_stations(i));
guiReference.latitudeToBasestationKey.emplace(reply.base_stations(i).latitude()*pylongps::PI/180.0, basestationIDPair); //Store in radians
guiReference.longitudeToBasestationKey.emplace(reply.base_stations(i).longitude()*pylongps::PI/180.0, basestationIDPair);
}
guiReference.lastQueryBoundaryInRadians = boundArray;
guiReference.timeOfLastQueryUpdate = Poco::Timestamp(); //Set to now

emit updatedBasestationLists();
}
catch(const std::exception &inputException)
{
return;
}
}
