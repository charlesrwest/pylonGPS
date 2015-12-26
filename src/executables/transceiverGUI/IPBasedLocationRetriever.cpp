#include "IPBasedLocationRetriever.hpp"

using namespace pylongps;

/**
This function performs the retrieval operation the object was created for.
*/
void IPBasedLocationRetriever::operate()
{
try
{
Json::Value receivedJson;
if(getJsonValueFromURL("http://ip-api.com/json", receivedJson) != true)
{
emit finished();
return;
}

if(receivedJson.isMember("lat") != true || receivedJson.isMember("lon") != true)
{
emit finished();
return;
}

if(!receivedJson["lat"].isDouble() || !receivedJson["lon"].isDouble())
{
emit finished();
return;
}


emit retrievedLongitudeLatitude(receivedJson["lon"].asDouble(), receivedJson["lat"].asDouble());

emit finished();
}
catch(const std::exception &inputException)
{
emit finished();
return;
}
}
 

