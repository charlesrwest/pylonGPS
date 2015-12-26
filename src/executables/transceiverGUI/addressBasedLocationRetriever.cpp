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
}

/**
This function performs the retrieval operation the object was created for.
*/
void addressBasedLocationRetriever::operate()
{
try
{
//Compose query string
std::string queryString =  "http://nominatim.openstreetmap.org/search?q=" + addressString + "&format=json";

Json::Value receivedJson;
if(getJsonValueFromURL(queryString, receivedJson) != true)
{
emit finished();
return;
}

if(receivedJson.isMember("lat") != true || receivedJson.isMember("lon") != true)
{
emit finished();
return;
}

if(!receivedJson["lat"].isString() || !receivedJson["lon"].isString())
{
emit finished();
return;
}

double lat = 0.0;
double lon = 0.0;
if(!convertStringToDouble(receivedJson["lat"].asString(), lat) || !convertStringToDouble(receivedJson["lon"].asString(), lon))
{
emit finished();
return;
}


emit retrievedLongitudeLatitude(lon, lat);

emit finished();
}
catch(const std::exception &inputException)
{
emit finished();
return;
}
}
