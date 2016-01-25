#pragma once

#include<QThread>
#include<string>
#include<utility>
#include<Poco/Net/StreamSocket.h>
#include<Poco/Net/DNS.h>
#include<Poco/Net/HTTPRequest.h>
#include<sstream>
#include "utilityFunctions.hpp"
#include<json.h>
#include "SOMScopeGuard.hpp"
#include<algorithm>

namespace pylongps
{

/**
This class retrieves the latitude/longitude associated with the address it was constructed with (if any) and emits it as a Qt signal.  It is meant to be run in a seperate Qt thread, as it makes network calls which can take some time to complete.
*/
class addressBasedLocationRetriever : public QThread
{
Q_OBJECT
public:
/**
This function sets the address string to search for.
@param inputAddressToSearchFor: The address to search for
*/
addressBasedLocationRetriever(const std::string &inputAddressToSearchFor);

std::string addressString;

public slots:
/**
This function performs the retrieval operation the object was created for.
*/
void run();

signals:
void retrievedLongitudeLatitude(double inputLongitude, double inputLatitude); //The lat/long pair (in that order)
void error(std::string inputError); //A signal emitted if a error was encountered


}; 

}
