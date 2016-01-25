#include "addressBasedLocationRetriever.hpp"

using namespace pylongps;

/**
This function sets the address string to search for.
@param inputAddressToSearchFor: The address to search for
*/
addressBasedLocationRetriever::addressBasedLocationRetriever(const std::string &inputAddressToSearchFor) : addressString(inputAddressToSearchFor)
{
//Replace ' ', '	',\n,\r characters with '+' characters
std::replace(addressString.begin(), addressString.end(), ' ', '+');
std::replace(addressString.begin(), addressString.end(), '	', '+');
std::replace(addressString.begin(), addressString.end(), '\n', '+');
std::replace(addressString.begin(), addressString.end(), '\r', '+');

connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
start();
}

/**
This function performs the retrieval operation the object was created for.
*/
void addressBasedLocationRetriever::run()
{
try
{
//Compose query string
std::string queryString =  "http://nominatim.openstreetmap.org/search?q=" + addressString + "&format=json";

Json::Value receivedJson;
if(getJsonValueFromURL(queryString, receivedJson) != true)
{
return;
}

if(receivedJson.isMember("lat") != true || receivedJson.isMember("lon") != true)
{
return;
}

if(!receivedJson["lat"].isString() || !receivedJson["lon"].isString())
{
return;
}

double lat = 0.0;
double lon = 0.0;
if(!convertStringToDouble(receivedJson["lat"].asString(), lat) || !convertStringToDouble(receivedJson["lon"].asString(), lon))
{
return;
}


emit retrievedLongitudeLatitude(lon, lat);
}
catch(const std::exception &inputException)
{
return;
}
}
