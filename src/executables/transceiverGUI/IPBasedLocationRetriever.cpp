#include "IPBasedLocationRetriever.hpp"

using namespace pylongps;

/**
This function sets up the object to delete itself when it has finished its task.
*/
IPBasedLocationRetriever::IPBasedLocationRetriever()
{
connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

/**
This function performs the retrieval operation the object was created for.
*/
void IPBasedLocationRetriever::run()
{
try
{
Json::Value receivedJson;
if(getJsonValueFromURL("http://ip-api.com/json", receivedJson) != true)
{
return;
}

if(receivedJson.isMember("lat") != true || receivedJson.isMember("lon") != true)
{
return;
}

if(!receivedJson["lat"].isDouble() || !receivedJson["lon"].isDouble())
{
return;
}


emit retrievedLongitudeLatitude(receivedJson["lon"].asDouble(), receivedJson["lat"].asDouble());
}
catch(const std::exception &inputException)
{
return;
}
}
 

